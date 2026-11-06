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
	PropertyValue DefaultArgument(const PropertyType& _type)
	{
		switch (_type)
		{
		case PropertyType::Bool:	return false;
		case PropertyType::Int:		return 0;
		case PropertyType::Float:	return 0.f;
		case PropertyType::Vector2:	return FVector2(0.f, 0.f);
		case PropertyType::Color:	return Color(255, 255, 255, 255);
		default:					return std::string();
		}
	}

	std::string TargetLabel(const std::string& _target)
	{
		if (_target.empty()) return "Self";
		if (_target == "Game") return "Game";
			if (_target.starts_with("T:")) return "Tag: " + _target.substr(2);
		return (_target.starts_with("A:") ? "Actor: " : "Widget: ") + _target.substr(2);
	}

	// The argument of a function, edited with the widget of its type. Returns true when it changed.
	bool DrawArgument(const FunctionInfo& _function, PropertyValue& _value)
	{
		const char* _label = "Argument";
			if (_function.argumentType == PropertyType::GameplayTag)
				return DrawGameplayTagPicker(_label, std::get<std::string>(_value));
		bool _changed = false;
		if (_function.argumentType == PropertyType::Scene)
		{
			std::string& _scene = std::get<std::string>(_value);
			if (ImGui::BeginCombo(_label, _scene.empty() ? "(choose a scene)" : _scene.c_str()))
			{
				std::error_code _error;
				for (const auto& _entry : std::filesystem::directory_iterator(SCENES_DIRECTORY, _error))
					if (_entry.path().extension() == ".ka7" && ImGui::Selectable(_entry.path().stem().string().c_str(), _entry.path().stem().string() == _scene))
					{
						_scene = _entry.path().stem().string();
						_changed = true;
					}
				ImGui::EndCombo();
			}
			return _changed;
		}

		if (std::holds_alternative<bool>(_value))
			_changed = ImGui::Checkbox(_label, &std::get<bool>(_value));
		else if (std::holds_alternative<int>(_value))
			_changed = ImGui::DragInt(_label, &std::get<int>(_value));
		else if (std::holds_alternative<float>(_value))
			_changed = ImGui::DragFloat(_label, &std::get<float>(_value), 0.1f);
		else if (std::holds_alternative<FVector2>(_value))
		{
			FVector2& _vector = std::get<FVector2>(_value);
			float _values[2] = { _vector.x, _vector.y };
			if (ImGui::DragFloat2(_label, _values, 0.1f))
			{
				_vector = FVector2(_values[0], _values[1]);
				_changed = true;
			}
		}
		else if (std::holds_alternative<Color>(_value))
		{
			Color& _color = std::get<Color>(_value);
			float _values[4] = { _color.r / 255.f, _color.g / 255.f, _color.b / 255.f, _color.a / 255.f };
			if (ImGui::ColorEdit4(_label, _values))
			{
				_color = Color(static_cast<std::uint8_t>(_values[0] * 255.f + 0.5f), static_cast<std::uint8_t>(_values[1] * 255.f + 0.5f),
					static_cast<std::uint8_t>(_values[2] * 255.f + 0.5f), static_cast<std::uint8_t>(_values[3] * 255.f + 0.5f));
				_changed = true;
			}
		}
		else
		{
			char _buffer[256];
			std::snprintf(_buffer, sizeof(_buffer), "%s", std::get<std::string>(_value).c_str());
			if (ImGui::InputText(_label, _buffer, sizeof(_buffer)))
			{
				std::get<std::string>(_value) = _buffer;
				_changed = true;
			}
		}
		return _changed;
	}

	// One block per call: Target, Component, Function, Argument and a button to remove it. Returns true when the list changed.
	bool DrawEventList(const PropertyInfo& _property, IGameObject& _owner, std::string& _text)
	{
		EventList _list = EventList::Decode(_text);
		bool _changed = false;
		Level* _level = g_historyContext ? g_historyContext->GetLevel() : nullptr;

		if (ImGui::TreeNodeEx(_property.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen, "%s  (%d)", _property.name.c_str(), static_cast<int>(_list.calls.size())))
		{
			int _remove = -1;
			for (int _i = 0; _i < static_cast<int>(_list.calls.size()); ++_i)
			{
				EventCall& _call = _list.calls[_i];
				ImGui::PushID(_i);
				ImGui::Separator();

				// Target
				if (ImGui::BeginCombo("Target", TargetLabel(_call.target).c_str()))
				{
					const auto _pick = [&](const std::string& _target)
					{
						if (!ImGui::Selectable(TargetLabel(_target).c_str(), _target == _call.target)) return;
						_call.target = _target;
						_call.component.clear();
						_call.function.clear();
						_changed = true;
					};
					_pick("");
					_pick("Game");
						for (const GameplayTag& _tag : GameplayTagsManager::GetAllTags())
							_pick("T:" + _tag.GetName());
					if (_level)
					{
						for (const std::unique_ptr<Actor>& _actor : _level->GetActorManager().GetActors())
							_pick("A:" + _actor->GetName());
						for (const std::unique_ptr<Widget>& _widget : _level->GetHUD().GetWidgets())
							_pick("W:" + _widget->GetName());
					}
					ImGui::EndCombo();
				}

				// Component: the actor / widget itself, or one of its components.
				if (_call.target != "Game")
				{
					EventCall _root = _call;
					_root.component.clear();
					IGameObject* _rootObject = EventList::Resolve(&_owner, _root);
					if (ImGui::BeginCombo("Component", _call.component.empty() ? "(the object itself)" : _call.component.c_str()))
					{
						if (ImGui::Selectable("(the object itself)", _call.component.empty()))
						{
							_call.component.clear();
							_call.function.clear();
							_changed = true;
						}
						const auto _addComponents = [&](const auto& _components)
						{
							for (const auto& _component : _components)
								if (ImGui::Selectable(ClassNameOf(*_component).c_str(), ClassNameOf(*_component) == _call.component))
								{
									_call.component = ClassNameOf(*_component);
									_call.function.clear();
									_changed = true;
								}
						};
						if (Actor* _actor = dynamic_cast<Actor*>(_rootObject)) _addComponents(_actor->GetComponents());
						else if (Widget* _widget = dynamic_cast<Widget*>(_rootObject)) _addComponents(_widget->GetComponents());
							else if (_call.target.starts_with("T:"))	// nobody has the tag in this scene: every component class
								for (const ClassInfo* _class : ClassRegistry::GetComponentClasses())
									if (ImGui::Selectable(_class->name.c_str(), _class->name == _call.component))
									{
										_call.component = _class->name;
										_call.function.clear();
										_changed = true;
									}
						ImGui::EndCombo();
					}
				}

				// Function
				IGameObject* _object = EventList::Resolve(&_owner, _call);
					const bool _byTag = _call.target.starts_with("T:");
				if (!_object && !_byTag)
					ImGui::TextColored(ImVec4(1.f, 0.45f, 0.3f, 1.f), "Target not found in this scene");
				else
				{
					std::string _className = _call.target == "Game" ? std::string("Game") : (_object ? ClassNameOf(*_object) : std::string());
						if (!_object && _call.target != "Game") _className = _call.component.empty() ? "Actor" : _call.component;	// a tag nobody has yet: the class is enough
						const ClassInfo* _class = ClassRegistry::Find(_className);
					const std::vector<const FunctionInfo*> _functions = _class ? _class->GetFunctions() : std::vector<const FunctionInfo*>{ };
					const FunctionInfo* _current = nullptr;
					for (const FunctionInfo* _function : _functions)
						if (_function->name == _call.function) _current = _function;

					if (ImGui::BeginCombo("Function", _call.function.empty() ? "(choose a function)" : _call.function.c_str()))
					{
						for (const FunctionInfo* _function : _functions)
							if (ImGui::Selectable(_function->name.c_str(), _function == _current))
							{
								_call.function = _function->name;
								_call.argument = DefaultArgument(_function->argumentType);
								_changed = true;
							}
						if (_functions.empty()) ImGui::TextDisabled("No function on this object (see KFUNCTION)");
						ImGui::EndCombo();
					}
					if (!_call.function.empty() && !_current)
						ImGui::TextColored(ImVec4(1.f, 0.45f, 0.3f, 1.f), "\"%s\" does not exist on this object", _call.function.c_str());
					if (_current && _current->hasArgument)
					{
						if (_call.argument.index() != DefaultArgument(_current->argumentType).index())
							_call.argument = DefaultArgument(_current->argumentType);
						if (DrawArgument(*_current, _call.argument)) _changed = true;
					}
				}

				if (ImGui::SmallButton("Remove")) _remove = _i;
				ImGui::PopID();
			}
			if (_remove >= 0)
			{
				_list.calls.erase(_list.calls.begin() + _remove);
				_changed = true;
			}

			if (!_list.calls.empty()) ImGui::Separator();
			if (ImGui::Button("+ Add call"))
			{
				_list.calls.push_back(EventCall{ });
				_changed = true;
			}
			ImGui::TreePop();
		}

		if (_changed) _text = _list.Encode();
		return _changed;
	}

	// Input events of an InputComponent ("A.Press", "MouseWheelScroll"...), each with its own list of function calls.
	// _messages: the names are network messages typed by the user (NetworkEventsComponent) instead of input events chosen in a list.
	bool DrawInputBindings(const PropertyInfo& _property, IGameObject& _owner, std::string& _text, const bool _messages)
	{
		std::vector<InputComponent::Binding> _bindings = InputComponent::DecodeBindings(_text);
		bool _changed = false;

		if (ImGui::TreeNodeEx(_property.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen, "%s  (%d)", _property.name.c_str(), static_cast<int>(_bindings.size())))
		{
			int _remove = -1;
			for (int _i = 0; _i < static_cast<int>(_bindings.size()); ++_i)
			{
				ImGui::PushID(_i);
				ImGui::Separator();

				PropertyInfo _list;
				_list.name = _bindings[_i].source;
				std::string _calls = _bindings[_i].calls.Encode();
				if (DrawEventList(_list, _owner, _calls))
				{
					_bindings[_i].calls = EventList::Decode(_calls);
					_changed = true;
				}
				if (ImGui::SmallButton("Remove input")) _remove = _i;
				ImGui::PopID();
			}
			if (_remove >= 0)
			{
				_bindings.erase(_bindings.begin() + _remove);
				_changed = true;
			}

			if (!_bindings.empty()) ImGui::Separator();
			if (ImGui::Button("+ Add input"))
				ImGui::OpenPopup(_messages ? "AddMessage" : "AddInput");
			if (_messages && ImGui::BeginPopup("AddMessage"))
			{
				static char _messageName[64] = "";
				ImGui::SetNextItemWidth(200.f);
				ImGui::InputTextWithHint("##message", "Message name", _messageName, sizeof(_messageName));
				const bool _used = std::any_of(_bindings.begin(), _bindings.end(), [&](const InputComponent::Binding& _binding) { return _binding.source == _messageName; });
				if (ImGui::Button("Add") && _messageName[0] && !_used)
				{
					_bindings.push_back(InputComponent::Binding{ _messageName, EventList() });
					_changed = true;
					_messageName[0] = 0;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
			if (!_messages && ImGui::BeginPopup("AddInput"))
			{
				static char _filter[64] = "";
				ImGui::SetNextItemWidth(220.f);
				ImGui::InputTextWithHint("##filter", "Filter (A, Mouse, Press...)", _filter, sizeof(_filter));
				ImGui::BeginChild("InputList", ImVec2(260.f, 300.f), ImGuiChildFlags_Borders);

				const std::string _search = Lower(_filter);
				if (g_historyContext)
					for (const std::string& _name : g_historyContext->engine->GetInputManager()->GetEventNames())
					{
						if (!_search.empty() && Lower(_name).find(_search) == std::string::npos) continue;

						const bool _used = std::any_of(_bindings.begin(), _bindings.end(), [&](const InputComponent::Binding& _binding) { return _binding.source == _name; });
						if (ImGui::Selectable(_name.c_str(), false, _used ? ImGuiSelectableFlags_Disabled : 0))
						{
							_bindings.push_back(InputComponent::Binding{ _name, EventList() });
							_changed = true;
							ImGui::CloseCurrentPopup();
						}
					}
				ImGui::EndChild();
				ImGui::EndPopup();
			}
			ImGui::TreePop();
		}

		if (_changed) _text = InputComponent::EncodeBindings(_bindings);
		return _changed;
	}

	// "Player" for an actor, "Player > SpriteComponent" for a component.
	std::string ReferenceLabel(IGameObject* _target)
	{
		if (Component* _component = dynamic_cast<Component*>(_target))
			return (_component->GetOwner() ? _component->GetOwner()->GetName() : std::string("?")) + " > " + ClassNameOf(*_component);
		return _target ? _target->GetName() : std::string("(none)");
	}

	// KPROPERTY(Actor*, ...), KPROPERTY(SpriteComponent*, ...): an object of the level picked in a list, or an actor dropped from the
	// Hierarchy (a component reference takes its component of the right class). The arrow selects the target.
	bool DrawReferencePicker(const PropertyInfo& _property, IGameObject& _owner, std::string& _text)
	{
		Level* _level = g_historyContext ? g_historyContext->GetLevel() : nullptr;
		IGameObject* _current = _text.empty() ? nullptr : Reflection::ResolveReference(&_owner, _text);
		const bool _components = _property.type == PropertyType::ComponentRef;
		bool _changed = false;
		const auto _pick = [&](IGameObject* _target)
		{
			const std::string _new = Reflection::EncodeReference(_target);
			if (_new == _text) return;
			_text = _new;
			_changed = true;
		};
		// What an actor dropped or picked gives: itself, or its first component the property accepts.
		const auto _fromActor = [&](Actor* _actor) -> IGameObject*
		{
			if (!_components) return _property.accepts(_actor) ? _actor : nullptr;
			for (const std::unique_ptr<Component>& _component : _actor->GetComponents())
				if (_property.accepts(_component.get())) return _component.get();
			return nullptr;
		};

		ImGui::PushID(_property.name.c_str());
		const float _arrow = ImGui::GetFrameHeight();
		ImGui::SetNextItemWidth((std::max)(60.f, ImGui::CalcItemWidth() - _arrow - ImGui::GetStyle().ItemInnerSpacing.x));
		const bool _missing = !_text.empty() && !_current;
		const std::string _preview = _missing ? "(missing " + _property.referenceClass + ")" : _current ? ReferenceLabel(_current) : "(none)";
		if (_missing) ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Get(EditorTheme::Custom::ConsoleWarning));
		const bool _open = ImGui::BeginCombo("##reference", _preview.c_str(), ImGuiComboFlags_HeightLarge);
		if (_missing) ImGui::PopStyleColor();
		if (_open)
		{
			static char _filter[64] = "";
			if (ImGui::IsWindowAppearing()) { _filter[0] = 0; ImGui::SetKeyboardFocusHere(); }
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputTextWithHint("##filter", "Search", _filter, sizeof(_filter));
			const std::string _search = Lower(_filter);

			if (ImGui::Selectable("(none)", _current == nullptr)) _pick(nullptr);
			int _shown = 0;
			if (_level)
				for (const std::unique_ptr<Actor>& _actor : _level->GetActorManager().GetActors())
				{
					std::vector<IGameObject*> _candidates;
					if (!_components) _candidates.push_back(_actor.get());
					else for (const std::unique_ptr<Component>& _component : _actor->GetComponents()) _candidates.push_back(_component.get());

					for (IGameObject* _candidate : _candidates)
					{
						if (!_property.accepts(_candidate)) continue;
						const std::string _label = ReferenceLabel(_candidate);
						if (!_search.empty() && Lower(_label).find(_search) == std::string::npos) continue;
						ImGui::PushID(_candidate);
						if (ImGui::Selectable(_label.c_str(), _candidate == _current)) _pick(_candidate);
						if (_candidate == &_owner && ImGui::IsItemHovered()) ImGui::SetTooltip("This object itself");
						ImGui::PopID();
						++_shown;
					}
				}
			if (_shown == 0) ImGui::TextDisabled("No %s in the scene", _property.referenceClass.c_str());
			ImGui::EndCombo();
		}
		if (ImGui::IsItemHovered() && !_open)
			ImGui::SetTooltip("%s\nPick one, or drag an actor from the Hierarchy here", _property.referenceClass.c_str());
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* _payload = ImGui::AcceptDragDropPayload("KRAMPUS_ACTOR"))
			{
				Actor* _actor = *static_cast<Actor* const*>(_payload->Data);
				if (IGameObject* _target = _fromActor(_actor)) _pick(_target);
				else LOG_WARNING("Inspector: " + _actor->GetName() + " has no " + _property.referenceClass + " for " + _property.name);
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::SameLine(0.f, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::BeginDisabled(_current == nullptr || !g_historyContext);
		if (ImGui::ArrowButton("##select", ImGuiDir_Right) && _current)
		{
			Component* _component = dynamic_cast<Component*>(_current);
			g_historyContext->selected = _component ? _component->GetOwner() : dynamic_cast<Actor*>(_current);
			g_historyContext->selectedWidget = nullptr;
		}
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("Select it");
		ImGui::SameLine(0.f, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::TextUnformatted(_property.name.c_str());
		ImGui::PopID();
		return _changed;
	}

}
