#include "EditorContext.h"
#include "Reflection/Reflection.h"
#include "Managers/LevelManager.h"
#include "Components/CameraComponent.h"
#include "UI/Widgets/Widget.h"
#include "Shortcuts.h"
#include <imgui.h>

using namespace Krampus;

namespace
{
	constexpr const char*	DRAG_PAYLOAD	= "KRAMPUS_ACTOR";

	struct HierarchyState
	{
		EditorContext*									context;
		Actor*											defaultCamera	= nullptr;
		std::unordered_map<Actor*, Actor*>				parentOf;
		Actor*											toDelete	= nullptr;
		Actor*											dragged		= nullptr;
		Actor*											dropTarget	= nullptr;	// null with dragged set = make root
	};

	bool IsDescendantOf(const HierarchyState& _state, Actor* _actor, Actor* _ancestor)
	{
		for (Actor* _current = _actor; _current; )
		{
			if (_current == _ancestor) return true;
			const auto _it = _state.parentOf.find(_current);
			_current = _it == _state.parentOf.end() ? nullptr : _it->second;
		}
		return false;
	}

	void DrawWidgetNode(EditorContext& _context, Widget* _widget, Widget*& _toDelete)
	{
		ImGuiTreeNodeFlags _flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
		if (_widget->GetChildren().empty()) _flags |= ImGuiTreeNodeFlags_Leaf;
		if (_context.selectedWidget == _widget) _flags |= ImGuiTreeNodeFlags_Selected;

		const bool _open = ImGui::TreeNodeEx(_widget, _flags, "%s", _widget->GetName().StdString().c_str());

		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		{
			_context.selectedWidget = _widget;
			_context.selected = nullptr;
			LOG(VerbosityType::Log, "Hierarchy: selected widget " + _widget->GetName().StdString());
		}

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete"))
				_toDelete = _widget;
			ImGui::EndPopup();
		}

		if (_open)
		{
			for (Widget* _child : _widget->GetChildren())
				DrawWidgetNode(_context, _child, _toDelete);
			ImGui::TreePop();
		}
	}

	void DrawActorNode(HierarchyState& _state, Actor* _actor)
	{
		ImGuiTreeNodeFlags _flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
		if (_actor->GetChildren().empty()) _flags |= ImGuiTreeNodeFlags_Leaf;
		if (_state.context->selected == _actor) _flags |= ImGuiTreeNodeFlags_Selected;

		const bool _open = ImGui::TreeNodeEx(_actor, _flags, "%s", _actor->GetName().StdString().c_str());

		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		{
			_state.context->selected = _actor;
			_state.context->selectedWidget = nullptr;
			LOG(VerbosityType::Log, "Hierarchy: selected actor " + _actor->GetName().StdString());
		}

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload(DRAG_PAYLOAD, &_actor, sizeof(Actor*));
			ImGui::TextUnformatted(_actor->GetName().StdString().c_str());
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* _payload = ImGui::AcceptDragDropPayload(DRAG_PAYLOAD))
			{
				_state.dragged = *static_cast<Actor**>(_payload->Data);
				_state.dropTarget = _actor;
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::BeginPopupContextItem())
		{
			// The level keeps a pointer to its default camera: deleting it would leave that pointer dangling.
			if (_actor != _state.defaultCamera && ImGui::MenuItem("Delete"))
				_state.toDelete = _actor;
			if (_state.parentOf.contains(_actor) && ImGui::MenuItem("Unparent"))
			{
				_state.dragged = _actor;
				_state.dropTarget = nullptr;
			}
			ImGui::EndPopup();
		}

		if (_open)
		{
			// Copy: reparenting or deleting happens after the loop, but keep the draw safe anyway.
			const std::vector<Actor*> _children = _actor->GetChildren();
			for (Actor* _child : _children)
				DrawActorNode(_state, _child);
			ImGui::TreePop();
		}
	}
}

