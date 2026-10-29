#include "EditorContext.h"
#include "EditorThumbnails.h"
#include "Actors/Actor.h"
#include "Components/AnimationComponent.h"
#include "Components/AnimationComponentSM.h"
#include "Components/SpriteComponent.h"
#include "UI/Widgets/Widget.h"
#include "UI/Widgets/WidgetComponents/AnimationWidgetComponent.h"
#include "UI/Widgets/WidgetComponents/ImageWidgetComponent.h"
#include <imgui.h>

using namespace Krampus;

// Visual editor of the animation of an AnimationComponent / AnimationComponentSM / AnimationWidgetComponent: the sprite sheet is shown,
// the first frame is drawn on it with the mouse, the frames follow in the chosen direction, and the result is previewed.
// It edits the same AnimationClip as the Inspector, so everything is saved with the scene and is undoable.

namespace
{
	bool	g_open		= false;

	// What the editor works on: the animation component of the selected actor or widget, found again every frame
	// (the selection is cleared whenever the level changes, so nothing here can dangle).
	struct Target
	{
		std::vector<AnimationClip>*	clips	= nullptr;
		String*					start		= nullptr;	// name of the animation the component starts with
		AnimationClip*			clip		= nullptr;	// the one being edited (set once the list was drawn)
		Bool*					playOnBegin	= nullptr;
		std::function<void()>	apply;
		ShapeObject*			shape		= nullptr;	// the picture the animation cuts frames from
		std::string				owner;
	};

	template <typename A, typename S, typename O>
	bool Fill(Target& _target, O& _object)
	{
		A* _animation = _object.template GetComponent<A>();
		if (!_animation) return false;

		_target.clips = &_animation->clips;
		_target.start = &_animation->startAnimation;
		_target.playOnBegin = &_animation->playOnBeginPlay;
		_target.apply = [_animation]() { _animation->ApplyClips(); };
		_target.owner = _object.GetName().StdString();
		if (S* _sprite = _object.template GetComponent<S>())
			_target.shape = _sprite->GetShapeObject();
		return true;
	}

	bool FindTarget(EditorContext& _context, Target& _target)
	{
		if (_context.selected)
			return Fill<AnimationComponent, SpriteComponent>(_target, *_context.selected)
				|| Fill<AnimationComponentSM, SpriteComponent>(_target, *_context.selected);
		if (_context.selectedWidget)
			return Fill<AnimationWidgetComponent, ImageWidgetComponent>(_target, *_context.selectedWidget);
		return false;
	}

	const char* ExtensionOf(const TextureExtensionType& _type)
	{
		static const char* _names[] = { "png", "jpg", "gif", "bmp", "tga", "psd", "hdr", "pic", "pnm" };
		return _names[static_cast<int>(_type)];
	}

	struct Frame { float x, y, w, h; };

	// Frame i of the clip: the first one moved by i frames along the direction (the same rule as AnimationData).
	Frame FrameOf(const AnimationClip& _clip, const int _index)
	{
		Frame _frame = { _clip.frameOrigin.x, _clip.frameOrigin.y, _clip.frameSize.x, _clip.frameSize.y };
		const int _wrap = std::max(0, static_cast<int>(_clip.wrap));
		const int _direction = static_cast<int>(_clip.direction);
		const bool _byColumn = _direction >= 2;
		const bool _reversed = _direction == 1 || _direction == 3;
		const int _along = (_wrap > 0 ? _index % _wrap : _index) * (_reversed ? -1 : 1);	// steps in the direction
		const int _line = _wrap > 0 ? _index / _wrap : 0;									// lines passed (down for a row, right for a column)
		if (_byColumn)
		{
			_frame.y += _along * _frame.h;
			_frame.x += _line * _frame.w;
		}
		else
		{
			_frame.x += _along * _frame.w;
			_frame.y += _line * _frame.h;
		}
		return _frame;
	}

	// Frames of the clip are numbered 0..count-1. Returns the index of the frame under a point of the sheet, or -1.
	int FrameAt(const AnimationClip& _clip, const ImVec2& _point)
	{
		const int _count = static_cast<int>(_clip.frameCount);
		for (int _i = 0; _i < _count; ++_i)
		{
			const Frame _frame = FrameOf(_clip, _i);
			if (_point.x >= _frame.x && _point.x < _frame.x + _frame.w && _point.y >= _frame.y && _point.y < _frame.y + _frame.h)
				return _i;
		}
		return -1;
	}

