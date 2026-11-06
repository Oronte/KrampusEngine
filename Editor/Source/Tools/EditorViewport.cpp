#include "EditorLayer.h"
#include "Layers/ScreenSpace.h"
#include "EditorThumbnails.h"
#include "Shortcuts.h"
#include "EditorTheme.h"
#include "FileDrop.h"
#include "GameplayTagWidgets.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>
#include "GameFramework/Level.h"
#include "Components/CameraComponent.h"
#include "Components/PhysicsComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/BillboardComponent.h"
#include "Navigation/NavigationSystem.h"
#include "Graphics/Shaders/ShaderLibrary.h"
#include "Managers/TextureManager.h"
#include "Utilities/Debug/Debug.h"
#include "Utilities/Debug/Profiler.h"
#include <unordered_set>
#include "Utilities/Math/Polygon2D.h"
#include "Actors/Actor.h"
#include "UI/Widgets/Widget.h"
#include "Graphics/Shapes/ShapeAssets.h"

using namespace Krampus;

void EditorLayer::DrawViewport()
{
	// Set again below when the picture is visible; hidden tab = no picture to point at.
	window.ResetMouseViewport();
	viewportHovered = false;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
	if (ImGui::Begin("Viewport"))
	{
		const ImVec2 _avail = ImGui::GetContentRegionAvail();
		const sf::Vector2f _texSize(viewportTexture.getSize());

		if (_avail.x > 1.f && _avail.y > 1.f)
		{
			const float _scale = std::min(_avail.x / _texSize.x, _avail.y / _texSize.y);
			const sf::Vector2f _drawSize(_texSize.x * _scale, _texSize.y * _scale);
			Debug::displayScale = _scale > 0.f ? 1.f / _scale : 1.f;	// debug lines stay as thick on the screen (used from the next frame)
			viewportTexture.setSmooth(_scale < 1.f);	// made smaller: blended pixels instead of whole rows skipped

			// Center the picture in the panel.
			ImGui::SetCursorPos(ImVec2(
				ImGui::GetCursorPosX() + (_avail.x - _drawSize.x) * 0.5f,
				ImGui::GetCursorPosY() + (_avail.y - _drawSize.y) * 0.5f));
			// The copy of the window comes out upside down: swap the V coordinates.
			ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<std::intptr_t>(viewportTexture.getNativeHandle())),
				ImVec2(_drawSize.x, _drawSize.y), ImVec2(0.f, 1.f), ImVec2(1.f, 0.f));

			// Where the picture sits in the window, so the game sees the mouse as if it filled the window.
			const ImVec2 _min = ImGui::GetItemRectMin();
			const ImVec2 _max = ImGui::GetItemRectMax();
			window.SetMouseViewport(sf::FloatRect({ _min.x, _min.y }, { _max.x - _min.x, _max.y - _min.y }));
			viewportHovered = ImGui::IsItemHovered();
			MoveCamera(_min, _max);
			DrawSelectionOutlines(_min, _max);
			DrawBillboards(_min, _max);	// under the gizmo
			DrawGizmo(_min, _max);
			PickInViewport(_min, _max);	// after the gizmo: a click on one of its handles is not a pick
			BoxSelectInViewport(_min, _max);
			DrawViewportOverlay(_min, _max);
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

// A left click on the picture (not on the gizmo) selects the actor under the mouse: inside the box2d shape of its PhysicsComponent
// or the outline of its SpriteComponent. The one in front wins; clicking again at the same place goes to the next one behind.
// A click on nothing clears the selection.
void EditorLayer::PickInViewport(const ImVec2& _min, const ImVec2& _max)
{
	if (context.playing || cameraDragging || gizmoAxis >= 0) return;
	if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left) || !ImGui::IsWindowHovered() || ImGui::IsAnyItemHovered()) return;
	const ImVec2 _mouse = ImGui::GetIO().MousePos;
	if (_mouse.x < _min.x || _mouse.y < _min.y || _mouse.x > _max.x || _mouse.y > _max.y) return;
	Level* _level = context.GetLevel();
	if (!_level) return;

	// The picture fills the window size once scaled: back to a window pixel, then to the world through the editor camera.
	const sf::Vector2f _windowSize(window.GetRenderWindow().getSize());
	const sf::Vector2i _pixel(static_cast<int>((_mouse.x - _min.x) * _windowSize.x / (_max.x - _min.x)), static_cast<int>((_mouse.y - _min.y) * _windowSize.y / (_max.y - _min.y)));
	const sf::Vector2f _world = window.GetRenderWindow().mapPixelToCoords(_pixel, context.GetCameraView(_windowSize));
	const FVector2 _point(_world.x, _world.y);

	struct Hit { Actor* actor; int zOrder; };
	std::vector<Hit> _hits;
	for (const std::unique_ptr<Actor>& _actor : _level->GetActorManager().GetActors())
	{
		if (!_actor->IsActive()) continue;
		bool _hit = false;
		int _zOrder = -1;
		if (PhysicsComponent* _physics = _actor->GetComponent<PhysicsComponent>())
			_hit = _physics->ContainsPoint(_point);
		if (SpriteComponent* _sprite = _actor->GetComponent<SpriteComponent>())
		{
			_zOrder = _sprite->GetZOrder();
			if (!_hit)
				_hit = Polygon2D(Polygon2D::Transform(_sprite->GetShapeObject()->GetOutline().points, _sprite->GetPosition(), _sprite->GetRotation(), _sprite->GetScale())).Contains(_point);
		}
		if (_hit) _hits.push_back({ _actor.get(), _zOrder });
	}

	// In front first: the highest draw order, then the actor made last (drawn last).
	std::reverse(_hits.begin(), _hits.end());
	std::stable_sort(_hits.begin(), _hits.end(), [](const Hit& _a, const Hit& _b) { return _a.zOrder > _b.zOrder; });

	// Billboard icons before shapes: they are drawn over the scene, and are the way to reach actors nothing shows.
	std::vector<Actor*> _order;
	for (auto _icon = billboardRects.rbegin(); _icon != billboardRects.rend(); ++_icon)
		if (_mouse.x >= _icon->second.x && _mouse.y >= _icon->second.y && _mouse.x <= _icon->second.z && _mouse.y <= _icon->second.w)
			_order.push_back(_icon->first);
	for (const Hit& _hit : _hits)
		if (std::find(_order.begin(), _order.end(), _hit.actor) == _order.end()) _order.push_back(_hit.actor);

	const bool _ctrl = ImGui::GetIO().KeyCtrl;
	if (_order.empty())
	{
		// On nothing: a rectangle may start here (BoxSelectInViewport). Ctrl keeps the selection to add to it.
		if (!_ctrl)
		{
			context.Select(nullptr);
			context.selectedWidget = nullptr;
		}
		boxSelecting = true;
		boxStart = _mouse;
		return;
	}

	// Ctrl: the actor in front goes in or out of the selection.
	if (_ctrl)
	{
		context.ToggleSelected(_order.front());
		LOG(VerbosityType::Verbose, "Viewport: " + std::to_string(context.selection.size()) + " actor(s) selected");
		return;
	}

	size_t _next = 0;
	for (size_t _i = 0; _i < _order.size(); ++_i)
		if (_order[_i] == context.selected) { _next = (_i + 1) % _order.size(); break; }

	context.Select(_order[_next]);
	LOG(VerbosityType::Verbose, "Viewport: selected actor " + context.selected->GetName());
}

