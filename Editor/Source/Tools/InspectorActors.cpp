#include "EditorContext.h"
#include "EditorWidgets.h"
#include "EditorThumbnails.h"
#include "Reflection/Reflection.h"
#include "Graphics/Resources/Animation.h"
#include "UI/Widgets/Widget.h"
#include "Actors/Actor.h"
#include "Components/InputComponent.h"
#include "Managers/InputManager.h"
#include "Graphics/Shapes/ShapeAssets.h"
#include "Graphics/Shaders/ShaderMaterial.h"
#include "EditorTheme.h"
#include "GameplayTagWidgets.h"
#include "Components/GameplayTagComponent.h"
#include <imgui.h>
#include "InspectorInternal.h"

using namespace Krampus;
using namespace Krampus::InspectorInternal;

namespace
{
	template <typename Owner>
	void DrawNameAndActive(Owner& _owner)
	{
		char _name[128];
		std::snprintf(_name, sizeof(_name), "%s", _owner.GetName().c_str());
		if (ImGui::InputText("Name", _name, sizeof(_name)))
			{
				RecordEdit("Rename", KeyOf(&_owner, "name"));
				_owner.SetName(String(_name));
			}

		bool _active = _owner.IsActive();
		if (ImGui::Checkbox("Active", &_active))
		{
			LOG(VerbosityType::Verbose, "Inspector: " + _owner.GetName() + (_active ? " activated" : " deactivated"));
			{
				RecordEdit(_active ? "Activate" : "Deactivate");
				_owner.SetActive(_active);
			}
		}
	}

