#include "EditorContext.h"
#include "EditorThumbnails.h"
#include "Graphics/Shapes/ShapeAssets.h"
#include <imgui.h>

using namespace Krampus;

// Tools > Shape Editor: makes the complex shapes of Content/Shapes (any points, concave included). A shape is used by name by the
// "polygon" property of SpriteComponent, ImageWidgetComponent and PhysicsComponent (shape = Polygon), and every
// component that uses it changes as soon as it is saved.
//
//   Drag a point to move it        Click a "+" (middle of an edge) to add a point there        Double click: a point on the nearest edge
//   Right click a point: delete    Wheel: zoom    Right / middle drag: move the view    Ctrl+S save, Ctrl+Z undo, Delete: the selected point

namespace
{
	bool						g_open			= false;
	std::string					g_current;							// name of the shape being edited (empty: none)
	Polygon2D					g_edit;								// its points, as edited (saved with Save)
	bool						g_dirty			= false;
	std::vector<Polygon2D>		g_undo;
	int							g_selected		= -1;
	int							g_dragging		= -1;
	ImVec2						g_pan			= ImVec2(0.f, 0.f);
	float						g_zoom			= 2.f;				// screen pixels per shape pixel
	bool						g_snap			= true;
	float						g_grid			= 5.f;
	bool						g_showParts		= true;				// the convex pieces collisions and box2d use
	bool						g_showTriangles	= false;			// how it is filled
	std::string					g_pendingOpen;						// a shape to open once the unsaved changes are saved or dropped
	bool						g_askDiscard	= false;
	char						g_newName[64]	= "";
	char						g_renameTo[64]	= "";
	std::string					g_trace;							// texture shown behind the shape (Content/Textures relative)
	float						g_traceOpacity	= 0.45f;
	float						g_traceScale	= 1.f;
	int							g_traceAlpha	= 20;				// alpha above which a pixel is part of the picture
	float						g_traceDetail	= 2.f;				// how far (pixels) the outline may be from the picture

	const char* const			PRESETS[]		= { "Triangle", "Square", "Pentagon", "Hexagon", "Octagon", "Circle (16)", "Star", "Arrow", "L", "Cross", "Capsule" };

	// ---- Shape data

	void PushUndo()
	{
		g_undo.push_back(g_edit);
		if (g_undo.size() > 100) g_undo.erase(g_undo.begin());
		g_dirty = true;
	}

	void Load(const std::string& _name)
	{
		g_current = _name;
		g_edit = ShapeAssets::Get(_name);
		g_dirty = false;
		g_undo.clear();
		g_selected = g_dragging = -1;
	}

	void RequestOpen(const std::string& _name)
	{
		if (_name == g_current) return;
		if (g_dirty)
		{
			g_pendingOpen = _name;
			g_askDiscard = true;
			return;
		}
		Load(_name);
	}

	bool Save()
	{
		if (g_current.empty()) return false;
		if (!ShapeAssets::Save(g_current, g_edit)) return false;
		g_dirty = false;
		return true;
	}

	bool ValidName(const std::string& _name)
	{
		return !_name.empty() && _name.find_first_of("\\:*?\"<>|.") == std::string::npos && _name.back() != '/' && _name.front() != '/';
	}

	Polygon2D Preset(const int _index)
	{
		switch (_index)
		{
		case 0:		return Polygon2D::Regular(3, 60.f);
		case 1:		return Polygon2D::Box(FVector2(100.f, 100.f));
		case 2:		return Polygon2D::Regular(5, 55.f);
		case 3:		return Polygon2D::Regular(6, 50.f);
		case 4:		return Polygon2D::Regular(8, 50.f, -67.5f);
		case 5:		return Polygon2D::Regular(16, 50.f);
		case 6:		return Polygon2D::Star(5, 60.f, 26.f);
		case 7:		return Polygon2D({ FVector2(-60.f, -15.f), FVector2(10.f, -15.f), FVector2(10.f, -40.f), FVector2(60.f, 0.f), FVector2(10.f, 40.f), FVector2(10.f, 15.f), FVector2(-60.f, 15.f) });
		case 8:		return Polygon2D({ FVector2(-50.f, -60.f), FVector2(-10.f, -60.f), FVector2(-10.f, 20.f), FVector2(50.f, 20.f), FVector2(50.f, 60.f), FVector2(-50.f, 60.f) });
		case 9:		return Polygon2D({ FVector2(-15.f, -50.f), FVector2(15.f, -50.f), FVector2(15.f, -15.f), FVector2(50.f, -15.f), FVector2(50.f, 15.f), FVector2(15.f, 15.f),
						FVector2(15.f, 50.f), FVector2(-15.f, 50.f), FVector2(-15.f, 15.f), FVector2(-50.f, 15.f), FVector2(-50.f, -15.f), FVector2(-15.f, -15.f) });
		default:
		{
			// A capsule: two half circles joined by straight sides.
			Polygon2D _capsule;
			for (int _i = 0; _i <= 6; ++_i)
			{
				const float _angle = (-90.f + 30.f * _i) * 3.14159265f / 180.f;
				_capsule.points.emplace_back(30.f + std::cos(_angle) * 30.f, std::sin(_angle) * 30.f);
			}
			for (int _i = 0; _i <= 6; ++_i)
			{
				const float _angle = (90.f + 30.f * _i) * 3.14159265f / 180.f;
				_capsule.points.emplace_back(-30.f + std::cos(_angle) * 30.f, std::sin(_angle) * 30.f);
			}
			return _capsule;
		}
		}
	}

