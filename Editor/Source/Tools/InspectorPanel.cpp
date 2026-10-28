#include "EditorContext.h"
#include "Reflection/Reflection.h"
#include "UI/Widgets/Widget.h"
#include <imgui.h>

using namespace Krampus;

namespace
{
	// Draws the widget of one property and writes the new value back when it changes.
	void DrawProperty(const PropertyInfo& _property, IGameObject& _object)
	{
		PropertyValue _value = _property.get(&_object);
		bool _changed = false;
		const bool _limited = _property.min != _property.max;

		ImGui::BeginDisabled(_property.readOnly);
		switch (_property.type)
		{
		case PropertyType::Bool:
			_changed = ImGui::Checkbox(_property.name.c_str(), &std::get<bool>(_value));
			break;
		case PropertyType::Int:
			_changed = ImGui::DragInt(_property.name.c_str(), &std::get<int>(_value), 1.f,
				static_cast<int>(_property.min), static_cast<int>(_property.max));
			break;
		case PropertyType::Float:
			_changed = _limited
				? ImGui::SliderFloat(_property.name.c_str(), &std::get<float>(_value), _property.min, _property.max)
				: ImGui::DragFloat(_property.name.c_str(), &std::get<float>(_value), 0.1f);
			break;
		case PropertyType::String:
		{
			char _buffer[256];
			std::snprintf(_buffer, sizeof(_buffer), "%s", std::get<std::string>(_value).c_str());
			// Applied on Enter or when the field loses focus, so a path is not loaded at every keystroke.
			const bool _edited = ImGui::InputText(_property.name.c_str(), _buffer, sizeof(_buffer), ImGuiInputTextFlags_EnterReturnsTrue);
			if (_edited || ImGui::IsItemDeactivatedAfterEdit())
			{
				std::get<std::string>(_value) = _buffer;
				_changed = true;
			}
			break;
		}
		case PropertyType::Vector2:
		{
			FVector2& _vector = std::get<FVector2>(_value);
			float _components[2] = { _vector.x, _vector.y };
			if (ImGui::DragFloat2(_property.name.c_str(), _components, 0.1f))
			{
				_vector = FVector2(_components[0], _components[1]);
				_changed = true;
			}
			break;
		}
		case PropertyType::Color:
		{
			Color& _color = std::get<Color>(_value);
			float _components[4] = { _color.r / 255.f, _color.g / 255.f, _color.b / 255.f, _color.a / 255.f };
			if (ImGui::ColorEdit4(_property.name.c_str(), _components))
			{
				_color = Color(
					static_cast<std::uint8_t>(_components[0] * 255.f + 0.5f), static_cast<std::uint8_t>(_components[1] * 255.f + 0.5f),
					static_cast<std::uint8_t>(_components[2] * 255.f + 0.5f), static_cast<std::uint8_t>(_components[3] * 255.f + 0.5f));
				_changed = true;
			}
			break;
		}
		}
		ImGui::EndDisabled();

		if (_changed && !_property.readOnly)
			_property.set(&_object, _value);
	}

	void DrawProperties(IGameObject& _object)
	{
		const ClassInfo* _class = ClassRegistry::Find(_object.ClassName());
		if (!_class) return;

		for (const PropertyInfo* _property : _class->GetProperties())
			DrawProperty(*_property, _object);
	}

	void DrawTransform(ITransformable& _actor)
	{
		FVector2 _position = _actor.GetPosition();
		float _positionValues[2] = { _position.x, _position.y };
		if (ImGui::DragFloat2("Position", _positionValues, 0.5f))
			_actor.SetPosition(FVector2(_positionValues[0], _positionValues[1]));

		float _rotation = _actor.GetRotation().ToDegrees();
		if (ImGui::DragFloat("Rotation", &_rotation, 0.5f))
			_actor.SetRotation(Angle::FromDegrees(_rotation));

		FVector2 _scale = _actor.GetScale();
		float _scaleValues[2] = { _scale.x, _scale.y };
		if (ImGui::DragFloat2("Scale", _scaleValues, 0.01f))
			_actor.SetScale(FVector2(_scaleValues[0], _scaleValues[1]));
	}
}

namespace
{
	template <typename Owner>
	void DrawNameAndActive(Owner& _owner)
	{
		char _name[128];
		std::snprintf(_name, sizeof(_name), "%s", _owner.GetName().StdString().c_str());
		if (ImGui::InputText("Name", _name, sizeof(_name)))
			_owner.SetName(String(_name));

		bool _active = _owner.IsActive();
		if (ImGui::Checkbox("Active", &_active))
			_owner.SetActive(Bool(_active));
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

	// One collapsible header per component of an actor or a widget. Right click: Remove Component.
	// Returns the component to remove (done by the caller once the list is no longer walked).
	template <typename C>
	C* DrawComponents(const std::vector<std::unique_ptr<C>>& _components)
	{
		C* _toRemove = nullptr;
		for (const std::unique_ptr<C>& _component : _components)
		{
			ImGui::PushID(_component.get());

			const std::string _title = _component->GetName().StdString() + "  (" + ClassNameOf(*_component) + ")";
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
					_component->SetActive(Bool(_componentActive));
				DrawProperties(*_component);
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
					_class->componentFactory(&_actor);
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
					_class->widgetComponentFactory(&_widget);
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
			_actor.RemoveComponentInstance(_toRemove);

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
			_widget.RemoveComponentInstance(_toRemove);

		ImGui::Separator();
		DrawAddComponent(_widget);

		ImGui::PopID();
	}
}

void Krampus::DrawInspectorPanel(EditorContext& _context)
{
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
}