	void DrawClassProperties(IGameObject& _object)
	{
		if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen))
		{
			DrawProperties(_object);
			const ClassInfo* _class = ClassRegistry::Find(ClassNameOf(_object));
			if (!_class || _class->GetProperties().empty())
				ImGui::TextDisabled("No KPROPERTY on this class");
		}
	}

	// Material: the shader of what is drawn, and its parameters (read from the shader file: the Inspector shows what it declares).
	void DrawMaterial(EditorContext& _context, IShaded& _shaded)
	{
		ShaderMaterial& _material = _shaded.material;
		ImGui::SeparatorText("Material");
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo("##shader", _material.shader.empty() ? "(no shader)" : _material.shader.c_str(), ImGuiComboFlags_HeightLarge))
		{
			if (ImGui::Selectable("(no shader)", _material.shader.empty())) { _context.history.Record(_context, "Set shader"); _material.shader.clear(); }
			for (const std::string& _name : ShaderLibrary::List())
				if (ImGui::Selectable(_name.c_str(), _name == _material.shader)) { _context.history.Record(_context, "Set shader"); _material.shader = _name; }
			ImGui::EndCombo();
		}
		if (_material.shader.empty()) return;

		const ShaderAsset* _asset = ShaderLibrary::Get(_material.shader);
		if (!_asset) { ImGui::TextColored(EditorTheme::Get(EditorTheme::Custom::ConsoleWarning), "No file %s", ShaderLibrary::FragmentPath(_material.shader).generic_string().c_str()); return; }
		if (!_asset->IsReady())
		{
			ImGui::TextColored(EditorTheme::Get(EditorTheme::Custom::ConsoleError), "Does not compile (drawn without it)");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", _asset->error.c_str());
		}
		for (const ShaderParam& _param : _asset->params)
		{
			std::string _value = _material.ValueOf(_param);
			if (DrawShaderParam(_param, _value))
			{
				_context.history.Record(_context, "Edit material", "material." + std::to_string(reinterpret_cast<std::uintptr_t>(&_shaded)) + "." + _param.name);
				_material.values[_param.name] = _value;
			}
		}
		const float _half = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
		if (ImGui::Button("Open in Shader Editor", ImVec2(_half, 0.f))) OpenShaderEditor(_material.shader);
		ImGui::SameLine();
		ImGui::BeginDisabled(_material.values.empty());
		if (ImGui::Button("Default values", ImVec2(-1.f, 0.f))) { _context.history.Record(_context, "Reset material"); _material.values.clear(); }
		ImGui::EndDisabled();
	}

	// Animation components: the list of their animations is edited in the Animation Editor; here one is picked as the starting one.
	void DrawStartAnimation(IGameObject& _component)
	{
		const ClassInfo* _class = ClassRegistry::Find(_component.ClassName());
		if (!_class) return;
		const PropertyInfo* _list = nullptr;
		const PropertyInfo* _start = nullptr;
		for (const PropertyInfo* _property : _class->GetProperties())
		{
			if (_property->name == "animations") _list = _property;
			else if (_property->name == "startAnimation") _start = _property;
		}
		if (!_list || !_start) return;

		const std::vector<AnimationClip> _clips = AnimationClips::Decode(std::get<std::string>(_list->get(&_component)));
		const std::string _current = std::get<std::string>(_start->get(&_component));
		const std::string _shown = _clips.empty() ? "(no animation)" : (_current.empty() ? _clips.front().name + " (first)" : _current);

		if (ImGui::BeginCombo("Start animation", _shown.c_str()))
		{
			for (const AnimationClip& _clip : _clips)
			{
				const std::string _name = _clip.name;
				if (!ImGui::Selectable(_name.c_str(), _name == _current)) continue;
				RecordEdit("Edit startAnimation", KeyOf(&_component, "startAnimation"));
				_start->set(&_component, PropertyValue(_name));
			}
			ImGui::EndCombo();
		}
	}

	// One collapsible header per component of an actor or a widget. Right click: Remove Component.
	// Returns the component to remove (done by the caller once the list is no longer walked).
	template <typename C>
	C* DrawComponents(const std::vector<std::unique_ptr<C>>& _components)
	{
		C* _toRemove = nullptr;
		for (const std::unique_ptr<C>& _component : _components)
		{
			ImGui::PushID(_component.get());

			const std::string _title = _component->GetName() + "  (" + ClassNameOf(*_component) + ")";
			const bool _open = ImGui::CollapsingHeader(_title.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Remove Component"))
					_toRemove = _component.get();
				ImGui::EndPopup();
			}

			if (_open)
			{
				bool _componentActive = _component->IsActive();
				if (ImGui::Checkbox("Active", &_componentActive))
					_component->SetActive(_componentActive);
				DrawProperties(*_component);
				if (ClassNameOf(*_component).starts_with("Animation")) DrawStartAnimation(*_component);
				if (IShaded* _shaded = dynamic_cast<IShaded*>(_component.get()); _shaded && g_historyContext) DrawMaterial(*g_historyContext, *_shaded);
				if (ClassNameOf(*_component).starts_with("Animation") && ImGui::Button("Open Animation Editor", ImVec2(-1.f, 0.f)))
					OpenAnimationEditor();
				if (ClassNameOf(*_component) == "AnimationComponentSM" && ImGui::Button("Open Animation Graph (transitions)", ImVec2(-1.f, 0.f)))
					OpenAnimationGraph();
			}

			ImGui::PopID();
		}
		return _toRemove;
	}

	template <typename C>
	bool HasComponentOfClass(const std::vector<std::unique_ptr<C>>& _components, const std::string& _className)
	{
		for (const std::unique_ptr<C>& _component : _components)
			if (ClassNameOf(*_component) == _className)
				return true;
		return false;
	}

	void DrawAddComponent(Actor& _actor)
	{
		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");
		if (ImGui::BeginPopup("AddComponent"))
		{
			for (ClassInfo* _class : ClassRegistry::GetComponentClasses())
			{
				const bool _already = HasComponentOfClass(_actor.GetComponents(), _class->name);	// one per type
				if (ImGui::MenuItem(_class->name.c_str(), nullptr, false, !_already))
				{
					LOG(VerbosityType::Log, "Inspector: adding " + _class->name + " to " + _actor.GetName());
					RecordEdit("Add " + _class->name);
					_class->componentFactory(&_actor);
				}
			}
			ImGui::EndPopup();
		}
	}

	void DrawAddComponent(Widget& _widget)
	{
		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");
		if (ImGui::BeginPopup("AddComponent"))
		{
			for (ClassInfo* _class : ClassRegistry::GetWidgetComponentClasses())
			{
				const bool _already = HasComponentOfClass(_widget.GetComponents(), _class->name);
				if (ImGui::MenuItem(_class->name.c_str(), nullptr, false, !_already))
				{
					LOG(VerbosityType::Log, "Inspector: adding " + _class->name + " to widget " + _widget.GetName());
					RecordEdit("Add " + _class->name);
					_class->widgetComponentFactory(&_widget);
				}
			}
			ImGui::EndPopup();
		}
	}

	void DrawActorInspector(Actor& _actor)
	{
		ImGui::PushID(&_actor);

		DrawNameAndActive(_actor);
		ImGui::TextDisabled("Class: %s", ClassNameOf(_actor).c_str());
		ImGui::Separator();

		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			DrawTransform(_actor);
		DrawClassProperties(_actor);

		if (Component* _toRemove = DrawComponents(_actor.GetComponents()))
			{
				LOG(VerbosityType::Log, "Inspector: removing " + ClassNameOf(*_toRemove) + " from " + _actor.GetName());
				RecordEdit("Remove " + ClassNameOf(*_toRemove));
				_actor.RemoveComponentInstance(_toRemove);
			}

		ImGui::Separator();
		DrawAddComponent(_actor);

		ImGui::PopID();
	}

	// Same panels as an actor: name, active, transform, reflected properties, then the widget components.
	void DrawWidgetInspector(Widget& _widget)
	{
		ImGui::PushID(&_widget);

		DrawNameAndActive(_widget);
		ImGui::TextDisabled("HUD widget  -  class: %s", ClassNameOf(_widget).c_str());
		ImGui::Separator();

		if (ImGui::CollapsingHeader("Transform (HUD view)", ImGuiTreeNodeFlags_DefaultOpen))
			DrawTransform(_widget);
		DrawClassProperties(_widget);

		if (WidgetComponent* _toRemove = DrawComponents(_widget.GetComponents()))
			{
				LOG(VerbosityType::Log, "Inspector: removing " + ClassNameOf(*_toRemove) + " from widget " + _widget.GetName());
				RecordEdit("Remove " + ClassNameOf(*_toRemove));
				_widget.RemoveComponentInstance(_toRemove);
			}

		ImGui::Separator();
		DrawAddComponent(_widget);

		ImGui::PopID();
	}
}

void Krampus::DrawInspectorPanel(EditorContext& _context)
{
	g_historyContext = &_context;
	if (ImGui::Begin("Inspector"))
	{
		if (_context.selectedWidget)
			DrawWidgetInspector(*_context.selectedWidget);
		else if (_context.selected)
			DrawActorInspector(*_context.selected);
		else
			ImGui::TextDisabled("Select an actor or a HUD widget in the Hierarchy");
	}
	ImGui::End();
	g_historyContext = nullptr;
}