	float Snap(const float _value)
	{
		return g_snap && g_grid > 0.f ? std::round(_value / g_grid) * g_grid : std::round(_value * 100.f) / 100.f;
	}

	// ---- Tracing a picture: the outline of its first opaque part (marching squares), simplified (Douglas-Peucker).

	float DistanceToSegment(const FVector2& _p, const FVector2& _a, const FVector2& _b)
	{
		const float _dx = _b.x - _a.x, _dy = _b.y - _a.y;
		const float _lengthSquared = _dx * _dx + _dy * _dy;
		float _t = _lengthSquared > 0.f ? ((_p.x - _a.x) * _dx + (_p.y - _a.y) * _dy) / _lengthSquared : 0.f;
		_t = std::clamp(_t, 0.f, 1.f);
		const float _x = _a.x + _dx * _t - _p.x, _y = _a.y + _dy * _t - _p.y;
		return std::sqrt(_x * _x + _y * _y);
	}

	void Simplify(const std::vector<FVector2>& _points, const size_t _first, const size_t _last, const float _tolerance, std::vector<bool>& _keep)
	{
		if (_last <= _first + 1) return;
		float _worst = 0.f;
		size_t _index = _first;
		for (size_t _i = _first + 1; _i < _last; ++_i)
		{
			const float _distance = DistanceToSegment(_points[_i], _points[_first], _points[_last]);
			if (_distance > _worst)
			{
				_worst = _distance;
				_index = _i;
			}
		}
		if (_worst <= _tolerance) return;
		_keep[_index] = true;
		Simplify(_points, _first, _index, _tolerance, _keep);
		Simplify(_points, _index, _last, _tolerance, _keep);
	}

	bool TracePicture(const std::filesystem::path& _file, const int _alpha, const float _tolerance, const float _scale, Polygon2D& _out, std::string& _error)
	{
		sf::Image _image;
		if (!_image.loadFromFile(_file))
		{
			_error = "cannot read the picture";
			return false;
		}
		const int _width = static_cast<int>(_image.getSize().x), _height = static_cast<int>(_image.getSize().y);
		std::vector<std::uint8_t> _mask(static_cast<size_t>(_width) * _height);
		for (int _y = 0; _y < _height; ++_y)
			for (int _x = 0; _x < _width; ++_x)
				_mask[static_cast<size_t>(_y) * _width + _x] = _image.getPixel(sf::Vector2u(_x, _y)).a > _alpha ? 1 : 0;
		const auto _solid = [&](const int _x, const int _y) { return _x >= 0 && _y >= 0 && _x < _width && _y < _height && _mask[static_cast<size_t>(_y) * _width + _x]; };

		int _startX = -1, _startY = -1;
		for (int _y = 0; _y < _height && _startX < 0; ++_y)
			for (int _x = 0; _x < _width; ++_x)
				if (_solid(_x, _y)) { _startX = _x; _startY = _y; break; }
		if (_startX < 0)
		{
			_error = "the picture has no opaque pixel (lower the alpha)";
			return false;
		}

		// Walk along the border, on the corners of the pixels.
		std::vector<FVector2> _contour;
		int _x = _startX, _y = _startY, _dx = 0, _dy = 0, _previousDx = 0, _previousDy = 0;
		const size_t _limit = static_cast<size_t>(_width + 1) * (_height + 1) * 4;
		do
		{
			const int _state = (_solid(_x - 1, _y - 1) ? 1 : 0) | (_solid(_x, _y - 1) ? 2 : 0) | (_solid(_x - 1, _y) ? 4 : 0) | (_solid(_x, _y) ? 8 : 0);
			switch (_state)
			{
			case 1: case 5: case 13:	_dx = 0; _dy = -1; break;	// up
			case 2: case 3: case 7:		_dx = 1; _dy = 0; break;	// right
			case 4: case 12: case 14:	_dx = -1; _dy = 0; break;	// left
			case 8: case 10: case 11:	_dx = 0; _dy = 1; break;	// down
			case 6:	if (_previousDy == -1) { _dx = -1; _dy = 0; } else { _dx = 1; _dy = 0; } break;
			case 9:	if (_previousDx == 1) { _dx = 0; _dy = -1; } else { _dx = 0; _dy = 1; } break;
			default: _error = "the outline could not be followed"; return false;
			}
			if (_dx != _previousDx || _dy != _previousDy) _contour.emplace_back(static_cast<float>(_x), static_cast<float>(_y));	// a corner
			_x += _dx;
			_y += _dy;
			_previousDx = _dx;
			_previousDy = _dy;
		} while ((_x != _startX || _y != _startY) && _contour.size() < _limit);

		if (_contour.size() < 3)
		{
			_error = "the opaque part is too small";
			return false;
		}

		// Douglas-Peucker on the closed outline: cut it at its two farthest points, simplify both halves.
		size_t _far = 0;
		float _farDistance = 0.f;
		for (size_t _i = 1; _i < _contour.size(); ++_i)
		{
			const float _distance = std::hypot(_contour[_i].x - _contour[0].x, _contour[_i].y - _contour[0].y);
			if (_distance > _farDistance) { _farDistance = _distance; _far = _i; }
		}
		std::vector<FVector2> _loop = _contour;
		_loop.push_back(_contour[0]);
		std::vector<bool> _keep(_loop.size(), false);
		_keep[0] = _keep[_far] = _keep[_loop.size() - 1] = true;
		Simplify(_loop, 0, _far, _tolerance, _keep);
		Simplify(_loop, _far, _loop.size() - 1, _tolerance, _keep);

		// Around the middle of the picture, like a sprite: its (0, 0) is the center of the texture.
		_out.points.clear();
		for (size_t _i = 0; _i + 1 < _loop.size(); ++_i)
			if (_keep[_i])
				_out.points.emplace_back(std::round((_loop[_i].x - _width * 0.5f) * _scale * 10.f) / 10.f, std::round((_loop[_i].y - _height * 0.5f) * _scale * 10.f) / 10.f);
		if (!_out.IsValid())
		{
			_error = "the outline is too thin";
			return false;
		}
		return true;
	}

