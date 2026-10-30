#include "EditorContext.h"
#include "EditorThumbnails.h"
#include "Reflection/Reflection.h"
#include "Graphics/Ressources/Animation.h"
#include "UI/Widgets/Widget.h"
#include "Actors/Actor.h"
#include "Components/InputComponent.h"
#include "Managers/InputManager.h"
#include "Graphics/Shapes/ShapeAssets.h"
#include "Components/BlueprintComponent.h"
#include "EditorTheme.h"
#include <imgui.h>

using namespace Krampus;

namespace
{
	enum class AssetKind { Texture, Font, Sound };

	std::string Lower(std::string _text)
	{
		std::transform(_text.begin(), _text.end(), _text.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
		return _text;
	}

	// Where the files of a kind live (relative to Content) and which extensions they can have.
	const char* FolderOf(const AssetKind& _kind)
	{
		switch (_kind)
		{
		case AssetKind::Font:	return "Fonts";
		case AssetKind::Sound:	return "Audio";
		default:				return "Textures";
		}
	}

	bool HasAssetExtension(const AssetKind& _kind, const std::string& _extension)
	{
		static const std::vector<std::string> _textures = { ".png", ".jpg", ".jpeg", ".gif", ".bmp", ".tga", ".psd", ".hdr", ".pic", ".pnm" };
		static const std::vector<std::string> _fonts = { ".otf", ".ttf" };
		static const std::vector<std::string> _sounds = { ".mp3", ".wav" };
		const std::vector<std::string>& _list = _kind == AssetKind::Font ? _fonts : (_kind == AssetKind::Sound ? _sounds : _textures);
		return std::find(_list.begin(), _list.end(), _extension) != _list.end();
	}

	// Every file of a kind, as "Folder/Name.ext" relative to its Content folder. Rebuilt each time a picker opens.
	const std::vector<std::string>& AssetList(const AssetKind& _kind, const bool& _refresh)
	{
		static std::vector<std::string> _lists[3];
		std::vector<std::string>& _list = _lists[static_cast<int>(_kind)];
		if (!_refresh) return _list;

		_list.clear();
		const std::filesystem::path _root = CONTENT_DIRECTORY / FolderOf(_kind);
		std::error_code _error;
		for (const auto& _entry : std::filesystem::recursive_directory_iterator(_root, _error))
		{
			if (!_entry.is_regular_file()) continue;
			if (!HasAssetExtension(_kind, Lower(_entry.path().extension().string()))) continue;
			_list.push_back(std::filesystem::relative(_entry.path(), _root).generic_string());
		}
		std::sort(_list.begin(), _list.end());
		return _list;
	}

	void DrawFitImage(ImDrawList* _draw, const ImVec2& _min, const ImVec2& _max, const sf::Texture& _texture)
	{
		const sf::Vector2f _size(_texture.getSize());
		if (_size.x <= 0.f || _size.y <= 0.f) return;
		const float _scale = std::min((_max.x - _min.x) / _size.x, (_max.y - _min.y) / _size.y);
		const ImVec2 _half(_size.x * _scale * 0.5f, _size.y * _scale * 0.5f);
		const ImVec2 _center((_min.x + _max.x) * 0.5f, (_min.y + _max.y) * 0.5f);
		_draw->AddImage(TextureId(_texture), ImVec2(_center.x - _half.x, _center.y - _half.y), ImVec2(_center.x + _half.x, _center.y + _half.y));
	}

	// A button showing the current file (with its picture for a texture). A click opens a searchable list of the files of
	// Content/Textures, Content/Fonts or Content/Audio: pictures for textures, names for fonts and sounds.
	bool DrawAssetPicker(const char* _label, std::string& _path, const AssetKind& _kind, const char* _emptyChoice = nullptr)
	{
		static char _filter[64] = "";
		bool _changed = false;
		const std::filesystem::path _root = CONTENT_DIRECTORY / FolderOf(_kind);

		ImGui::PushID(_label);
		ImGui::TextUnformatted(_label);
		ImGui::SameLine();

		// Picture of the current texture next to the button.
		const float _frame = ImGui::GetFrameHeight();
		if (_kind == AssetKind::Texture && !_path.empty())
		{
			if (sf::Texture* _texture = ThumbnailOf(_root / _path))
			{
				const ImVec2 _min = ImGui::GetCursorScreenPos();
				ImGui::Dummy(ImVec2(_frame, _frame));
				ImDrawList* _draw = ImGui::GetWindowDrawList();
				_draw->AddRectFilled(_min, ImVec2(_min.x + _frame, _min.y + _frame), IM_COL32(38, 38, 44, 255), 3.f);
				DrawFitImage(_draw, _min, ImVec2(_min.x + _frame, _min.y + _frame), *_texture);
				ImGui::SameLine();
			}
		}

		const char* _none = _emptyChoice ? _emptyChoice : (_kind == AssetKind::Sound ? "(silent)" : "(none)");
		if (ImGui::Button(_path.empty() ? _none : _path.c_str(), ImVec2(-FLT_MIN, 0.f)))
		{
			AssetList(_kind, true);
			_filter[0] = '\0';
			ImGui::OpenPopup("PickAsset");
		}

		if (ImGui::BeginPopup("PickAsset"))
		{
			ImGui::SetNextItemWidth(260.f);
			ImGui::InputTextWithHint("##filter", "Search...", _filter, sizeof(_filter));
			const std::string _lowerFilter = Lower(_filter);

			const bool _grid = _kind == AssetKind::Texture;
			const float _tile = 84.f;
			const float _labelHeight = ImGui::GetTextLineHeight() + 4.f;
			const ImVec2 _listSize = _grid ? ImVec2(4.f * (_tile + 8.f) + 20.f, 320.f) : ImVec2(300.f, 260.f);

			if (ImGui::BeginChild("##list", _listSize, ImGuiChildFlags_Borders))
			{
				if ((_kind == AssetKind::Sound || _emptyChoice) && ImGui::Selectable(_none, _path.empty()))
				{
					_path.clear();
					_changed = true;
					ImGui::CloseCurrentPopup();
				}

				const int _columns = _grid ? std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / (_tile + 8.f))) : 1;
				int _index = 0;
				for (const std::string& _file : AssetList(_kind, false))
				{
					if (!_lowerFilter.empty() && Lower(_file).find(_lowerFilter) == std::string::npos) continue;

					ImGui::PushID(_file.c_str());
					bool _picked = false;

					if (_grid)
					{
						if (_index % _columns != 0) ImGui::SameLine(0.f, 8.f);
						++_index;

						const ImVec2 _min = ImGui::GetCursorScreenPos();
						_picked = ImGui::InvisibleButton("##tile", ImVec2(_tile, _tile + _labelHeight));
						const bool _hovered = ImGui::IsItemHovered();
						const ImVec2 _max(_min.x + _tile, _min.y + _tile + _labelHeight);
						ImDrawList* _draw = ImGui::GetWindowDrawList();
						if (!ImGui::IsItemVisible())	// scrolled out: no drawing, no thumbnail to load (draw lists have a vertex limit)
						{
							ImGui::PopID();
							if (_picked) { _path = _file; _changed = true; ImGui::CloseCurrentPopup(); }
							continue;
						}

						if (_file == _path)		_draw->AddRectFilled(_min, _max, IM_COL32(70, 120, 220, 110), 5.f);
						else if (_hovered)		_draw->AddRectFilled(_min, _max, IM_COL32(255, 255, 255, 30), 5.f);

						const ImVec2 _iconMin(_min.x + 4.f, _min.y + 4.f);
						const ImVec2 _iconMax(_min.x + _tile - 4.f, _min.y + _tile - 4.f);
						_draw->AddRectFilled(_iconMin, _iconMax, IM_COL32(38, 38, 44, 255), 4.f);
						if (sf::Texture* _texture = ThumbnailOf(_root / _file))
							DrawFitImage(_draw, _iconMin, _iconMax, *_texture);

						const std::string _stem = std::filesystem::path(_file).stem().string();
						_draw->PushClipRect(ImVec2(_min.x + 2.f, _min.y + _tile), ImVec2(_max.x - 2.f, _max.y), true);
						_draw->AddText(ImVec2(_min.x + 4.f, _min.y + _tile + 1.f), IM_COL32(230, 230, 235, 255), _stem.c_str());
						_draw->PopClipRect();

						// Bigger picture and the full name while the mouse is over a tile.
						if (_hovered)
						{
							ImGui::BeginTooltip();
							ImGui::TextUnformatted(_file.c_str());
							if (sf::Texture* _texture = ThumbnailOf(_root / _file))
							{
								const sf::Vector2f _size(_texture->getSize());
								const float _scale = std::min(1.f, 200.f / std::max(_size.x, _size.y));
								ImGui::Image(TextureId(*_texture), ImVec2(_size.x * _scale, _size.y * _scale));
								ImGui::TextDisabled("%d x %d", static_cast<int>(_size.x), static_cast<int>(_size.y));
							}
							ImGui::EndTooltip();
						}
					}
					else
					{
						_picked = ImGui::Selectable(_file.c_str(), _file == _path);
					}

					ImGui::PopID();

					if (_picked)
					{
						_path = _file;
						_changed = true;
						ImGui::CloseCurrentPopup();
					}
				}
			}
			ImGui::EndChild();
			ImGui::EndPopup();
		}
		ImGui::PopID();
		return _changed;
	}

	// The scenes of Content/Scenes, as "Name" (or "Folder/Name"), for a property that holds a scene.
	std::vector<std::string> SceneNames()
	{
		std::vector<std::string> _names;
		std::error_code _error;
		for (const auto& _entry : std::filesystem::recursive_directory_iterator(SCENES_DIRECTORY, _error))
		{
			if (!_entry.is_regular_file() || _entry.path().extension() != ".ka7") continue;
			std::filesystem::path _relative = std::filesystem::relative(_entry.path(), SCENES_DIRECTORY, _error);
			_relative.replace_extension();
			_names.push_back(_relative.generic_string());
		}
		std::sort(_names.begin(), _names.end());
		return _names;
	}

	// The outline of a shape of Content/Shapes, fitted in a square of the window.
	void DrawShapePreview(ImDrawList* _draw, const ImVec2& _min, const float _size, const std::string& _shape)
	{
		_draw->AddRectFilled(_min, ImVec2(_min.x + _size, _min.y + _size), IM_COL32(38, 38, 44, 255), 3.f);
		const Polygon2D& _polygon = ShapeAssets::Get(_shape);
		const FRect _bounds = _polygon.Bounds();
		const float _scale = (_size - 6.f) / std::max(1.f, std::max(_bounds.GetSize().x, _bounds.GetSize().y));
		const ImVec2 _center(_min.x + _size * 0.5f, _min.y + _size * 0.5f);
		const FVector2 _middle(_bounds.GetPosition().x + _bounds.GetSize().x * 0.5f, _bounds.GetPosition().y + _bounds.GetSize().y * 0.5f);
		std::vector<ImVec2> _points;
		for (const FVector2& _point : _polygon.points)
			_points.emplace_back(_center.x + (_point.x - _middle.x) * _scale, _center.y + (_point.y - _middle.y) * _scale);
		for (const std::array<int, 3>& _triangle : _polygon.Triangulate())
			_draw->AddTriangleFilled(_points[_triangle[0]], _points[_triangle[1]], _points[_triangle[2]], IM_COL32(90, 150, 255, 110));
		_draw->AddPolyline(_points.data(), static_cast<int>(_points.size()), IM_COL32(140, 200, 255, 255), ImDrawFlags_Closed, 1.f);
	}

	// A drop-down list of the shapes of Content/Shapes with their outline, and a button to edit the chosen one in the Shape Editor.
	bool DrawShapePicker(const char* _label, std::string& _shape)
	{
		bool _changed = false;
		const float _frame = ImGui::GetFrameHeight();
		ImGui::PushID(_label);

		const ImVec2 _previewAt = ImGui::GetCursorScreenPos();
		ImGui::Dummy(ImVec2(_frame, _frame));
		DrawShapePreview(ImGui::GetWindowDrawList(), _previewAt, _frame, _shape);
		ImGui::SameLine();

		ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - _frame - ImGui::GetStyle().ItemSpacing.x - 46.f);
		if (ImGui::BeginCombo("##shape", _shape.empty() ? "(none)" : _shape.c_str(), ImGuiComboFlags_HeightLarge))
		{
			for (const std::string& _name : ShapeAssets::List())
			{
				const ImVec2 _at = ImGui::GetCursorScreenPos();
				if (ImGui::Selectable(("##" + _name).c_str(), _name == _shape, 0, ImVec2(0.f, 28.f)))
				{
					_shape = _name;
					_changed = true;
				}
				DrawShapePreview(ImGui::GetWindowDrawList(), _at, 28.f, _name);
				ImGui::SameLine(40.f);
				ImGui::TextUnformatted(_name.c_str());
			}
			if (ShapeAssets::List().empty()) ImGui::TextDisabled("No shape: Tools > Shape Editor");
			ImGui::EndCombo();
		}
		ImGui::SameLine();
		if (ImGui::Button("Edit", ImVec2(42.f, 0.f))) OpenShapeEditor(_shape);
		ImGui::SetItemTooltip("Opens this shape in the Shape Editor (Tools > Shape Editor)");
		ImGui::SameLine();
		ImGui::TextUnformatted(_label);

		if (!_shape.empty() && !ShapeAssets::Exists(_shape))
			ImGui::TextColored(ImVec4(1.f, 0.45f, 0.3f, 1.f), "No shape \"%s\" in Content/Shapes: a hexagon is used", _shape.c_str());
		ImGui::PopID();
		return _changed;
	}

	// A drop-down list of the scenes. A name that matches no file is shown in red: the game would start on an empty level.
	bool DrawScenePicker(const char* _label, std::string& _scene)
	{
		bool _changed = false;
		const std::vector<std::string> _names = SceneNames();
		const bool _exists = std::find(_names.begin(), _names.end(), _scene) != _names.end();

		if (ImGui::BeginCombo(_label, _scene.empty() ? "(none)" : _scene.c_str()))
		{
			for (const std::string& _name : _names)
			{
				if (ImGui::Selectable(_name.c_str(), _name == _scene))
				{
					_scene = _name;
					_changed = true;
				}
			}
			if (_names.empty()) ImGui::TextDisabled("No scene in Content/Scenes");
			ImGui::EndCombo();
		}
		if (!_exists)
			ImGui::TextColored(ImVec4(1.f, 0.45f, 0.3f, 1.f), "No scene named \"%s\" in Content/Scenes", _scene.c_str());
		return _changed;
	}

	// One check box per name. Returns true when a bit changed.
	bool DrawFlags(const PropertyInfo& _property, int& _bits)
	{
		bool _changed = false;
		const int _total = static_cast<int>(_property.flagNames.size());
		int _count = 0;
		for (int _i = 0; _i < _total; ++_i)
			if (_bits & (1 << _i)) ++_count;

		if (ImGui::TreeNode(_property.name.c_str(), "%s  (%d / %d)", _property.name.c_str(), _count, _total))
		{
			if (ImGui::SmallButton("None")) { _bits = 0; _changed = true; }
			ImGui::SameLine();
			if (ImGui::SmallButton("All")) { _bits = -1; _changed = true; }

			if (ImGui::BeginTable("##flags", 2))
			{
				for (int _i = 0; _i < _total; ++_i)
				{
					ImGui::TableNextColumn();
					bool _on = (_bits & (1 << _i)) != 0;
					if (ImGui::Checkbox(_property.flagNames[_i].c_str(), &_on))
					{
						_bits = _on ? (_bits | (1 << _i)) : (_bits & ~(1 << _i));
						_changed = true;
					}
				}
				ImGui::EndTable();
			}
			ImGui::TreePop();
		}
		return _changed;
	}

	bool g_propertyEdited = false;	// set by DrawProperty, read by DrawObjectProperties
	EditorContext* g_historyContext = nullptr;	// set while the Inspector draws: its edits can be undone (the Settings page edits no scene object)

	void RecordEdit(const std::string& _label, const std::string& _key = "")
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
		return (_target.starts_with("A:") ? "Actor: " : "Widget: ") + _target.substr(2);
	}

	// The argument of a function, edited with the widget of its type. Returns true when it changed.
	bool DrawArgument(const FunctionInfo& _function, PropertyValue& _value)
	{
		const char* _label = "Argument";
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
					if (_level)
					{
						for (const std::unique_ptr<Actor>& _actor : _level->GetActorManager().GetActors())
							_pick("A:" + _actor->GetName().StdString());
						for (const std::unique_ptr<Widget>& _widget : _level->GetHUD().GetWidgets())
							_pick("W:" + _widget->GetName().StdString());
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
						ImGui::EndCombo();
					}
				}

				// Function
				IGameObject* _object = EventList::Resolve(&_owner, _call);
				if (!_object)
					ImGui::TextColored(ImVec4(1.f, 0.45f, 0.3f, 1.f), "Target not found in this scene");
				else
				{
					const ClassInfo* _class = ClassRegistry::Find(_call.target == "Game" ? std::string("Game") : ClassNameOf(*_object));
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
	bool DrawInputBindings(const PropertyInfo& _property, IGameObject& _owner, std::string& _text, const bool _messages = false)
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
		case PropertyType::MessageBindings:
			_changed = DrawInputBindings(_property, _object, std::get<std::string>(_value), true);
			break;
		case PropertyType::InputBindings:
			_changed = DrawInputBindings(_property, _object, std::get<std::string>(_value));
			break;
		case PropertyType::Events:
			_changed = DrawEventList(_property, _object, std::get<std::string>(_value));
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
			LOG(VerbosityType::Verbose, "Inspector: " + _object.GetName().StdString() + "." + _property.name + " = " + ValueText(_value));
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

	void DrawTransform(ITransformable& _actor)
	{
		FVector2 _position = _actor.GetPosition();
		float _positionValues[2] = { _position.x, _position.y };
		if (ImGui::DragFloat2("Position", _positionValues, 0.5f))
		{
			RecordEdit("Move", KeyOf(&_actor, "position"));
			_actor.SetPosition(FVector2(_positionValues[0], _positionValues[1]));
		}

		float _rotation = _actor.GetRotation().ToDegrees();
		if (ImGui::DragFloat("Rotation", &_rotation, 0.5f))
		{
			RecordEdit("Rotate", KeyOf(&_actor, "rotation"));
			_actor.SetRotation(Angle::FromDegrees(_rotation));
		}

		FVector2 _scale = _actor.GetScale();
		float _scaleValues[2] = { _scale.x, _scale.y };
		if (ImGui::DragFloat2("Scale", _scaleValues, 0.01f))
		{
			RecordEdit("Scale", KeyOf(&_actor, "scale"));
			_actor.SetScale(FVector2(_scaleValues[0], _scaleValues[1]));
		}
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
			{
				RecordEdit("Rename", KeyOf(&_owner, "name"));
				_owner.SetName(String(_name));
			}

		bool _active = _owner.IsActive();
		if (ImGui::Checkbox("Active", &_active))
		{
			LOG(VerbosityType::Verbose, "Inspector: " + _owner.GetName().StdString() + (_active ? " activated" : " deactivated"));
			{
				RecordEdit(_active ? "Activate" : "Deactivate");
				_owner.SetActive(Bool(_active));
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

	// BlueprintComponent: which blueprint it runs (a list of Content/Blueprints), a new one, and the editor.
	void DrawBlueprintPicker(EditorContext& _context, BlueprintComponent& _component)
	{
		const std::string _shown = _component.blueprint.empty() ? std::string("(none)") : _component.blueprint;
		ImGui::SetNextItemWidth(-1.f);
		if (ImGui::BeginCombo("##blueprint", _shown.c_str()))
		{
			if (ImGui::Selectable("(none)", _component.blueprint.empty()))
			{
				_context.history.Record(_context, "Set blueprint");
				_component.blueprint.clear();
			}
			for (const std::string& _name : BlueprintAsset::List())
				if (ImGui::Selectable(_name.c_str(), _name == _component.blueprint))
				{
					_context.history.Record(_context, "Set blueprint");
					_component.blueprint = _name;
				}
			ImGui::EndCombo();
		}
		if (!_component.blueprint.empty() && !BlueprintAsset::Exists(_component.blueprint))
			ImGui::TextColored(EditorTheme::Get(EditorTheme::Custom::ConsoleWarning), "No file %s", BlueprintAsset::PathOf(_component.blueprint).generic_string().c_str());

		const float _half = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
		ImGui::BeginDisabled(_component.blueprint.empty());
		if (ImGui::Button("Open in Blueprint Editor", ImVec2(_half, 0.f))) OpenBlueprintEditor(_component.blueprint);
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("+ New Blueprint", ImVec2(-1.f, 0.f)))
		{
			const std::string _created = CreateBlueprintAsset(_context.selected ? _context.selected->GetName().StdString() : "NewBlueprint");
			if (!_created.empty())
			{
				_context.history.Record(_context, "Set blueprint");
				_component.blueprint = _created;
				OpenBlueprintEditor(_created);
			}
		}
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
		const std::string _shown = _clips.empty() ? "(no animation)" : (_current.empty() ? _clips.front().name.StdString() + " (first)" : _current);

		if (ImGui::BeginCombo("Start animation", _shown.c_str()))
		{
			for (const AnimationClip& _clip : _clips)
			{
				const std::string _name = _clip.name.StdString();
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
				if (ClassNameOf(*_component).starts_with("Animation")) DrawStartAnimation(*_component);
				if (BlueprintComponent* _blueprint = dynamic_cast<BlueprintComponent*>(_component.get()); _blueprint && g_historyContext) DrawBlueprintPicker(*g_historyContext, *_blueprint);
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
					LOG(VerbosityType::Log, "Inspector: adding " + _class->name + " to " + _actor.GetName().StdString());
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
					LOG(VerbosityType::Log, "Inspector: adding " + _class->name + " to widget " + _widget.GetName().StdString());
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
				LOG(VerbosityType::Log, "Inspector: removing " + ClassNameOf(*_toRemove) + " from " + _actor.GetName().StdString());
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
				LOG(VerbosityType::Log, "Inspector: removing " + ClassNameOf(*_toRemove) + " from widget " + _widget.GetName().StdString());
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

bool Krampus::DrawObjectProperties(IGameObject& _object)
{
	g_propertyEdited = false;
	DrawProperties(_object);
	return g_propertyEdited;
}