	// Number of frames (from the first one, along the direction) that fit in a picture of this size.
	int FramesThatFit(const AnimationClip& _clip, const ImVec2& _size)
	{
		const float _w = std::max(1.f, static_cast<float>(_clip.frameSize.x));
		const float _h = std::max(1.f, static_cast<float>(_clip.frameSize.y));
		const int _wrap = std::max(0, static_cast<int>(_clip.wrap));
		if (_wrap > 0)	// whole lines of _wrap frames
		{
			const bool _byColumn = static_cast<int>(_clip.direction) >= 2;
			const int _lines = _byColumn ? static_cast<int>((_size.x - _clip.frameOrigin.x) / _w) : static_cast<int>((_size.y - _clip.frameOrigin.y) / _h);
			return std::max(0, _lines) * _wrap;
		}
		switch (static_cast<int>(_clip.direction))
		{
		case 0: return std::max(0, static_cast<int>((_size.x - _clip.frameOrigin.x) / _w));
		case 1: return std::max(0, static_cast<int>(_clip.frameOrigin.x / _w) + 1);
		case 2: return std::max(0, static_cast<int>((_size.y - _clip.frameOrigin.y) / _h));
		default: return std::max(0, static_cast<int>(_clip.frameOrigin.y / _h) + 1);
		}
	}

	struct View
	{
		std::string		texture;			// picture the view was fitted to
		float			zoom		= 1.f;
		ImVec2			pan			= ImVec2(0.f, 0.f);	// where the picture's top left corner is, relative to the sheet area
		bool			fitted		= false;
	};

	View		g_view;
	int			g_frame			= 0;	// frame shown in the preview (the one selected)
	int			g_clip			= 0;	// animation of the list being edited
	int			g_lastClip		= -1;
	const void*	g_list			= nullptr;
	bool		g_playing		= false;
	float		g_time			= 0.f;
	ImVec2		g_dragStart;
	bool		g_dragging		= false;	// the left button went down on the sheet
	bool		g_defining		= false;	// ... and moved: a new first frame is being drawn
	int			g_gridColumns	= 4;
	int			g_gridRows		= 1;

	void Edit(EditorContext& _context, const Target& _target, const char* _what)
	{
		_context.history.Record(_context, "Edit animation", std::to_string(reinterpret_cast<std::uintptr_t>(_target.clip)) + "." + _what);
	}

	ImU32 Color(const int _r, const int _g, const int _b, const int _a = 255)
	{
		return IM_COL32(_r, _g, _b, _a);
	}

	void FitView(const ImVec2& _area, const ImVec2& _texture)
	{
		g_view.zoom = std::max(0.05f, std::min((_area.x - 20.f) / std::max(1.f, _texture.x), (_area.y - 20.f) / std::max(1.f, _texture.y)));
		g_view.pan = ImVec2((_area.x - _texture.x * g_view.zoom) * 0.5f, (_area.y - _texture.y * g_view.zoom) * 0.5f);
		g_view.fitted = true;
	}

	// The picture with the frames drawn on it. Left drag: first frame. Shift+click: last frame. Click: select a frame. Wheel: zoom. Middle drag: move.
	void DrawSheet(EditorContext& _context, Target& _target, const sf::Texture* _texture, const ImVec2& _size)
	{
		ImGui::BeginChild("Sheet", _size, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		const ImVec2 _min = ImGui::GetCursorScreenPos();
		const ImVec2 _area = ImGui::GetContentRegionAvail();
		const ImVec2 _max = ImVec2(_min.x + _area.x, _min.y + _area.y);
		ImDrawList* _draw = ImGui::GetWindowDrawList();
		_draw->AddRectFilled(_min, _max, Color(28, 28, 30));

		if (!_texture)
		{
			ImGui::TextDisabled("No picture: give a texture to the Sprite (or Image) component first.");
			ImGui::EndChild();
			return;
		}

		const ImVec2 _textureSize = ImVec2(static_cast<float>(_texture->getSize().x), static_cast<float>(_texture->getSize().y));
		if (!g_view.fitted) FitView(_area, _textureSize);

		ImGui::InvisibleButton("SheetArea", _area, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonMiddle | ImGuiButtonFlags_MouseButtonRight);
		const bool _hovered = ImGui::IsItemHovered();
		ImGuiIO& _io = ImGui::GetIO();
		const ImVec2 _origin = ImVec2(_min.x + g_view.pan.x, _min.y + g_view.pan.y);
		auto _toSheet = [&](const ImVec2& _screen) { return ImVec2((_screen.x - _origin.x) / g_view.zoom, (_screen.y - _origin.y) / g_view.zoom); };
		auto _toScreen = [&](const float _x, const float _y) { return ImVec2(_origin.x + _x * g_view.zoom, _origin.y + _y * g_view.zoom); };

		if (_hovered && _io.MouseWheel != 0.f)	// zoom around the cursor
		{
			const ImVec2 _before = _toSheet(_io.MousePos);
			g_view.zoom = std::clamp(g_view.zoom * (_io.MouseWheel > 0.f ? 1.15f : 1.f / 1.15f), 0.05f, 64.f);
			g_view.pan.x = _io.MousePos.x - _min.x - _before.x * g_view.zoom;
			g_view.pan.y = _io.MousePos.y - _min.y - _before.y * g_view.zoom;
		}
		if (ImGui::IsItemActive() && (ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.f) || ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.f)))
		{
			g_view.pan.x += _io.MouseDelta.x;
			g_view.pan.y += _io.MouseDelta.y;
		}