	std::vector<std::string> TextureFiles()
	{
		static const std::vector<std::string> EXTENSIONS = { ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".gif" };
		std::vector<std::string> _files;
		const std::filesystem::path _root = CONTENT_DIRECTORY / "Textures";
		std::error_code _error;
		for (const auto& _entry : std::filesystem::recursive_directory_iterator(_root, _error))
		{
			if (!_entry.is_regular_file()) continue;
			std::string _extension = _entry.path().extension().string();
			std::transform(_extension.begin(), _extension.end(), _extension.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
			if (std::find(EXTENSIONS.begin(), EXTENSIONS.end(), _extension) == EXTENSIONS.end()) continue;
			_files.push_back(std::filesystem::relative(_entry.path(), _root, _error).generic_string());
		}
		std::sort(_files.begin(), _files.end());
		return _files;
	}

	// ---- Panels

	void DrawShapeList()
	{
		ImGui::TextUnformatted("Shapes (Content/Shapes)");
		ImGui::SetNextItemWidth(-60.f);
		ImGui::InputTextWithHint("##new", "New shape name", g_newName, sizeof(g_newName));
		ImGui::SameLine();
		const std::string _newName = g_newName;
		ImGui::BeginDisabled(!ValidName(_newName) || ShapeAssets::Exists(_newName));
		if (ImGui::Button("Create", ImVec2(-FLT_MIN, 0.f)))
		{
			if (ShapeAssets::Save(_newName, Polygon2D::Regular(6, 50.f)))
			{
				RequestOpen(_newName);
				g_newName[0] = 0;
			}
		}
		ImGui::EndDisabled();
		ImGui::SetItemTooltip("A new hexagon, to change with the points or a preset (folders: \"Enemies/Bat\")");

		ImGui::BeginChild("ShapeList", ImVec2(0.f, -ImGui::GetFrameHeightWithSpacing() * 2.f), ImGuiChildFlags_Borders);
		for (const std::string& _name : ShapeAssets::List())
		{
			const bool _current = _name == g_current;
			const std::string _label = _name + (_current && g_dirty ? " *" : "");
			if (ImGui::Selectable(_label.c_str(), _current)) RequestOpen(_name);
		}
		ImGui::EndChild();

		ImGui::BeginDisabled(g_current.empty());
		if (ImGui::Button("Duplicate"))
		{
			std::string _copy = g_current + "Copy";
			for (int _i = 2; ShapeAssets::Exists(_copy); ++_i) _copy = g_current + "Copy" + std::to_string(_i);
			if (ShapeAssets::Save(_copy, g_edit)) { g_dirty = false; Load(_copy); }
		}
		ImGui::SameLine();
		if (ImGui::Button("Rename"))
		{
			std::snprintf(g_renameTo, sizeof(g_renameTo), "%s", g_current.c_str());
			ImGui::OpenPopup("RenameShape");
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete")) ImGui::OpenPopup("DeleteShape");
		ImGui::EndDisabled();

		if (ImGui::BeginPopup("RenameShape"))
		{
			ImGui::SetNextItemWidth(200.f);
			ImGui::InputText("##rename", g_renameTo, sizeof(g_renameTo));
			const std::string _to = g_renameTo;
			ImGui::BeginDisabled(!ValidName(_to) || ShapeAssets::Exists(_to));
			if (ImGui::Button("Rename") && ShapeAssets::Rename(g_current, _to))
			{
				g_current = _to;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndDisabled();
			ImGui::TextDisabled("Components that use the old name must be given the new one");
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopup("DeleteShape"))
		{
			ImGui::Text("Delete \"%s\"? Components that use it will show a hexagon.", g_current.c_str());
			if (ImGui::Button("Delete"))
			{
				ShapeAssets::Delete(g_current);
				g_current.clear();
				g_edit = Polygon2D();
				g_dirty = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}

	void DrawToolbar()
	{
		ImGui::BeginDisabled(!g_dirty);
		if (ImGui::Button("Save")) Save();
		ImGui::SameLine();
		if (ImGui::Button("Revert")) Load(g_current);
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(g_undo.empty());
		if (ImGui::Button("Undo"))
		{
			g_edit = g_undo.back();
			g_undo.pop_back();
			g_selected = -1;
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::SetNextItemWidth(130.f);
		if (ImGui::BeginCombo("##preset", "Preset..."))
		{
			for (int _i = 0; _i < static_cast<int>(std::size(PRESETS)); ++_i)
				if (ImGui::Selectable(PRESETS[_i]))
				{
					PushUndo();
					g_edit = Preset(_i);
					g_selected = -1;
				}
			ImGui::EndCombo();
		}
		ImGui::SameLine();
		ImGui::Checkbox("Snap", &g_snap);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(60.f);
		ImGui::DragFloat("##grid", &g_grid, 0.5f, 1.f, 100.f, "%.0f px");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(110.f);
		ImGui::SliderFloat("Zoom", &g_zoom, 0.25f, 12.f, "%.2fx", ImGuiSliderFlags_Logarithmic);
		ImGui::SameLine();
		ImGui::Checkbox("Convex parts", &g_showParts);
		ImGui::SetItemTooltip("The convex pieces collisions and box2d use (a concave shape is cut in several)");
		ImGui::SameLine();
		ImGui::Checkbox("Triangles", &g_showTriangles);

		// Changes of the whole shape.
		const auto _apply = [](const std::function<FVector2(const FVector2&)>& _change)
		{
			PushUndo();
			for (FVector2& _point : g_edit.points) _point = _change(_point);
		};
		if (ImGui::Button("Center"))
		{
			const FVector2 _center = g_edit.Centroid();
			_apply([_center](const FVector2& _p) { return FVector2(Snap(_p.x - _center.x), Snap(_p.y - _center.y)); });
		}
		ImGui::SetItemTooltip("Moves the shape so that its middle is on the pivot (0, 0), where the actor is");
		ImGui::SameLine();
		if (ImGui::Button("Flip H")) { _apply([](const FVector2& _p) { return FVector2(-_p.x, _p.y); }); std::reverse(g_edit.points.begin(), g_edit.points.end()); }
		ImGui::SameLine();
		if (ImGui::Button("Flip V")) { _apply([](const FVector2& _p) { return FVector2(_p.x, -_p.y); }); std::reverse(g_edit.points.begin(), g_edit.points.end()); }
		ImGui::SameLine();
		if (ImGui::Button("Rotate 90")) _apply([](const FVector2& _p) { return FVector2(-_p.y, _p.x); });
		ImGui::SameLine();
		if (ImGui::Button("x0.5")) _apply([](const FVector2& _p) { return FVector2(_p.x * 0.5f, _p.y * 0.5f); });
		ImGui::SameLine();
		if (ImGui::Button("x2")) _apply([](const FVector2& _p) { return FVector2(_p.x * 2.f, _p.y * 2.f); });
		ImGui::SameLine();
		if (ImGui::Button("Snap all")) _apply([](const FVector2& _p) { return FVector2(Snap(_p.x), Snap(_p.y)); });

		// What the shape is, and whether it is usable.
		const bool _valid = g_edit.IsValid();
		const bool _simple = g_edit.IsSimple();
		ImGui::Text("%d points", static_cast<int>(g_edit.points.size()));
		ImGui::SameLine();
		const FRect _bounds = g_edit.Bounds();
		ImGui::TextDisabled("| %.0f x %.0f px | area %.0f |", _bounds.GetSize().x, _bounds.GetSize().y, g_edit.Area());
		ImGui::SameLine();
		if (!_valid) ImGui::TextColored(ImVec4(1.f, 0.4f, 0.3f, 1.f), "needs 3 points that are not on one line");
		else if (!_simple) ImGui::TextColored(ImVec4(1.f, 0.4f, 0.3f, 1.f), "edges cross: move the points so that they do not");
		else if (g_edit.IsConvex()) ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.5f, 1.f), "convex");
		else ImGui::TextColored(ImVec4(0.95f, 0.8f, 0.4f, 1.f), "concave: %d convex parts for collisions and box2d", static_cast<int>(g_edit.ConvexParts(8).size()));
	}

	void DrawTracePanel()
	{
		if (!ImGui::CollapsingHeader("Trace a picture")) return;
		ImGui::TextDisabled("Shows a texture behind the shape, or makes the shape from its opaque outline.");
		ImGui::SetNextItemWidth(200.f);
		if (ImGui::BeginCombo("Texture", g_trace.empty() ? "(none)" : g_trace.c_str()))
		{
			if (ImGui::Selectable("(none)", g_trace.empty())) g_trace.clear();
			for (const std::string& _file : TextureFiles())
				if (ImGui::Selectable(_file.c_str(), _file == g_trace)) g_trace = _file;
			ImGui::EndCombo();
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.f);
		ImGui::SliderFloat("Opacity", &g_traceOpacity, 0.f, 1.f, "%.2f");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.f);
		ImGui::DragFloat("Scale", &g_traceScale, 0.01f, 0.05f, 20.f, "%.2f");
		ImGui::SetItemTooltip("Size of the picture against the shape: the same as the sprite's (its size / the texture size)");

		ImGui::SetNextItemWidth(90.f);
		ImGui::SliderInt("Alpha", &g_traceAlpha, 0, 254);
		ImGui::SetItemTooltip("A pixel more opaque than this is part of the picture");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.f);
		ImGui::SliderFloat("Detail", &g_traceDetail, 0.25f, 20.f, "%.2f px", ImGuiSliderFlags_Logarithmic);
		ImGui::SetItemTooltip("How far the outline may be from the picture: small = more points");
		ImGui::SameLine();
		ImGui::BeginDisabled(g_trace.empty() || g_current.empty());
		if (ImGui::Button("Make shape from picture"))
		{
			Polygon2D _traced;
			std::string _error;
			if (TracePicture(CONTENT_DIRECTORY / "Textures" / g_trace, g_traceAlpha, g_traceDetail, g_traceScale, _traced, _error))
			{
				PushUndo();
				g_edit = _traced;
				g_selected = -1;
				LOG_MSG("Shape Editor: traced " + g_trace + " (" + std::to_string(_traced.points.size()) + " points)");
			}
			else
				LOG_WARNING("Shape Editor: cannot trace " + g_trace + ": " + _error);
		}
		ImGui::EndDisabled();
	}

	void DrawCanvas()
	{
		const ImVec2 _size = ImGui::GetContentRegionAvail();
		if (_size.x < 50.f || _size.y < 50.f) return;
		const ImVec2 _min = ImGui::GetCursorScreenPos();
		const ImVec2 _max(_min.x + _size.x, _min.y + _size.y);
		ImGui::InvisibleButton("ShapeCanvas", _size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
		const bool _hovered = ImGui::IsItemHovered();
		ImGuiIO& _io = ImGui::GetIO();
		ImDrawList* _draw = ImGui::GetWindowDrawList();
		_draw->PushClipRect(_min, _max, true);
		_draw->AddRectFilled(_min, _max, IM_COL32(28, 30, 36, 255));

		const ImVec2 _origin(_min.x + _size.x * 0.5f + g_pan.x, _min.y + _size.y * 0.5f + g_pan.y);
		const auto _toScreen = [&](const FVector2& _p) { return ImVec2(_origin.x + _p.x * g_zoom, _origin.y + _p.y * g_zoom); };
		const auto _toShape = [&](const ImVec2& _s) { return FVector2((_s.x - _origin.x) / g_zoom, (_s.y - _origin.y) / g_zoom); };

		// Grid (every 'grid' pixels, a stronger line every 5), and the axes through the pivot.
		float _step = std::max(1.f, g_grid);
		while (_step * g_zoom < 8.f) _step *= 5.f;
		const FVector2 _topLeft = _toShape(_min), _bottomRight = _toShape(_max);
		for (float _x = std::floor(_topLeft.x / _step) * _step; _x <= _bottomRight.x; _x += _step)
		{
			const bool _major = std::fmod(std::abs(_x), _step * 5.f) < 0.01f;
			_draw->AddLine(ImVec2(_toScreen(FVector2(_x, 0.f)).x, _min.y), ImVec2(_toScreen(FVector2(_x, 0.f)).x, _max.y), _major ? IM_COL32(60, 64, 76, 255) : IM_COL32(40, 43, 52, 255));
		}
		for (float _y = std::floor(_topLeft.y / _step) * _step; _y <= _bottomRight.y; _y += _step)
		{
			const bool _major = std::fmod(std::abs(_y), _step * 5.f) < 0.01f;
			_draw->AddLine(ImVec2(_min.x, _toScreen(FVector2(0.f, _y)).y), ImVec2(_max.x, _toScreen(FVector2(0.f, _y)).y), _major ? IM_COL32(60, 64, 76, 255) : IM_COL32(40, 43, 52, 255));
		}
		_draw->AddLine(ImVec2(_origin.x, _min.y), ImVec2(_origin.x, _max.y), IM_COL32(70, 160, 70, 160), 1.5f);
		_draw->AddLine(ImVec2(_min.x, _origin.y), ImVec2(_max.x, _origin.y), IM_COL32(170, 70, 70, 160), 1.5f);

		// The picture to trace, centered on the pivot like a sprite.
		if (!g_trace.empty())
			if (sf::Texture* _texture = ThumbnailOf(CONTENT_DIRECTORY / "Textures" / g_trace))
			{
				const FVector2 _half(_texture->getSize().x * 0.5f * g_traceScale, _texture->getSize().y * 0.5f * g_traceScale);
				_draw->AddImage(TextureId(*_texture), _toScreen(FVector2(-_half.x, -_half.y)), _toScreen(_half), ImVec2(0.f, 0.f), ImVec2(1.f, 1.f),
					IM_COL32(255, 255, 255, static_cast<int>(g_traceOpacity * 255.f)));
			}

		std::vector<FVector2>& _points = g_edit.points;
		const int _count = static_cast<int>(_points.size());

		// The inside (triangles), the convex parts, the outline.
		if (_count >= 3)
		{
			const ImU32 _fill = g_edit.IsSimple() ? IM_COL32(90, 150, 255, 70) : IM_COL32(255, 90, 70, 70);
			for (const std::array<int, 3>& _triangle : g_edit.Triangulate())
			{
				_draw->AddTriangleFilled(_toScreen(_points[_triangle[0]]), _toScreen(_points[_triangle[1]]), _toScreen(_points[_triangle[2]]), _fill);
				if (g_showTriangles)
					_draw->AddTriangle(_toScreen(_points[_triangle[0]]), _toScreen(_points[_triangle[1]]), _toScreen(_points[_triangle[2]]), IM_COL32(255, 255, 255, 50));
			}
			if (g_showParts && !g_edit.IsConvex())
				for (const std::vector<FVector2>& _part : g_edit.ConvexParts(8))
				{
					std::vector<ImVec2> _screen;
					for (const FVector2& _p : _part) _screen.push_back(_toScreen(_p));
					_draw->AddPolyline(_screen.data(), static_cast<int>(_screen.size()), IM_COL32(255, 200, 80, 150), ImDrawFlags_Closed, 1.f);
				}
		}
		if (_count >= 2)
		{
			std::vector<ImVec2> _screen;
			for (const FVector2& _p : _points) _screen.push_back(_toScreen(_p));
			_draw->AddPolyline(_screen.data(), _count, IM_COL32(120, 190, 255, 255), ImDrawFlags_Closed, 2.f);
		}

		// What is under the mouse: a point, or the "+" in the middle of an edge.
		const ImVec2 _mouse = _io.MousePos;
		int _hoverPoint = -1, _hoverEdge = -1;
		if (_hovered && g_dragging < 0)
		{
			float _best = 8.f;
			for (int _i = 0; _i < _count; ++_i)
			{
				const ImVec2 _s = _toScreen(_points[_i]);
				const float _distance = std::hypot(_s.x - _mouse.x, _s.y - _mouse.y);
				if (_distance < _best) { _best = _distance; _hoverPoint = _i; }
			}
			if (_hoverPoint < 0)
				for (int _i = 0; _i < _count; ++_i)
				{
					const FVector2& _a = _points[_i];
					const FVector2& _b = _points[(_i + 1) % _count];
					const ImVec2 _s = _toScreen(FVector2((_a.x + _b.x) * 0.5f, (_a.y + _b.y) * 0.5f));
					if (std::hypot(_s.x - _mouse.x, _s.y - _mouse.y) < 7.f) { _hoverEdge = _i; break; }
				}
		}

		// Mouse.
		if (_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			if (_hoverPoint >= 0)
			{
				PushUndo();
				g_selected = g_dragging = _hoverPoint;
			}
			else if (_hoverEdge >= 0)
			{
				PushUndo();
				const FVector2& _a = _points[_hoverEdge];
				const FVector2& _b = _points[(_hoverEdge + 1) % _count];
				_points.insert(_points.begin() + _hoverEdge + 1, FVector2(Snap((_a.x + _b.x) * 0.5f), Snap((_a.y + _b.y) * 0.5f)));
				g_selected = g_dragging = _hoverEdge + 1;
			}
			else
				g_selected = -1;
		}
		if (_hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && _hoverPoint < 0 && _hoverEdge < 0 && !g_current.empty())
		{
			// A new point where the mouse is, on the edge closest to it.
			const FVector2 _at = _toShape(_mouse);
			int _edge = _count - 1;
			float _best = std::numeric_limits<float>::max();
			for (int _i = 0; _i < _count; ++_i)
			{
				const float _distance = DistanceToSegment(_at, _points[_i], _points[(_i + 1) % _count]);
				if (_distance < _best) { _best = _distance; _edge = _i; }
			}
			PushUndo();
			_points.insert(_points.begin() + std::max(0, _edge + 1), FVector2(Snap(_at.x), Snap(_at.y)));
			g_selected = std::max(0, _edge + 1);
		}
		if (g_dragging >= 0)
		{
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && g_dragging < static_cast<int>(_points.size()))
			{
				const FVector2 _at = _toShape(_mouse);
				_points[g_dragging] = FVector2(Snap(_at.x), Snap(_at.y));
			}
			else
				g_dragging = -1;
		}
		if (_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && _hoverPoint >= 0 && _count > 3)
		{
			PushUndo();
			_points.erase(_points.begin() + _hoverPoint);
			g_selected = -1;
		}
		if (_hovered && (ImGui::IsMouseDragging(ImGuiMouseButton_Right) || ImGui::IsMouseDragging(ImGuiMouseButton_Middle)))
		{
			g_pan.x += _io.MouseDelta.x;
			g_pan.y += _io.MouseDelta.y;
		}
		if (_hovered && _io.MouseWheel != 0.f)
		{
			// Zoom around the mouse: the point under it stays under it.
			const FVector2 _before = _toShape(_mouse);
			g_zoom = std::clamp(g_zoom * (_io.MouseWheel > 0.f ? 1.15f : 1.f / 1.15f), 0.25f, 12.f);
			const ImVec2 _after = _toScreen(_before);
			g_pan.x += _mouse.x - _after.x;
			g_pan.y += _mouse.y - _after.y;
		}

		// Handles: the "+" of each edge, then the points (numbered).
		for (int _i = 0; _i < static_cast<int>(_points.size()); ++_i)
		{
			const FVector2& _a = _points[_i];
			const FVector2& _b = _points[(_i + 1) % _points.size()];
			const ImVec2 _s = _toScreen(FVector2((_a.x + _b.x) * 0.5f, (_a.y + _b.y) * 0.5f));
			const ImU32 _color = _i == _hoverEdge ? IM_COL32(255, 255, 255, 255) : IM_COL32(150, 160, 180, 140);
			_draw->AddLine(ImVec2(_s.x - 3.f, _s.y), ImVec2(_s.x + 3.f, _s.y), _color);
			_draw->AddLine(ImVec2(_s.x, _s.y - 3.f), ImVec2(_s.x, _s.y + 3.f), _color);
		}
		for (int _i = 0; _i < static_cast<int>(_points.size()); ++_i)
		{
			const ImVec2 _s = _toScreen(_points[_i]);
			const bool _selected = _i == g_selected;
			_draw->AddCircleFilled(_s, _selected ? 6.f : 4.5f, _selected ? IM_COL32(255, 210, 80, 255) : (_i == _hoverPoint ? IM_COL32(255, 255, 255, 255) : IM_COL32(120, 190, 255, 255)));
			_draw->AddText(ImVec2(_s.x + 6.f, _s.y - 15.f), IM_COL32(200, 205, 220, 160), std::to_string(_i).c_str());
		}

		if (_hovered)
		{
			const FVector2 _at = _toShape(_mouse);
			char _text[64];
			std::snprintf(_text, sizeof(_text), "%.1f, %.1f", _at.x, _at.y);
			_draw->AddText(ImVec2(_min.x + 6.f, _max.y - 20.f), IM_COL32(180, 185, 200, 200), _text);
		}
		_draw->AddText(ImVec2(_min.x + 6.f, _min.y + 4.f), IM_COL32(150, 155, 170, 200),
			"Drag a point | click + to add | double click: point on nearest edge | right click a point: delete | wheel: zoom | right drag: move");
		_draw->PopClipRect();
	}

	void DrawPointList()
	{
		ImGui::TextUnformatted("Points");
		ImGui::BeginChild("Points", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders);
		int _remove = -1;
		for (int _i = 0; _i < static_cast<int>(g_edit.points.size()); ++_i)
		{
			ImGui::PushID(_i);
			FVector2& _point = g_edit.points[_i];
			float _values[2] = { _point.x, _point.y };
			if (ImGui::Selectable(std::to_string(_i).c_str(), _i == g_selected, 0, ImVec2(22.f, 0.f))) g_selected = _i;
			ImGui::SameLine();
			ImGui::SetNextItemWidth(-28.f);
			const bool _edited = ImGui::DragFloat2("##p", _values, 0.5f);
			if (ImGui::IsItemActivated()) PushUndo();	// one undo step for the whole drag
			if (_edited)
			{
				_point = FVector2(_values[0], _values[1]);
				g_dirty = true;
			}
			ImGui::SameLine();
			ImGui::BeginDisabled(g_edit.points.size() <= 3);
			if (ImGui::SmallButton("x")) _remove = _i;
			ImGui::EndDisabled();
			ImGui::PopID();
		}
		if (_remove >= 0)
		{
			PushUndo();
			g_edit.points.erase(g_edit.points.begin() + _remove);
			g_selected = -1;
		}
		ImGui::EndChild();
	}
}

namespace Krampus
{
	void SaveShapeEditorChanges()
	{
		if (!g_dirty || g_current.empty()) return;
		if (Save()) LOG_MSG("Shape Editor: unsaved changes of \"" + g_current + "\" saved before the reload");
		else LOG_WARNING("Shape Editor: the changes of \"" + g_current + "\" could not be saved (the shape is not valid)");
	}

	void OpenShapeEditor(const std::string& _shape)
	{
		g_open = true;
		ImGui::SetWindowFocus("###Shape Editor");	// the title shows the shape: the window is known by what follows ###
		if (!_shape.empty()) RequestOpen(_shape);
	}

	void DrawShapeEditor(EditorContext& _context)
	{
		if (!g_open) return;

		ImGui::SetNextWindowSize(ImVec2(1100.f, 700.f), ImGuiCond_FirstUseEver);
		const std::string _title = std::string("Shape Editor") + (g_current.empty() ? "" : " - " + g_current + (g_dirty ? " *" : "")) + "###Shape Editor";
		if (!ImGui::Begin(_title.c_str(), &g_open))
		{
			ImGui::End();
			return;
		}

		// Shortcuts while the window has the focus.
		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::GetIO().WantTextInput)
		{
			const bool _ctrl = ImGui::GetIO().KeyCtrl;
			if (_ctrl && ImGui::IsKeyPressed(ImGuiKey_S) && g_dirty) Save();
			if (_ctrl && ImGui::IsKeyPressed(ImGuiKey_Z) && !g_undo.empty())
			{
				g_edit = g_undo.back();
				g_undo.pop_back();
				g_selected = -1;
			}
			if (ImGui::IsKeyPressed(ImGuiKey_Delete) && g_selected >= 0 && g_selected < static_cast<int>(g_edit.points.size()) && g_edit.points.size() > 3)
			{
				PushUndo();
				g_edit.points.erase(g_edit.points.begin() + g_selected);
				g_selected = -1;
			}
		}

		ImGui::BeginChild("ShapeLeft", ImVec2(230.f, 0.f), ImGuiChildFlags_ResizeX);
		DrawShapeList();
		ImGui::EndChild();
		ImGui::SameLine();

		ImGui::BeginChild("ShapeMain", ImVec2(0.f, 0.f));
		if (g_current.empty())
		{
			ImGui::TextDisabled("Pick a shape on the left, or create one.");
			ImGui::TextDisabled("A shape is used by SpriteComponent, ImageWidgetComponent and PhysicsComponent:");
			ImGui::TextDisabled("set their \"shape\" to Polygon, then choose it in \"polygon\".");
		}
		else
		{
			DrawToolbar();
			DrawTracePanel();
			ImGui::BeginChild("ShapeCanvasArea", ImVec2(-210.f, 0.f));
			DrawCanvas();
			ImGui::EndChild();
			ImGui::SameLine();
			ImGui::BeginChild("ShapePoints", ImVec2(0.f, 0.f));
			DrawPointList();
			ImGui::EndChild();
		}
		ImGui::EndChild();

		// Unsaved changes when another shape is opened.
		if (g_askDiscard)
		{
			ImGui::OpenPopup("Unsaved shape");
			g_askDiscard = false;
		}
		if (ImGui::BeginPopupModal("Unsaved shape", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("\"%s\" has changes that are not saved.", g_current.c_str());
			if (ImGui::Button("Save"))
			{
				Save();
				Load(g_pendingOpen);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Discard"))
			{
				Load(g_pendingOpen);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		(void)_context;
		ImGui::End();
	}
}