void Krampus::DrawHierarchyPanel(EditorContext& _context)
{
	if (ImGui::Begin("Hierarchy"))
	{
		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && Shortcuts::Pressed("Delete Selection"))
			DeleteSelection(_context);

		Level* _level = _context.GetLevel();
		if (_level)
		{
			HierarchyState _state;
			_state.context = &_context;
			if (const CameraComponent* _camera = _level->GetDefaultCamera())
				_state.defaultCamera = _camera->GetOwner();

			std::vector<Actor*> _actors;
			for (const std::unique_ptr<Actor>& _actor : _level->GetActorManager().GetActors())
				_actors.push_back(_actor.get());

			// A selection from before a level change or a delete must not be used.
			if (_context.selected && std::find(_actors.begin(), _actors.end(), _context.selected) == _actors.end())
				_context.selected = nullptr;

			for (Actor* _actor : _actors)
				for (Actor* _child : _actor->GetChildren())
					_state.parentOf[_child] = _actor;

			if (ImGui::Button("+ Add Actor"))
				ImGui::OpenPopup("AddActor");
			if (ImGui::BeginPopup("AddActor"))
			{
				for (ClassInfo* _class : ClassRegistry::GetActorClasses())
				{
					if (!ImGui::MenuItem(_class->name.c_str())) continue;

					_context.history.Record(_context, "Add " + _class->name);
					Actor* _actor = _class->factory(_level);
					LOG_MSG("Hierarchy: added actor " + _class->name);
					const CameraComponent* _camera = _level->GetDefaultCamera();
					if (_camera && _camera->GetOwner())
						_actor->SetPosition(_camera->GetOwner()->GetPosition()); // where the Viewport looks
					_context.selected = _actor;
					_context.selectedWidget = nullptr;
				}
				ImGui::EndPopup();
			}
			ImGui::Separator();

			for (Actor* _actor : _actors)
				if (!_state.parentOf.contains(_actor))
					DrawActorNode(_state, _actor);

			// HUD: the screen widgets of the level (drawn in the HUD view, not in the world).
			ImGui::Separator();
			if (ImGui::CollapsingHeader("HUD", ImGuiTreeNodeFlags_DefaultOpen))
			{
				Widget* _widgetToDelete = nullptr;

				if (ImGui::Button("+ Add Widget"))
					ImGui::OpenPopup("AddWidget");
				if (ImGui::BeginPopup("AddWidget"))
				{
					for (ClassInfo* _class : ClassRegistry::GetWidgetClasses())
					{
						if (!ImGui::MenuItem(_class->name.c_str())) continue;
						_context.history.Record(_context, "Add " + _class->name);
						_context.selectedWidget = _class->widgetFactory(_level);
						LOG_MSG("Hierarchy: added widget " + _class->name);
						_context.selected = nullptr;
					}
					ImGui::EndPopup();
				}

				std::vector<Widget*> _widgets;
				std::unordered_set<Widget*> _childWidgets;
				for (const std::unique_ptr<Widget>& _widget : _level->GetHUD().GetWidgets())
				{
					_widgets.push_back(_widget.get());
					for (Widget* _child : _widget->GetChildren())
						_childWidgets.insert(_child);
				}

				// A selection from before a level change must not be used.
				if (_context.selectedWidget && std::find(_widgets.begin(), _widgets.end(), _context.selectedWidget) == _widgets.end())
					_context.selectedWidget = nullptr;

				for (Widget* _widget : _widgets)
					if (!_childWidgets.contains(_widget))
						DrawWidgetNode(_context, _widget, _widgetToDelete);
				if (_widgets.empty())
					ImGui::TextDisabled("No widget in this level");

				if (_widgetToDelete)
				{
					_context.history.Record(_context, "Delete " + _widgetToDelete->GetName().StdString());
					LOG_MSG("Hierarchy: deleting widget " + _widgetToDelete->GetName().StdString());
					if (_context.selectedWidget == _widgetToDelete) _context.selectedWidget = nullptr;
					for (const std::unique_ptr<Widget>& _other : _level->GetHUD().GetWidgets())
						_other->RemoveChild(_widgetToDelete);
					_widgetToDelete->Destroy();
				}
			}

			// Empty area below the tree: drop here to make an actor a root again.
			const ImVec2 _rest = ImGui::GetContentRegionAvail();
			ImGui::InvisibleButton("##root_drop", ImVec2(_rest.x, std::max(_rest.y, 24.f)));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* _payload = ImGui::AcceptDragDropPayload(DRAG_PAYLOAD))
				{
					_state.dragged = *static_cast<Actor**>(_payload->Data);
					_state.dropTarget = nullptr;
				}
				ImGui::EndDragDropTarget();
			}

			if (_state.dragged)
			{
				const auto _old = _state.parentOf.find(_state.dragged);
				const bool _cycle = _state.dropTarget && IsDescendantOf(_state, _state.dropTarget, _state.dragged);
				if (!_cycle)
				{
					_context.history.Record(_context, "Reparent " + _state.dragged->GetName().StdString());
					LOG_MSG("Hierarchy: " + _state.dragged->GetName().StdString() + (_state.dropTarget ? " attached to " + _state.dropTarget->GetName().StdString() : std::string(" detached")));
					if (_old != _state.parentOf.end()) _old->second->RemoveChild(_state.dragged);
					if (_state.dropTarget) _state.dropTarget->AddChild(_state.dragged);
				}
			}

			if (_state.toDelete)
			{
				_context.history.Record(_context, "Delete " + _state.toDelete->GetName().StdString());
				LOG_MSG("Hierarchy: deleting actor " + _state.toDelete->GetName().StdString());
				const auto _parent = _state.parentOf.find(_state.toDelete);
				if (_parent != _state.parentOf.end()) _parent->second->RemoveChild(_state.toDelete);
				if (_context.selected == _state.toDelete) _context.selected = nullptr;
				_state.toDelete->Destroy();
			}
		}
	}
	ImGui::End();
}

void Krampus::DeleteSelection(EditorContext& _context)
{
	Level* _level = _context.GetLevel();
	if (!_level || _context.playing) return;

	if (_context.selected)
	{
		// The level keeps a pointer to its default camera: deleting it would leave that pointer dangling.
		const CameraComponent* _camera = _level->GetDefaultCamera();
		if (_camera && _camera->GetOwner() == _context.selected) return;

		_context.history.Record(_context, "Delete " + _context.selected->GetName().StdString());
		LOG_MSG("Editor: deleting actor " + _context.selected->GetName().StdString());
		for (const std::unique_ptr<Actor>& _actor : _level->GetActorManager().GetActors())
			_actor->RemoveChild(_context.selected);
		_context.selected->Destroy();
		_context.selected = nullptr;
	}
	else if (_context.selectedWidget)
	{
		_context.history.Record(_context, "Delete " + _context.selectedWidget->GetName().StdString());
		LOG_MSG("Editor: deleting widget " + _context.selectedWidget->GetName().StdString());
		for (const std::unique_ptr<Widget>& _widget : _level->GetHUD().GetWidgets())
			_widget->RemoveChild(_context.selectedWidget);
		_context.selectedWidget->Destroy();
		_context.selectedWidget = nullptr;
	}
}