// A drag that started on nothing draws a rectangle; on release the actors whose position is inside are selected (Ctrl: added).
void EditorLayer::BoxSelectInViewport(const ImVec2& _min, const ImVec2& _max)
{
	if (!boxSelecting) return;
	Level* _level = context.GetLevel();
	if (context.playing || !_level || gizmoAxis >= 0) { boxSelecting = false; return; }

	const ImVec2 _mouse = ImGui::GetIO().MousePos;
	const ImVec2 _a(std::min(boxStart.x, _mouse.x), std::min(boxStart.y, _mouse.y)), _b(std::max(boxStart.x, _mouse.x), std::max(boxStart.y, _mouse.y));
	const bool _dragged = _b.x - _a.x > 4.f || _b.y - _a.y > 4.f;
	if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		if (!_dragged) return;
		ImDrawList* _draw = ImGui::GetWindowDrawList();
		_draw->PushClipRect(_min, _max, true);
		const ImU32 _accent = EditorTheme::GetU32(EditorTheme::Custom::Selection);
		_draw->AddRectFilled(_a, _b, (_accent & 0x00FFFFFF) | 0x30000000);
		_draw->AddRect(_a, _b, _accent);
		_draw->PopClipRect();
		return;
	}

	boxSelecting = false;
	if (!_dragged) return;
	const sf::Vector2f _windowSize(window.GetRenderWindow().getSize());
	const sf::View _view = context.GetCameraView(_windowSize);
	if (!ImGui::GetIO().KeyCtrl) context.Select(nullptr);
	for (const std::unique_ptr<Actor>& _actor : _level->GetActorManager().GetActors())
	{
		const FVector2 _position = _actor->GetPosition();
		const sf::Vector2i _pixel = window.GetRenderWindow().mapCoordsToPixel(sf::Vector2f(_position.x, _position.y), _view);
		const ImVec2 _screen(_min.x + _pixel.x * (_max.x - _min.x) / _windowSize.x, _min.y + _pixel.y * (_max.y - _min.y) / _windowSize.y);
		if (_screen.x >= _a.x && _screen.x <= _b.x && _screen.y >= _a.y && _screen.y <= _b.y)
			context.AddToSelection(_actor.get());
	}
	LOG(VerbosityType::Verbose, "Viewport: " + std::to_string(context.selection.size()) + " actor(s) selected by rectangle");
}

