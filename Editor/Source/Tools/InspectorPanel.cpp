#include "EditorContext.h"
#include "EditorThumbnails.h"
#include "Reflection/Reflection.h"
#include "UI/Widgets/Widget.h"
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
	bool DrawAssetPicker(const char* _label, std::string& _path, const AssetKind& _kind)
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

		const char* _none = _kind == AssetKind::Sound ? "(silent)" : "(none)";
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
				if (_kind == AssetKind::Sound && ImGui::Selectable("(silent)", _path.empty()))
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
		case PropertyType::Flags:
			_changed = DrawFlags(_property, std::get<int>(_value));
			break;
		case PropertyType::Texture:
			_changed = DrawAssetPicker(_property.name.c_str(), std::get<std::string>(_value), AssetKind::Texture);
			break;
		case PropertyType::Font:
			_changed = DrawAssetPicker(_property.name.c_str(), std::get<std::string>(_value), AssetKind::Font);
			break;
		case PropertyType::Sound:
			_changed = DrawAssetPicker(_property.name.c_str(), std::get<std::string>(_value), AssetKind::Sound);
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
			LOG(VerbosityType::Log, "Inspector: " + _object.GetName().StdString() + "." + _property.name + " = " + ValueText(_value));
			_property.set(&_object, _value);
		}
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
			LOG(VerbosityType::Log, "Inspector: " + _owner.GetName().StdString() + (_active ? " activated" : " deactivated"));
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
				{
					LOG_MSG("Inspector: adding " + _class->name + " to " + _actor.GetName().StdString());
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
					LOG_MSG("Inspector: adding " + _class->name + " to widget " + _widget.GetName().StdString());
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
				LOG_MSG("Inspector: removing " + ClassNameOf(*_toRemove) + " from " + _actor.GetName().StdString());
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
				LOG_MSG("Inspector: removing " + ClassNameOf(*_toRemove) + " from widget " + _widget.GetName().StdString());
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