		_draw->PushClipRect(_min, _max, true);

		// The picture, on a checkerboard so transparent parts can be seen.
		const ImVec2 _p0 = _toScreen(0.f, 0.f);
		const ImVec2 _p1 = _toScreen(_textureSize.x, _textureSize.y);
		const float _cell = 16.f;
		for (float _y = _p0.y, _row = 0.f; _y < _p1.y; _y += _cell, _row += 1.f)
			for (float _x = _p0.x, _col = 0.f; _x < _p1.x; _x += _cell, _col += 1.f)
				_draw->AddRectFilled(ImVec2(_x, _y), ImVec2(std::min(_x + _cell, _p1.x), std::min(_y + _cell, _p1.y)), (static_cast<int>(_row + _col) % 2) ? Color(70, 70, 74) : Color(52, 52, 56));
		_draw->AddImage(TextureId(*_texture), _p0, _p1);
		_draw->AddRect(_p0, _p1, Color(120, 120, 130));

		// Frames.
		const int _count = static_cast<int>(_target.clip->frameCount);
		for (int _i = 0; _i < _count; ++_i)
		{
			const Frame _frame = FrameOf(*_target.clip, _i);
			const ImVec2 _a = _toScreen(_frame.x, _frame.y);
			const ImVec2 _b = _toScreen(_frame.x + _frame.w, _frame.y + _frame.h);
			const bool _current = _i == g_frame;
			if (_current) _draw->AddRectFilled(_a, _b, Color(255, 200, 40, 50));
			_draw->AddRect(_a, _b, _i == 0 ? Color(80, 220, 120) : (_current ? Color(255, 200, 40) : Color(255, 140, 40)), 0.f, 0, _current ? 2.f : 1.f);
			if (_b.x - _a.x > 18.f)
			{
				const std::string _label = std::to_string(_i);
				_draw->AddText(ImVec2(_a.x + 3.f, _a.y + 1.f), _i == 0 ? Color(80, 220, 120) : Color(255, 220, 120), _label.c_str());
			}
		}