// Every selected actor: the outline of its sprite (or a small square where it is), the one shown in the Inspector thicker.
void EditorLayer::DrawSelectionOutlines(const ImVec2& _min, const ImVec2& _max)
{
	if (context.playing || context.selection.empty()) return;
	const sf::Vector2f _windowSize(window.GetRenderWindow().getSize());
	const sf::View _view = context.GetCameraView(_windowSize);
	const auto _toScreen = [&](const FVector2& _world)
	{
		const sf::Vector2i _pixel = window.GetRenderWindow().mapCoordsToPixel(sf::Vector2f(_world.x, _world.y), _view);
		return ImVec2(_min.x + _pixel.x * (_max.x - _min.x) / _windowSize.x, _min.y + _pixel.y * (_max.y - _min.y) / _windowSize.y);
	};

	ImDrawList* _draw = ImGui::GetWindowDrawList();
	_draw->PushClipRect(_min, _max, true);
	const ImU32 _accent = EditorTheme::GetU32(EditorTheme::Custom::Selection);
	for (Actor* _actor : context.selection)
	{
		const float _thickness = _actor == context.selected ? 2.5f : 1.5f;
		if (SpriteComponent* _sprite = _actor->GetComponent<SpriteComponent>())
		{
			std::vector<ImVec2> _points;
			for (const FVector2& _point : Polygon2D::Transform(_sprite->GetShapeObject()->GetOutline().points, _sprite->GetPosition(), _sprite->GetRotation(), _sprite->GetScale()))
				_points.push_back(_toScreen(_point));
			if (_points.size() >= 2) _draw->AddPolyline(_points.data(), static_cast<int>(_points.size()), _accent, ImDrawFlags_Closed, _thickness);
		}
		else
		{
			const ImVec2 _center = _toScreen(_actor->GetPosition());
			_draw->AddRect(ImVec2(_center.x - 6.f, _center.y - 6.f), ImVec2(_center.x + 6.f, _center.y + 6.f), _accent, 0.f, 0, _thickness);
		}
	}
	_draw->PopClipRect();
}

