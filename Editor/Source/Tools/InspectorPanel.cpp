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

namespace Krampus::InspectorInternal
{
	bool g_propertyEdited = false;	// set by DrawProperty, read by DrawObjectProperties
	EditorContext* g_historyContext = nullptr;	// set while the Inspector draws: its edits can be undone (the Settings page edits no scene object)

	void RecordEdit(const std::string& _label, const std::string& _key)
	{
		if (g_historyContext) g_historyContext->history.Record(*g_historyContext, _label, _key);
	}

	std::string KeyOf(const void* _object, const std::string& _what)
	{
		return std::to_string(reinterpret_cast<std::uintptr_t>(_object)) + "." + _what;
	}

	std::string ValueText(const PropertyValue& _value)
	{
		return std::visit([](const auto& _v) -> std::string
		{
			using V = std::decay_t<decltype(_v)>;
			if constexpr (std::is_same_v<V, bool>)			return _v ? "true" : "false";
			else if constexpr (std::is_same_v<V, int>)		return std::to_string(_v);
			else if constexpr (std::is_same_v<V, float>)	return std::to_string(_v);
			else if constexpr (std::is_same_v<V, std::string>) return "\"" + _v + "\"";
			else if constexpr (std::is_same_v<V, FVector2>)	return "(" + std::to_string(_v.x) + ", " + std::to_string(_v.y) + ")";
			else										return "rgba(" + std::to_string(_v.r) + ", " + std::to_string(_v.g) + ", " + std::to_string(_v.b) + ", " + std::to_string(_v.a) + ")";
		}, _value);
	}

	// ---- Event lists (like Unity events): which function of which object is called when something happens.
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
				? EditorUI::SliderFloat(_property.name.c_str(), &std::get<float>(_value), _property.min, _property.max)
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
		case PropertyType::MessageBindings:
			_changed = DrawInputBindings(_property, _object, std::get<std::string>(_value), true);
			break;
		case PropertyType::InputBindings:
			_changed = DrawInputBindings(_property, _object, std::get<std::string>(_value));
			break;
		case PropertyType::Events:
			_changed = DrawEventList(_property, _object, std::get<std::string>(_value));
			break;
		case PropertyType::GameplayTag:
			_changed = DrawGameplayTagPicker(_property.name.c_str(), std::get<std::string>(_value));
			break;
		case PropertyType::GameplayTagContainer:
			_changed = DrawGameplayTagContainerEditor(_property.name.c_str(), std::get<std::string>(_value));
			break;
		case PropertyType::GameplayTagQuery:
			_changed = DrawGameplayTagQueryEditor(_property.name.c_str(), std::get<std::string>(_value));
			break;
		case PropertyType::Enum:
		{
			int& _index = std::get<int>(_value);
			const int _total = static_cast<int>(_property.flagNames.size());
			const char* _current = (_index >= 0 && _index < _total) ? _property.flagNames[_index].c_str() : "?";
			if (ImGui::BeginCombo(_property.name.c_str(), _current))
			{
				for (int _i = 0; _i < _total; ++_i)
				{
					if (ImGui::Selectable(_property.flagNames[_i].c_str(), _i == _index))
					{
						_index = _i;
						_changed = true;
					}
				}
				ImGui::EndCombo();
			}
			break;
		}
		case PropertyType::Scene:
			_changed = DrawScenePicker(_property.name.c_str(), std::get<std::string>(_value));
			break;
		case PropertyType::Flags:
			_changed = DrawFlags(_property, std::get<int>(_value));
			break;
		case PropertyType::Texture:
			// A name in flagNames: the property may be empty, shown with that name (a particle without texture is a soft dot).
			_changed = DrawAssetPicker(_property.name.c_str(), std::get<std::string>(_value), AssetKind::Texture, _property.flagNames.empty() ? nullptr : _property.flagNames[0].c_str());
			break;
		case PropertyType::Font:
			_changed = DrawAssetPicker(_property.name.c_str(), std::get<std::string>(_value), AssetKind::Font);
			break;
		case PropertyType::Sound:
			_changed = DrawAssetPicker(_property.name.c_str(), std::get<std::string>(_value), AssetKind::Sound);
			break;
		case PropertyType::Shape:
			_changed = DrawShapePicker(_property.name.c_str(), std::get<std::string>(_value));
			break;
		case PropertyType::ActorRef:
		case PropertyType::ComponentRef:
			_changed = DrawReferencePicker(_property, _object, std::get<std::string>(_value));
			break;
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
		{
			g_propertyEdited = true;
			RecordEdit("Edit " + _property.name, KeyOf(&_object, _property.name));
			LOG(VerbosityType::Verbose, "Inspector: " + _object.GetName() + "." + _property.name + " = " + ValueText(_value));
			_property.set(&_object, _value);
		}
	}

	void DrawProperties(IGameObject& _object)
	{
		const ClassInfo* _class = ClassRegistry::Find(_object.ClassName());
		if (!_class) return;

		for (const PropertyInfo* _property : _class->GetProperties())
		{
			if (_property->visible && !_property->visible(&_object)) continue;	// e.g. the radius of a rectangle
			DrawProperty(*_property, _object);
		}
	}

	// A child shows (and edits) its place in its parent, like the gizmo moves it in the world.
	void DrawTransform(ITransformable& _actor)
	{
		const bool _local = _actor.HasParentTransform();
		Transform _transform = _actor.GetLocalTransform();
		if (_local) ImGui::TextDisabled("In the parent (local)");

		float _positionValues[2] = { _transform.position.x, _transform.position.y };
		if (ImGui::DragFloat2("Position", _positionValues, 0.5f))
		{
			RecordEdit("Move", KeyOf(&_actor, "position"));
			_transform.position = FVector2(_positionValues[0], _positionValues[1]);
			_actor.SetLocalTransform(_transform);
		}

		float _rotation = _transform.rotation.ToDegrees();
		if (ImGui::DragFloat("Rotation", &_rotation, 0.5f))
		{
			RecordEdit("Rotate", KeyOf(&_actor, "rotation"));
			_transform.rotation = Angle::FromDegrees(_rotation);
			_actor.SetLocalTransform(_transform);
		}

		float _scaleValues[2] = { _transform.scale.x, _transform.scale.y };
		if (ImGui::DragFloat2("Scale", _scaleValues, 0.01f))
		{
			RecordEdit("Scale", KeyOf(&_actor, "scale"));
			_transform.scale = FVector2(_scaleValues[0], _scaleValues[1]);
			_actor.SetLocalTransform(_transform);
		}
		if (_local && ImGui::IsItemHovered()) ImGui::SetTooltip("World: %.1f, %.1f  rotation %.1f", _actor.GetPosition().x, _actor.GetPosition().y, static_cast<float>(_actor.GetRotation().ToDegrees()));
	}
}

bool Krampus::DrawObjectProperties(IGameObject& _object)
{
	g_propertyEdited = false;
	DrawProperties(_object);
	return g_propertyEdited;
}