		// Mouse.
		const ImVec2 _mouse = _toSheet(_io.MousePos);
		auto _clampToPicture = [&](const ImVec2& _point) { return ImVec2(std::clamp(std::round(_point.x), 0.f, _textureSize.x), std::clamp(std::round(_point.y), 0.f, _textureSize.y)); };

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			g_dragging = true;
			g_defining = false;
			g_dragStart = _clampToPicture(_mouse);
		}
		if (g_dragging && ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			const ImVec2 _now = _clampToPicture(_mouse);
			if (!g_defining && (std::abs(_now.x - g_dragStart.x) > 2.f / g_view.zoom || std::abs(_now.y - g_dragStart.y) > 2.f / g_view.zoom))
				g_defining = true;

			if (g_defining)
			{
				const float _w = std::abs(_now.x - g_dragStart.x);
				const float _h = std::abs(_now.y - g_dragStart.y);
				if (_w >= 1.f && _h >= 1.f)
				{
					Edit(_context, _target, "frame");
					_target.clip->frameOrigin = FVector2(std::min(_now.x, g_dragStart.x), std::min(_now.y, g_dragStart.y));
					_target.clip->frameSize = FVector2(_w, _h);
					if (static_cast<int>(_target.clip->frameCount) < 1) _target.clip->frameCount = Int(1);
					_target.apply();
					g_frame = 0;
				}
			}
		}
		if (g_dragging && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			g_dragging = false;
			if (!g_defining)
			{
				const int _hit = FrameAt(*_target.clip, _mouse);
				if (_io.KeyShift)	// last frame = the one clicked, counting from the first
				{
					const Frame _first = FrameOf(*_target.clip, 0);
					// The cell under the mouse, counted from the first frame the way the frames are laid out.
					const int _direction = static_cast<int>(_target.clip->direction);
					const int _wrap = std::max(0, static_cast<int>(_target.clip->wrap));
					const bool _byColumn = _direction >= 2;
					const bool _reversed = _direction == 1 || _direction == 3;
					const int _x = static_cast<int>(std::floor((_mouse.x - _first.x) / _first.w));
					const int _y = static_cast<int>(std::floor((_mouse.y - _first.y) / _first.h));
					const int _xBack = static_cast<int>(std::floor((_first.x + _first.w - _mouse.x) / _first.w));
					const int _yBack = static_cast<int>(std::floor((_first.y + _first.h - _mouse.y) / _first.h));
					const int _along = _byColumn ? (_reversed ? _yBack : _y) : (_reversed ? _xBack : _x);
					const int _line = _byColumn ? _x : _y;
					int _index = -1;
					if (_wrap > 0) { if (_along >= 0 && _along < _wrap && _line >= 0) _index = _line * _wrap + _along; }
					else if (_line == 0 && _along >= 0) _index = _along;
					if (_index >= 0 && _index < 4096)
					{
						Edit(_context, _target, "frameCount");
						_target.clip->frameCount = Int(_index + 1);
						_target.apply();
					}
				}
				else if (_hit >= 0)
					g_frame = _hit;
			}
			g_defining = false;
		}

		// The rectangle being drawn.
		if (g_defining)
			_draw->AddRect(_toScreen(std::min(_mouse.x, g_dragStart.x), std::min(_mouse.y, g_dragStart.y)), _toScreen(std::max(_mouse.x, g_dragStart.x), std::max(_mouse.y, g_dragStart.y)), Color(80, 220, 120), 0.f, 0, 2.f);

		_draw->PopClipRect();

		// Hints and zoom.
		ImGui::SetCursorScreenPos(ImVec2(_min.x + 6.f, _max.y - ImGui::GetTextLineHeightWithSpacing() * 2.f - 4.f));
		ImGui::TextDisabled("Drag: draw the first frame   Shift+click: last frame   Click: select a frame");
		ImGui::SetCursorScreenPos(ImVec2(_min.x + 6.f, _max.y - ImGui::GetTextLineHeightWithSpacing() - 2.f));
		ImGui::TextDisabled("Wheel: zoom   Middle / right drag: move   Zoom %d%%   Pixel %d, %d", static_cast<int>(g_view.zoom * 100.f), static_cast<int>(std::floor(_mouse.x)), static_cast<int>(std::floor(_mouse.y)));
		ImGui::SetCursorScreenPos(ImVec2(_max.x - 52.f, _min.y + 6.f));
		if (ImGui::Button("Fit")) FitView(_area, _textureSize);

		ImGui::EndChild();
	}

	std::string UniqueName(const std::vector<AnimationClip>& _clips, const std::string& _base)
	{
		for (int _n = 1; ; ++_n)
		{
			const std::string _name = _n == 1 ? _base : _base + " " + std::to_string(_n);
			if (std::none_of(_clips.begin(), _clips.end(), [&](const AnimationClip& _clip) { return _clip.name.StdString() == _name; }))
				return _name;
		}
	}

	// One button per animation (the starting one has a star), then Add / Remove / Set as start.
	void DrawAnimationList(EditorContext& _context, Target& _target)
	{
		std::vector<AnimationClip>& _clips = *_target.clips;
		const std::string _startName = _target.start->StdString();
		const auto _isStart = [&](const int _index)
		{
			return _clips[_index].name.StdString() == _startName || (_index == 0 && std::none_of(_clips.begin(), _clips.end(), [&](const AnimationClip& _clip) { return _clip.name.StdString() == _startName; }));
		};
		const std::string _key = std::to_string(reinterpret_cast<std::uintptr_t>(_target.clips)) + ".list";

		ImGui::BeginChild("AnimationList", ImVec2(0.f, ImGui::GetFrameHeight() + ImGui::GetStyle().WindowPadding.y * 2.f + 12.f), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
		for (int _i = 0; _i < static_cast<int>(_clips.size()); ++_i)
		{
			ImGui::PushID(_i);
			const bool _selected = _i == g_clip;
			if (_selected) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
			if (ImGui::Button(((_isStart(_i) ? "* " : "") + _clips[_i].name.StdString()).c_str())) g_clip = _i;
			if (_selected) ImGui::PopStyleColor();
			if (ImGui::IsItemHovered()) ImGui::SetTooltip(_isStart(_i) ? "Starting animation" : "Click to edit");
			ImGui::PopID();
			ImGui::SameLine();
		}

		if (ImGui::Button("+ Add animation"))
		{
			_context.history.Record(_context, "Add animation", _key);
			AnimationClip _new;
			if (!_clips.empty()) _new = _clips[std::clamp(g_clip, 0, static_cast<int>(_clips.size()) - 1)];	// same cut as the one being edited
			_new.name = String(UniqueName(_clips, "Animation"));
			if (_new.frameCount <= 0) _new.frameCount = Int(1);
			_clips.push_back(_new);
			g_clip = static_cast<int>(_clips.size()) - 1;
			if (_clips.size() == 1) *_target.start = _new.name;
			_target.apply();
			ImGui::EndChild();
			return;
		}

		if (!_clips.empty())
		{
			const int _index = std::clamp(g_clip, 0, static_cast<int>(_clips.size()) - 1);
			ImGui::SameLine();
			if (ImGui::Button("Set as start"))
			{
				_context.history.Record(_context, "Set starting animation", _key);
				*_target.start = _clips[_index].name;
				_target.apply();
			}
			ImGui::SameLine();
			if (ImGui::Button("Remove"))
			{
				_context.history.Record(_context, "Remove animation", _key);
				const bool _wasStart = _isStart(_index);
				_clips.erase(_clips.begin() + _index);
				if (_wasStart) *_target.start = String(_clips.empty() ? "" : _clips.front().name.StdString());
				g_clip = std::max(0, _index - 1);
				_target.apply();
			}
		}
		ImGui::EndChild();
	}

	void DrawPreview(const sf::Texture& _texture, const AnimationClip& _clip, const ImVec2& _box)
	{
		const int _count = static_cast<int>(_clip.frameCount);
		const ImVec2 _min = ImGui::GetCursorScreenPos();
		ImGui::Dummy(_box);
		ImDrawList* _draw = ImGui::GetWindowDrawList();
		_draw->AddRectFilled(_min, ImVec2(_min.x + _box.x, _min.y + _box.y), Color(28, 28, 30));
		_draw->AddRect(_min, ImVec2(_min.x + _box.x, _min.y + _box.y), Color(90, 90, 96));
		if (_count <= 0) return;

		const Frame _frame = FrameOf(_clip, std::clamp(g_frame, 0, _count - 1));
		if (_frame.w < 1.f || _frame.h < 1.f) return;

		const float _scale = std::min((_box.x - 8.f) / _frame.w, (_box.y - 8.f) / _frame.h);
		const ImVec2 _half = ImVec2(_frame.w * _scale * 0.5f, _frame.h * _scale * 0.5f);
		const ImVec2 _center = ImVec2(_min.x + _box.x * 0.5f, _min.y + _box.y * 0.5f);
		const float _textureWidth = static_cast<float>(_texture.getSize().x);
		const float _textureHeight = static_cast<float>(_texture.getSize().y);
		_draw->AddImage(TextureId(_texture), ImVec2(_center.x - _half.x, _center.y - _half.y), ImVec2(_center.x + _half.x, _center.y + _half.y),
			ImVec2(_frame.x / _textureWidth, _frame.y / _textureHeight), ImVec2((_frame.x + _frame.w) / _textureWidth, (_frame.y + _frame.h) / _textureHeight));
	}

	// Plays the preview with the real time, whatever the engine does: the level is frozen in the editor.
	void UpdatePreview(const AnimationClip& _clip)
	{
		const int _count = static_cast<int>(_clip.frameCount);
		if (!g_playing || _count <= 0) return;

		const float _perFrame = std::max(0.001f, static_cast<float>(_clip.duration) / static_cast<float>(_count));
		g_time += ImGui::GetIO().DeltaTime;
		int _index = static_cast<int>(g_time / _perFrame);
		if (_index >= _count)
		{
			if (_clip.loop)
			{
				g_time = std::fmod(g_time, _perFrame * static_cast<float>(_count));
				_index = static_cast<int>(g_time / _perFrame) % _count;
			}
			else
			{
				_index = _count - 1;
				g_playing = false;
			}
		}
		g_frame = _index;
	}

	void DrawSettings(EditorContext& _context, Target& _target, const sf::Texture* _texture, const ImVec2& _size)
	{
		ImGui::BeginChild("AnimationSettings", _size, ImGuiChildFlags_Borders);
		AnimationClip& _clip = *_target.clip;
		ImGui::PushItemWidth(-125.f);
		const int _count = static_cast<int>(_clip.frameCount);

		if (_texture)
			DrawPreview(*_texture, _clip, ImVec2(ImGui::GetContentRegionAvail().x, 170.f));

		const float _buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2.f) / 3.f;
		if (ImGui::Button(g_playing ? "Pause" : "Play", ImVec2(_buttonWidth, 0.f)))
		{
			g_playing = !g_playing;
			if (g_playing)
			{
				const float _perFrame = _count > 0 ? static_cast<float>(_clip.duration) / static_cast<float>(_count) : 0.f;
				g_time = _perFrame * static_cast<float>(g_frame);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop", ImVec2(_buttonWidth, 0.f))) { g_playing = false; g_frame = 0; g_time = 0.f; }
		ImGui::SameLine();
		ImGui::TextDisabled("%d / %d", _count > 0 ? std::clamp(g_frame, 0, _count - 1) + 1 : 0, _count);
		ImGui::Separator();

		char _name[128];
		std::snprintf(_name, sizeof(_name), "%s", _clip.name.StdString().c_str());
		if (ImGui::InputText("Name", _name, sizeof(_name)))
		{
			Edit(_context, _target, "name");
			const String _oldName = _clip.name;
			_clip.name = String(_name);
			if (*_target.start == _oldName) *_target.start = _clip.name;
			_target.apply();
		}

		int _origin[2] = { static_cast<int>(_clip.frameOrigin.x), static_cast<int>(_clip.frameOrigin.y) };
		if (ImGui::DragInt2("First frame", _origin, 1.f, 0, 65536))
		{
			Edit(_context, _target, "frameOrigin");
			_clip.frameOrigin = FVector2(static_cast<float>(_origin[0]), static_cast<float>(_origin[1]));
			_target.apply();
		}
		int _frameSize[2] = { static_cast<int>(_clip.frameSize.x), static_cast<int>(_clip.frameSize.y) };
		if (ImGui::DragInt2("Frame size", _frameSize, 1.f, 1, 65536))
		{
			Edit(_context, _target, "frameSize");
			_clip.frameSize = FVector2(static_cast<float>(std::max(1, _frameSize[0])), static_cast<float>(std::max(1, _frameSize[1])));
			_target.apply();
		}
		int _frames = _count;
		if (ImGui::InputInt("Frames", &_frames))
		{
			Edit(_context, _target, "frameCount");
			_clip.frameCount = Int(std::clamp(_frames, 0, 4096));
			_target.apply();
		}
		if (_texture)
		{
			ImGui::SameLine();
			if (ImGui::SmallButton("Max"))
			{
				Edit(_context, _target, "frameCount");
				_clip.frameCount = Int(std::clamp(FramesThatFit(_clip, ImVec2(static_cast<float>(_texture->getSize().x), static_cast<float>(_texture->getSize().y))), 0, 4096));
				_target.apply();
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("As many frames as fit in the picture, from the first one");
		}

		int _direction = static_cast<int>(_clip.direction);
		const char* _directions[] = { "Row (right)", "Row reversed (left)", "Column (down)", "Column reversed (up)" };
		if (ImGui::Combo("Direction", &_direction, _directions, 4))
		{
			Edit(_context, _target, "direction");
			_clip.direction = Int(_direction);
			_target.apply();
		}
		int _perLine = static_cast<int>(_clip.wrap);
		if (ImGui::InputInt("Frames per line", &_perLine))
		{
			Edit(_context, _target, "wrap");
			_clip.wrap = Int(std::clamp(_perLine, 0, 4096));
			_target.apply();
		}
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("0 = all the frames on one line.\nOtherwise the animation goes on the next line (down for a row, right for a column) after this many frames.");


		float _duration = static_cast<float>(_clip.duration);
		if (ImGui::DragFloat("Duration (s)", &_duration, 0.01f, 0.01f, 600.f, "%.2f"))
		{
			Edit(_context, _target, "duration");
			_clip.duration = Float(std::max(0.01f, _duration));
			_target.apply();
		}
		float _fps = _count > 0 ? static_cast<float>(_count) / std::max(0.01f, static_cast<float>(_clip.duration)) : 0.f;
		if (_count > 0 && ImGui::DragFloat("Frames per second", &_fps, 0.1f, 0.1f, 240.f, "%.1f"))
		{
			Edit(_context, _target, "duration");
			_clip.duration = Float(static_cast<float>(_count) / std::max(0.1f, _fps));
			_target.apply();
		}

		bool _loop = static_cast<bool>(_clip.loop);
		if (ImGui::Checkbox("Loop", &_loop))
		{
			Edit(_context, _target, "loop");
			_clip.loop = Bool(_loop);
			_target.apply();
		}
		bool _playOnBegin = static_cast<bool>(*_target.playOnBegin);
		if (ImGui::Checkbox("Play on begin play", &_playOnBegin))
		{
			Edit(_context, _target, "playOnBeginPlay");
			*_target.playOnBegin = Bool(_playOnBegin);
		}

		if (_texture)
		{
			ImGui::Separator();
			ImGui::TextUnformatted("Cut the sheet in a grid");
			ImGui::SetNextItemWidth(70.f);
			ImGui::InputInt("Columns", &g_gridColumns, 0);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(70.f);
			ImGui::InputInt("Rows", &g_gridRows, 0);
			g_gridColumns = std::clamp(g_gridColumns, 1, 512);
			g_gridRows = std::clamp(g_gridRows, 1, 512);
			// Frame size = picture / (columns, rows): the first row only, or every cell of the grid on several lines.
			const auto _cut = [&](const bool _wholeGrid)
			{
				Edit(_context, _target, "grid");
				_clip.frameOrigin = FVector2(0.f, 0.f);
				_clip.frameSize = FVector2(std::floor(static_cast<float>(_texture->getSize().x) / static_cast<float>(g_gridColumns)), std::floor(static_cast<float>(_texture->getSize().y) / static_cast<float>(g_gridRows)));
				_clip.direction = Int(0);
				_clip.wrap = Int(_wholeGrid && g_gridRows > 1 ? g_gridColumns : 0);
				_clip.frameCount = Int(_wholeGrid ? g_gridColumns * g_gridRows : g_gridColumns);
				_target.apply();
				g_frame = 0;
			};
			const float _half = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
			if (ImGui::Button("First row", ImVec2(_half, 0.f))) _cut(false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The animation is the first row of the grid (one line).");
			ImGui::SameLine();
			if (ImGui::Button("Whole grid", ImVec2(-1.f, 0.f))) _cut(true);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Every cell of the grid, row after row (several lines).");
		}

		ImGui::PopItemWidth();
		ImGui::EndChild();
	}

	// One small picture per frame, under the sheet: click to select, the playing frame is outlined.
	void DrawTimeline(const sf::Texture* _texture, const AnimationClip& _clip, const ImVec2& _size)
	{
		ImGui::BeginChild("Timeline", _size, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
		const int _count = static_cast<int>(_clip.frameCount);
		if (!_texture || _count <= 0)
		{
			ImGui::TextDisabled("No frame yet: draw the first frame on the sheet.");
			ImGui::EndChild();
			return;
		}

		const float _height = std::max(24.f, _size.y - 36.f);
		ImDrawList* _draw = ImGui::GetWindowDrawList();
		const float _textureWidth = static_cast<float>(_texture->getSize().x);
		const float _textureHeight = static_cast<float>(_texture->getSize().y);

		for (int _i = 0; _i < _count; ++_i)
		{
			const Frame _frame = FrameOf(_clip, _i);
			const float _width = _frame.h > 0.f ? std::max(16.f, _height * _frame.w / _frame.h) : _height;
			const ImVec2 _min = ImGui::GetCursorScreenPos();

			ImGui::PushID(_i);
			if (ImGui::InvisibleButton("frame", ImVec2(_width, _height)))
			{
				g_frame = _i;
				g_playing = false;
			}
			ImGui::PopID();
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Frame %d  (%d, %d  %dx%d)", _i, static_cast<int>(_frame.x), static_cast<int>(_frame.y), static_cast<int>(_frame.w), static_cast<int>(_frame.h));

			const ImVec2 _max = ImVec2(_min.x + _width, _min.y + _height);
			_draw->AddRectFilled(_min, _max, Color(28, 28, 30));
			_draw->AddImage(TextureId(*_texture), _min, _max, ImVec2(_frame.x / _textureWidth, _frame.y / _textureHeight), ImVec2((_frame.x + _frame.w) / _textureWidth, (_frame.y + _frame.h) / _textureHeight));
			_draw->AddRect(_min, _max, _i == g_frame ? Color(255, 200, 40) : Color(90, 90, 96), 0.f, 0, _i == g_frame ? 2.f : 1.f);
			const std::string _label = std::to_string(_i);
			_draw->AddText(ImVec2(_min.x + 3.f, _min.y + 1.f), Color(255, 255, 255, 220), _label.c_str());
			ImGui::SameLine();
		}
		ImGui::EndChild();
	}
}

namespace Krampus
{
	void OpenAnimationEditor()
	{
		g_open = true;
		ImGui::SetWindowFocus("Animation Editor");
	}

	void DrawAnimationEditor(EditorContext& _context)
	{
		if (!g_open) return;

		ImGui::SetNextWindowSize(ImVec2(1100.f, 640.f), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin("Animation Editor", &g_open))
		{
			ImGui::End();
			return;
		}

		Target _target;
		if (!FindTarget(_context, _target))
		{
			ImGui::TextWrapped("Select an actor (or HUD widget) that has an AnimationComponent, AnimationComponentSM or AnimationWidgetComponent. "
				"The animation is cut from the texture of its Sprite (or Image) component.");
			ImGui::End();
			return;
		}
		if (_context.playing)
		{
			ImGui::TextWrapped("Stop the game to edit the animation.");
			ImGui::End();
			return;
		}

		// The list of animations of the component, and which one is being edited.
		if (g_list != _target.clips)
		{
			g_list = _target.clips;
			g_clip = 0;
			g_frame = 0;
			g_playing = false;
		}
		ImGui::TextDisabled("%s", _target.owner.c_str());
		DrawAnimationList(_context, _target);	// may add or remove a clip: the pointer to the edited one is taken after it
		if (_target.clips->empty())
		{
			ImGui::TextWrapped("No animation yet. Click \"+ Add animation\".");
			ImGui::End();
			return;
		}
		g_clip = std::clamp(g_clip, 0, static_cast<int>(_target.clips->size()) - 1);
		_target.clip = &(*_target.clips)[g_clip];
		if (g_clip != g_lastClip)
		{
			g_lastClip = g_clip;
			g_frame = 0;
			g_playing = false;
		}
		// The picture to cut. Reload the view when it changes.
		const sf::Texture* _texture = nullptr;
		std::string _textureName;
		if (_target.shape && !_target.shape->GetTexturePath().StdString().empty())
		{
			_textureName = _target.shape->GetTexturePath().StdString() + "." + ExtensionOf(_target.shape->GetTextureType());
			_texture = ThumbnailOf(CONTENT_DIRECTORY / "Textures" / _textureName);
		}
		if (_textureName != g_view.texture)
		{
			g_view = View();
			g_view.texture = _textureName;
			g_frame = 0;
			g_playing = false;
		}

		if (_target.shape && !_texture && !_textureName.empty())
			ImGui::TextColored(ImVec4(1.f, 0.6f, 0.3f, 1.f), "Could not load Content/Textures/%s", _textureName.c_str());

		UpdatePreview(*_target.clip);

		const ImVec2 _available = ImGui::GetContentRegionAvail();
		const float _timelineHeight = 96.f;
		const float _settingsWidth = 320.f;
		const float _mainHeight = std::max(120.f, _available.y - _timelineHeight - ImGui::GetStyle().ItemSpacing.y);

		DrawSheet(_context, _target, _texture, ImVec2(std::max(100.f, _available.x - _settingsWidth - ImGui::GetStyle().ItemSpacing.x), _mainHeight));
		ImGui::SameLine();
		DrawSettings(_context, _target, _texture, ImVec2(0.f, _mainHeight));
		DrawTimeline(_texture, *_target.clip, ImVec2(0.f, _timelineHeight));

		ImGui::End();
	}
}