// The icons of the BillboardComponents, where their actor is, at the same size on the screen whatever the zoom. Only while
// editing: the game never draws them. Their rectangles are kept for PickInViewport (an icon is clicked before any shape).
void EditorLayer::DrawBillboards(const ImVec2& _min, const ImVec2& _max)
{
	billboardRects.clear();
	if (context.playing || !editorSettings.showBillboards) return;
	Level* _level = context.GetLevel();
	if (!_level) return;

	const sf::Vector2f _windowSize(window.GetRenderWindow().getSize());
	const ImVec2 _rectSize(_max.x - _min.x, _max.y - _min.y);
	if (_windowSize.x < 1.f || _windowSize.y < 1.f || _rectSize.x < 1.f || _rectSize.y < 1.f) return;
	const sf::View _view = context.GetCameraView(_windowSize);

	// The drawn symbols, from Segoe UI Symbol (a letter when the font has none).
	static const char* GLYPHS[8][2] = {
		{ "\xE2\x97\x8F", "A" },	// actor: a dot
		{ "\xE2\x97\x89", "C" },	// camera
		{ "\xE2\x99\xAA", "S" },	// sound: a note
		{ "\xE2\x98\x80", "L" },	// light: a sun
		{ "\xE2\x98\x84", "E" },	// effect: a comet
		{ "\xE2\x9A\x91", "P" },	// spawn: a flag
		{ "\xE2\x9A\x99", "G" },	// logic: a gear
		{ "\xE2\x98\x85", "*" } };	// star
	static std::unordered_set<std::string> missingTextures;	// asked once: a missing file is not looked for every frame

	ImDrawList* _draw = ImGui::GetWindowDrawList();
	_draw->PushClipRect(_min, _max, true);
	const ImVec2 _mouse = ImGui::GetIO().MousePos;
	const bool _windowHovered = ImGui::IsWindowHovered();
	const ImU32 _accent = EditorTheme::GetU32(EditorTheme::Custom::Selection);

	for (const std::unique_ptr<Actor>& _actor : _level->GetActorManager().GetActors())
	{
		BillboardComponent* _billboard = _actor->GetComponent<BillboardComponent>();
		if (!_billboard || !_billboard->IsActive()) continue;

		const FVector2 _position = _actor->GetPosition();
		const sf::Vector2i _pixel = window.GetRenderWindow().mapCoordsToPixel(sf::Vector2f(_position.x, _position.y), _view);
		const ImVec2 _center(_min.x + _pixel.x * _rectSize.x / _windowSize.x, _min.y + _pixel.y * _rectSize.y / _windowSize.y);
		const float _half = std::clamp(_billboard->size, 8.f, 256.f) * 0.5f * editorSettings.uiScale;
		if (_center.x + _half < _min.x || _center.x - _half > _max.x || _center.y + _half < _min.y || _center.y - _half > _max.y) continue;

		const ImVec2 _a(_center.x - _half, _center.y - _half), _b(_center.x + _half, _center.y + _half);
		billboardRects.push_back({ _actor.get(), ImVec4(_a.x, _a.y, _b.x, _b.y) });

		const bool _selected = context.IsSelected(_actor.get());
		const bool _hovered = _windowHovered && _mouse.x >= _a.x && _mouse.y >= _a.y && _mouse.x <= _b.x && _mouse.y <= _b.y;
		const float _fade = _actor->IsActive() ? 1.f : 0.4f;	// an inactive actor: a faded icon
		const Color& _tint = _billboard->tint;
		const ImU32 _color = IM_COL32(_tint.r, _tint.g, _tint.b, static_cast<int>(_tint.a * _fade));

		// A picture of Content/Textures, or a drawn icon.
		std::shared_ptr<sf::Texture> _texture;
		if (!_billboard->texture.empty() && !missingTextures.contains(_billboard->texture))
		{
			_texture = M_TEXTURE.GetSharedTexture(String(_billboard->texture));
			if (!_texture) missingTextures.insert(_billboard->texture);
		}

		if (_texture)
		{
			_draw->AddImage(reinterpret_cast<ImTextureID>(static_cast<std::intptr_t>(_texture->getNativeHandle())), _a, _b, ImVec2(0.f, 0.f), ImVec2(1.f, 1.f), _color);
			if (_selected || _hovered) _draw->AddRect(_a, _b, _selected ? _accent : IM_COL32(255, 255, 255, 140), 4.f, 0, _selected ? 2.f : 1.f);
		}
		else
		{
			_draw->AddCircleFilled(_center, _half, IM_COL32(18, 19, 22, static_cast<int>((_hovered ? 230 : 190) * _fade)), 24);
			_draw->AddCircle(_center, _half - 1.f, _selected ? _accent : _color, 24, _selected ? 2.5f : 1.5f);
			const int _icon = std::clamp(_billboard->icon, 0, 7);
			const char* _glyph = EditorTheme::Symbol(GLYPHS[_icon][0], GLYPHS[_icon][1]);
			const float _fontSize = _half * 1.15f;
			const ImVec2 _textSize = ImGui::GetFont()->CalcTextSizeA(_fontSize, FLT_MAX, 0.f, _glyph);
			_draw->AddText(ImGui::GetFont(), _fontSize, ImVec2(_center.x - _textSize.x * 0.5f, _center.y - _textSize.y * 0.5f), _color, _glyph);
		}

		if (_hovered) ImGui::SetTooltip("%s", _actor->GetName().c_str());
	}
	_draw->PopClipRect();
}

