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
	bool DrawAssetPicker(const char* _label, std::string& _path, const AssetKind& _kind, const char* _emptyChoice)
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
}
