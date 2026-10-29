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
	constexpr const char*	DRAG_WIDGET		= "KRAMPUS_WIDGET";

	// What the HUD tree asks for during a frame (done after it is drawn).
	struct WidgetOps
	{
		Widget*								toDelete	= nullptr;
		Widget*								dragged		= nullptr;
		Widget*								dropTarget	= nullptr;	// null with dragged set = make root
		std::unordered_map<Widget*, Widget*>	parentOf;
	};

	bool IsWidgetDescendantOf(const WidgetOps& _ops, Widget* _widget, Widget* _ancestor)
	{
		for (Widget* _current = _widget; _current; )
		{
			if (_current == _ancestor) return true;
			const auto _it = _ops.parentOf.find(_current);
			_current = _it == _ops.parentOf.end() ? nullptr : _it->second;
		}
		return false;
	}

	struct HierarchyState
	{
		EditorContext*									context;
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

	void DrawWidgetNode(EditorContext& _context, Widget* _widget, WidgetOps& _ops)
	{
		ImGuiTreeNodeFlags _flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
		if (_widget->GetChildren().empty()) _flags |= ImGuiTreeNodeFlags_Leaf;
		if (_context.selectedWidget == _widget) _flags |= ImGuiTreeNodeFlags_Selected;

		const bool _open = ImGui::TreeNodeEx(_widget, _flags, "%s", _widget->GetName().StdString().c_str());

		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		{
			_context.selectedWidget = _widget;
			_context.selected = nullptr;
			LOG(VerbosityType::Verbose, "Hierarchy: selected widget " + _widget->GetName().StdString());
		}

		// Drag a widget onto another one to attach it (it then moves with it).
		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload(DRAG_WIDGET, &_widget, sizeof(Widget*));
			ImGui::Text("%s", _widget->GetName().StdString().c_str());
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* _peek = ImGui::AcceptDragDropPayload(DRAG_WIDGET, ImGuiDragDropFlags_AcceptPeekOnly | ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
			{
				Widget* _moved = *static_cast<Widget**>(_peek->Data);
				const bool _ok = !IsWidgetDescendantOf(_ops, _widget, _moved);
				ImGui::SetTooltip(_ok ? "Attach %s to %s" : "%s cannot be attached to %s (it would contain itself)", _moved->GetName().StdString().c_str(), _widget->GetName().StdString().c_str());
			}
			if (const ImGuiPayload* _payload = ImGui::AcceptDragDropPayload(DRAG_WIDGET))
			{
				_ops.dragged = *static_cast<Widget**>(_payload->Data);
				_ops.dropTarget = _widget;
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete"))
				_ops.toDelete = _widget;
			if (_ops.parentOf.contains(_widget) && ImGui::MenuItem("Unparent"))
			{
				_ops.dragged = _widget;
				_ops.dropTarget = nullptr;
			}
			ImGui::EndPopup();
		}

		if (_open)
		{
			for (Widget* _child : _widget->GetChildren())
				DrawWidgetNode(_context, _child, _ops);
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
			LOG(VerbosityType::Verbose, "Hierarchy: selected actor " + _actor->GetName().StdString());
		}

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload(DRAG_PAYLOAD, &_actor, sizeof(Actor*));
			ImGui::TextUnformatted(_actor->GetName().StdString().c_str());
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget())
		{
			// While the mouse is over this actor: say what the drop will do.
			if (const ImGuiPayload* _peek = ImGui::AcceptDragDropPayload(DRAG_PAYLOAD, ImGuiDragDropFlags_AcceptPeekOnly | ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
			{
				Actor* _moved = *static_cast<Actor**>(_peek->Data);
				const bool _ok = !IsDescendantOf(_state, _actor, _moved);
				ImGui::SetTooltip(_ok ? "Attach %s to %s" : "%s cannot be attached to %s (it would contain itself)", _moved->GetName().StdString().c_str(), _actor->GetName().StdString().c_str());
			}
			if (const ImGuiPayload* _payload = ImGui::AcceptDragDropPayload(DRAG_PAYLOAD))
			{
				_state.dragged = *static_cast<Actor**>(_payload->Data);
				_state.dropTarget = _actor;
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete"))
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
					LOG(VerbosityType::Log, "Hierarchy: added actor " + _class->name);
					_actor->SetPosition(_context.cameraCenter); // where the Viewport looks
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
			WidgetOps _widgetOps;
			if (ImGui::CollapsingHeader("HUD", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ImGui::Button("+ Add Widget"))
					ImGui::OpenPopup("AddWidget");
				if (ImGui::BeginPopup("AddWidget"))
				{
					for (ClassInfo* _class : ClassRegistry::GetWidgetClasses())
					{
						if (!ImGui::MenuItem(_class->name.c_str())) continue;
						_context.history.Record(_context, "Add " + _class->name);
						_context.selectedWidget = _class->widgetFactory(_level);
						LOG(VerbosityType::Log, "Hierarchy: added widget " + _class->name);
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
					for (Widget* _child : _widget->GetChildren())
						_widgetOps.parentOf[_child] = _widget.get();
				}

				// A selection from before a level change must not be used.
				if (_context.selectedWidget && std::find(_widgets.begin(), _widgets.end(), _context.selectedWidget) == _widgets.end())
					_context.selectedWidget = nullptr;

				for (Widget* _widget : _widgets)
					if (!_childWidgets.contains(_widget))
						DrawWidgetNode(_context, _widget, _widgetOps);
				if (_widgets.empty())
					ImGui::TextDisabled("No widget in this level");

				if (_widgetOps.toDelete)
				{
					Widget* _widgetToDelete = _widgetOps.toDelete;
					_context.history.Record(_context, "Delete " + _widgetToDelete->GetName().StdString());
					LOG(VerbosityType::Log, "Hierarchy: deleting widget " + _widgetToDelete->GetName().StdString());
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
				if (const ImGuiPayload* _widgetPayload = ImGui::AcceptDragDropPayload(DRAG_WIDGET))
				{
					_widgetOps.dragged = *static_cast<Widget**>(_widgetPayload->Data);
					_widgetOps.dropTarget = nullptr;
				}
				ImGui::EndDragDropTarget();
			}

			// Widget dropped on a widget (attach) or on the empty area (detach).
			if (_widgetOps.dragged)
			{
				Widget* _moved = _widgetOps.dragged;
				const auto _oldParent = _widgetOps.parentOf.find(_moved);
				Widget* _currentParent = _oldParent == _widgetOps.parentOf.end() ? nullptr : _oldParent->second;
				const bool _cycle = _widgetOps.dropTarget && IsWidgetDescendantOf(_widgetOps, _widgetOps.dropTarget, _moved);
				if (!_cycle && _currentParent != _widgetOps.dropTarget)
				{
					_context.history.Record(_context, "Reparent " + _moved->GetName().StdString());
					LOG(VerbosityType::Log, "Hierarchy: widget " + _moved->GetName().StdString() + (_widgetOps.dropTarget ? " attached to " + _widgetOps.dropTarget->GetName().StdString() : std::string(" detached")));
					if (_currentParent) _currentParent->RemoveChild(_moved);
					if (_widgetOps.dropTarget) _widgetOps.dropTarget->AddChild(_moved);
				}
			}

			if (_state.dragged)
			{
				const auto _old = _state.parentOf.find(_state.dragged);
				const bool _cycle = _state.dropTarget && IsDescendantOf(_state, _state.dropTarget, _state.dragged);
				Actor* _currentParent = _old == _state.parentOf.end() ? nullptr : _old->second;
				if (!_cycle && _currentParent != _state.dropTarget)	// already attached there: nothing to do
				{
					_context.history.Record(_context, "Reparent " + _state.dragged->GetName().StdString());
					LOG(VerbosityType::Log, "Hierarchy: " + _state.dragged->GetName().StdString() + (_state.dropTarget ? " attached to " + _state.dropTarget->GetName().StdString() : std::string(" detached")));
					if (_old != _state.parentOf.end()) _old->second->RemoveChild(_state.dragged);
					if (_state.dropTarget) _state.dropTarget->AddChild(_state.dragged);
				}
			}

			if (_state.toDelete)
			{
				_context.history.Record(_context, "Delete " + _state.toDelete->GetName().StdString());
				LOG(VerbosityType::Log, "Hierarchy: deleting actor " + _state.toDelete->GetName().StdString());
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
		_context.history.Record(_context, "Delete " + _context.selected->GetName().StdString());
		LOG(VerbosityType::Log, "Editor: deleting actor " + _context.selected->GetName().StdString());
		for (const std::unique_ptr<Actor>& _actor : _level->GetActorManager().GetActors())
			_actor->RemoveChild(_context.selected);
		_context.selected->Destroy();
		_context.selected = nullptr;
	}
	else if (_context.selectedWidget)
	{
		_context.history.Record(_context, "Delete " + _context.selectedWidget->GetName().StdString());
		LOG(VerbosityType::Log, "Editor: deleting widget " + _context.selectedWidget->GetName().StdString());
		for (const std::unique_ptr<Widget>& _widget : _level->GetHUD().GetWidgets())
			_widget->RemoveChild(_context.selectedWidget);
		_context.selectedWidget->Destroy();
		_context.selectedWidget = nullptr;
	}
}