// Play: a colored frame around the picture, so it is clear the game runs. Edit: how to move the camera, in a corner.
void EditorLayer::DrawViewportOverlay(const ImVec2& _min, const ImVec2& _max)
{
	ImDrawList* _draw = ImGui::GetWindowDrawList();
	const float _scale = editorSettings.uiScale;

	if (context.playing)
	{
		const ImU32 _play = EditorTheme::GetU32(EditorTheme::Custom::PlayMode);
		_draw->AddRect(_min, _max, _play, 0.f, 0, 3.f * _scale);

		const std::string _text = "PLAYING  -  " + Shortcuts::Text("Play / Stop") + " or Stop to go back to the scene";
		const char* _label = _text.c_str();
		const ImVec2 _size = ImGui::CalcTextSize(_label);
		const ImVec2 _at(_min.x + (_max.x - _min.x - _size.x) * 0.5f, _min.y + 8.f * _scale);
		_draw->AddRectFilled(ImVec2(_at.x - 10.f * _scale, _at.y - 4.f * _scale), ImVec2(_at.x + _size.x + 10.f * _scale, _at.y + _size.y + 4.f * _scale), _play, 4.f * _scale);
		_draw->AddText(_at, IM_COL32(20, 20, 20, 255), _label);
		return;
	}

	const std::string _text = "Right mouse: move   Wheel: zoom   " + Shortcuts::Text("Focus Selection") + ": focus      Zoom "
		+ std::to_string(static_cast<int>(std::round(context.cameraZoom * 100.f))) + "%";
	const char* _hint = _text.c_str();
	const ImVec2 _size = ImGui::CalcTextSize(_hint);
	const ImVec2 _at(_min.x + 10.f * _scale, _max.y - _size.y - 10.f * _scale);
	_draw->AddRectFilled(ImVec2(_at.x - 6.f * _scale, _at.y - 3.f * _scale), ImVec2(_at.x + _size.x + 6.f * _scale, _at.y + _size.y + 3.f * _scale), IM_COL32(0, 0, 0, viewportHovered ? 150 : 90), 4.f * _scale);
	_draw->AddText(_at, viewportHovered ? IM_COL32(230, 232, 235, 255) : IM_COL32(230, 232, 235, 140), _hint);
}

// The editor camera, like Unreal's: hold the right mouse button (or the middle one) and move the mouse to slide the view, 
// turn the wheel to zoom where the mouse points, F to look at the selection.
void EditorLayer::MoveCamera(const ImVec2& _min, const ImVec2& _max)
{
	ImGuiIO& _io = ImGui::GetIO();
	if (context.playing || !context.cameraReady) { cameraDragging = false; return; }

	const sf::Vector2f	_windowSize(window.GetRenderWindow().getSize());
	const float			_pixels = (_max.x - _min.x) > 1.f ? _windowSize.x / (_max.x - _min.x) : 1.f;	// window pixels per screen pixel of the picture
	const bool			_over = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(_min, _max);

	if (!cameraDragging && _over && gizmoAxis < 0 && (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle)))
	{
		cameraDragging = true;
		LOG(VerbosityType::VeryVerbose, "Camera: editor camera drag started");
	}
	if (cameraDragging && !ImGui::IsMouseDown(ImGuiMouseButton_Right) && !ImGui::IsMouseDown(ImGuiMouseButton_Middle))
	{
		cameraDragging = false;
		LOG(VerbosityType::Verbose, "Camera: editor camera at (" + std::to_string(context.cameraCenter.x) + ", " + std::to_string(context.cameraCenter.y) + ") zoom " + std::to_string(context.cameraZoom));
	}

	const float _zoom = std::max(context.cameraZoom, 0.01f);
	if (cameraDragging)
	{
		context.cameraCenter.x -= _io.MouseDelta.x * _pixels / _zoom;
		context.cameraCenter.y -= _io.MouseDelta.y * _pixels / _zoom;
	}

	if (_over && _io.MouseWheel != 0.f && !_io.WantTextInput)
	{
		// Zoom around the mouse: the point of the world under it stays under it.
		const ImVec2 _middle((_min.x + _max.x) * 0.5f, (_min.y + _max.y) * 0.5f);
		const float _offsetX = (_io.MousePos.x - _middle.x) * _pixels;
		const float _offsetY = (_io.MousePos.y - _middle.y) * _pixels;
		const FVector2 _world(context.cameraCenter.x + _offsetX / _zoom, context.cameraCenter.y + _offsetY / _zoom);

		const float _newZoom = std::clamp(_zoom * std::pow(1.15f, _io.MouseWheel), 0.05f, 20.f);
		context.cameraZoom = _newZoom;
		context.cameraCenter = FVector2(_world.x - _offsetX / _newZoom, _world.y - _offsetY / _newZoom);
	}

	if (_over && !cameraDragging && context.selected && !_io.WantTextInput && Shortcuts::Pressed("Focus Selection"))
	{
		context.cameraCenter = context.selected->GetPosition();
		LOG(VerbosityType::Log, "Camera: editor camera on " + context.selected->GetName());
	}
}
