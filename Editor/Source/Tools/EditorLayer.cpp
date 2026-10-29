#include "EditorLayer.h"
#include "Layers/ScreenSpace.h"
#include "EditorThumbnails.h"
#include "Shortcuts.h"
#include "EditorTheme.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>
#include "GameFramework/Level.h"
#include "Components/CameraComponent.h"
#include "Actors/Actor.h"
#include "UI/Widgets/Widget.h"

using namespace Krampus;

namespace
{
	ImVec4 ColorOf(const VerbosityType& _type)
	{
		switch (_type)
		{
		case VerbosityType::Warning:	return EditorTheme::Get(EditorTheme::Custom::ConsoleWarning);
		case VerbosityType::Error:		return EditorTheme::Get(EditorTheme::Custom::ConsoleError);
		case VerbosityType::Fatal:		return EditorTheme::Get(EditorTheme::Custom::ConsoleFatal);
		case VerbosityType::Display:	return EditorTheme::Get(EditorTheme::Custom::ConsoleMessage);
		default:						return EditorTheme::Get(EditorTheme::Custom::ConsoleLog);
		}
	}
}

void EditorLayer::OnAttach()
{
	LOG_MSG("Editor: starting");
	Shortcuts::Initialize();
	context.history.Clear();	// files of a previous session
	EngineSettings::LoadSections(EditorSettings::GetPath(), { &editorSettings });
	std::filesystem::create_directories(std::filesystem::path(iniPath).parent_path());

	initialized = ImGui::SFML::Init(window.GetRenderWindow());
	if (!initialized)
	{
		LOG_ERROR("Editor: ImGui-SFML failed to initialize");
		Close();
		return;
	}

	ImGuiIO& _io = ImGui::GetIO();
	_io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	_io.IniFilename = iniPath.c_str();
	EditorTheme::Load();

	context.engine->SetLevelPaused(true); // edit mode: the level is shown but not running until Play
	Logger::collectEntries = true;
}

void EditorLayer::OnDetach()
{
	LOG_MSG(std::string("Editor: stopping") + (IsHotReloading() ? " (hot reload)" : ""));
	if (IsHotReloading()) SaveSettings();	// the settings changed and not saved yet survive a rebuild
	if (IsHotReloading()) SaveSession(context);
	ClearThumbnails();
	if (Level* _level = context.GetLevel()) _level->GetCameraManager().ClearViewOverride();
	Logger::collectEntries = false;
	context.engine->SetLevelPaused(false);
	window.ResetMouseViewport();
	if (initialized)
		ImGui::SFML::Shutdown();
	initialized = false;
}

// The scene is looked at through the editor's own camera while it is not playing; Play gives the view back to the cameras of the scene.
// Set every frame because each level (new scene, Undo, Stop) has its own camera manager.
void EditorLayer::OnUpdate(const Float& _deltaTime)
{
	Level* _level = context.GetLevel();
	if (!_level) return;

	const sf::Vector2f _windowSize(window.GetRenderWindow().getSize());
	if (!context.cameraReady && _windowSize.x > 0.f)
	{
		context.cameraCenter = FVector2(_windowSize.x * 0.5f, _windowSize.y * 0.5f);	// where the old default camera was
		context.cameraReady = true;
	}

	if (context.playing) _level->GetCameraManager().ClearViewOverride();
	else _level->GetCameraManager().SetViewOverride(context.GetCameraView(_windowSize));
}

Bool EditorLayer::OnEvent(const sf::Event& _event)
{
	if (!initialized) return false;

	ImGui::SFML::ProcessEvent(window.GetRenderWindow(), _event);

	const ImGuiIO& _io = ImGui::GetIO();
	if (_event.is<sf::Event::MouseButtonPressed>() || _event.is<sf::Event::MouseButtonReleased>() ||
		_event.is<sf::Event::MouseMoved>() || _event.is<sf::Event::MouseWheelScrolled>())
		return _io.WantCaptureMouse && !viewportHovered; // the game gets the mouse while it is over the Viewport picture

	if (_event.is<sf::Event::KeyPressed>() || _event.is<sf::Event::KeyReleased>() || _event.is<sf::Event::TextEntered>())
		return _io.WantCaptureKeyboard;

	return false;
}

void EditorLayer::OnRender(Window& _window)
{
	if (!initialized) return;

	sf::RenderWindow& _renderWindow = _window.GetRenderWindow();
	const sf::Vector2u _size = _renderWindow.getSize();
	if (_size.x == 0 || _size.y == 0) return;

	// Grab the finished game picture (everything pushed below this layer), then replace the window content by the editor.
	if (viewportTexture.getSize() != _size && !viewportTexture.resize(_size))
		return;
	viewportTexture.update(_renderWindow);

	const sf::View _gameView = _renderWindow.getView();
	UseScreenView(_window);
	{
		const ImVec4 _background = EditorTheme::Get(EditorTheme::Custom::Background);
		_renderWindow.clear(sf::Color(static_cast<std::uint8_t>(_background.x * 255.f), static_cast<std::uint8_t>(_background.y * 255.f), static_cast<std::uint8_t>(_background.z * 255.f)));
	}

	ImGui::GetIO().FontGlobalScale = editorSettings.uiScale;
	ImGui::SFML::Update(_renderWindow, clock.restart());
	context.history.Update(context);	// finishes an undo that swapped the scene
	HandleShortcuts();

	DrawMainMenuBar(context);
	DrawSettingsWindow();
	const ImGuiID _dockspace = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
	BuildDefaultLayout(_dockspace);

	DrawViewport();
	DrawConsole();
	DrawHierarchyPanel(context);
	DrawInspectorPanel(context);
	DrawContentPanel(context);

	ImGui::SFML::Render(_renderWindow);

	// Mouse::GetPosition maps through the current view: give back the game one, not the pixel view of the editor.
	_renderWindow.setView(_gameView);
}

// First run only (no saved imgui.ini): Hierarchy left, Inspector right, Console and Content at the bottom, Viewport in the middle.
void EditorLayer::BuildDefaultLayout(unsigned int _dockspace)
{
	if (layoutChecked) return;
	layoutChecked = true;

	const ImGuiDockNode* _node = ImGui::DockBuilderGetNode(_dockspace);
	if (_node && (!_node->IsLeafNode() || _node->Windows.Size > 0)) return;

	ImGui::DockBuilderRemoveNode(_dockspace);
	ImGui::DockBuilderAddNode(_dockspace, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(_dockspace, ImGui::GetMainViewport()->WorkSize);

	ImGuiID _center = _dockspace;
	const ImGuiID _left = ImGui::DockBuilderSplitNode(_center, ImGuiDir_Left, 0.18f, nullptr, &_center);
	const ImGuiID _right = ImGui::DockBuilderSplitNode(_center, ImGuiDir_Right, 0.24f, nullptr, &_center);
	const ImGuiID _bottom = ImGui::DockBuilderSplitNode(_center, ImGuiDir_Down, 0.28f, nullptr, &_center);

	ImGui::DockBuilderDockWindow("Hierarchy", _left);
	ImGui::DockBuilderDockWindow("Inspector", _right);
	ImGui::DockBuilderDockWindow("Console", _bottom);
	ImGui::DockBuilderDockWindow("Content", _bottom);
	ImGui::DockBuilderDockWindow("Viewport", _center);
	ImGui::DockBuilderFinish(_dockspace);
}

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
			DrawGizmo(_min, _max);
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
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
		LOG(VerbosityType::Log, "Camera: editor camera on " + context.selected->GetName().StdString());
	}
}

void EditorLayer::DrawConsole()
{
	Logger::DrainEntries(consoleLines);
	if (consoleLines.size() > static_cast<size_t>(editorSettings.consoleMaxLines))
		consoleLines.erase(consoleLines.begin(), consoleLines.begin() + (consoleLines.size() - static_cast<size_t>(editorSettings.consoleMaxLines)));

	if (ImGui::Begin("Console"))
	{
		if (ImGui::Button("Clear"))
			consoleLines.clear();
		ImGui::SameLine();
		bool _autoScroll = editorSettings.autoScrollConsole;
		if (ImGui::Checkbox("Auto-scroll", &_autoScroll))
			editorSettings.autoScrollConsole = _autoScroll;
		ImGui::Separator();

		if (ImGui::BeginChild("ConsoleLines", ImVec2(0.f, 0.f), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
		{
			for (const Logger::Entry& _line : consoleLines)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ColorOf(_line.type));
				ImGui::TextUnformatted(_line.text.c_str());
				ImGui::PopStyleColor();
			}

			if (editorSettings.autoScrollConsole && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.f);
		}
		ImGui::EndChild();
	}
	ImGui::End();
}


// One gizmo on the selected actor (or HUD widget) to move, rotate and scale it:
//   - the red and green arrows move on one axis, the yellow square moves freely;
//   - the ring turns it (Shift: steps of 15 degrees);
//   - the red and green boxes on the arrows scale on one axis, the grey box scales both (Shift: steps of 0.1).
// The buttons at the top left of the Viewport (or the keys Q W E R) keep only one of the tools.
// The gizmo is drawn in the Viewport; drags are converted back to the game world through the view the level is drawn with.
void EditorLayer::DrawGizmo(const ImVec2& _min, const ImVec2& _max)
{
	if (context.playing || !editorSettings.showGizmo) return;
	Level* _level = context.GetLevel();
	if (!_level) return;

	ITransformable* _target = nullptr;
	sf::View _view;
	if (context.selectedWidget)
	{
		_target = context.selectedWidget;
		_view = _level->GetHUD().GetView();
	}
	else if (context.selected)
	{
		_target = context.selected;
		_view = context.GetCameraView(sf::Vector2f(window.GetRenderWindow().getSize()));
	}
	if (!_target) { gizmoAxis = -1; return; }

	const sf::Vector2f	_windowSize(window.GetRenderWindow().getSize());
	const ImVec2		_rectSize(_max.x - _min.x, _max.y - _min.y);
	if (_windowSize.x < 1.f || _windowSize.y < 1.f || _rectSize.x < 1.f || _rectSize.y < 1.f) return;

	ImGuiIO& _io = ImGui::GetIO();

	// ---- Which tools are shown: buttons and keys.
	ImGui::SetCursorScreenPos(ImVec2(_min.x + 8.f, _min.y + 8.f));
	static const char* MODE_NAMES[4] = { "All (Q)", "Move (W)", "Rotate (E)", "Scale (R)" };
	for (int _mode = 0; _mode < 4; ++_mode)
	{
		if (_mode > 0) ImGui::SameLine();
		const bool _selected = gizmoMode == _mode;
		if (_selected) ImGui::PushStyleColor(ImGuiCol_Button, EditorTheme::Get(EditorTheme::Custom::Selection));
		if (ImGui::SmallButton(MODE_NAMES[_mode]) && gizmoAxis < 0) gizmoMode = _mode;
		if (_selected) ImGui::PopStyleColor();
	}
	if (viewportHovered && gizmoAxis < 0 && !_io.WantTextInput && !cameraDragging)	// W A S D fly the camera while the right button is held
	{
		if (Shortcuts::Pressed("Gizmo: All tools")) gizmoMode = 0;
		if (Shortcuts::Pressed("Gizmo: Move")) gizmoMode = 1;
		if (Shortcuts::Pressed("Gizmo: Rotate")) gizmoMode = 2;
		if (Shortcuts::Pressed("Gizmo: Scale")) gizmoMode = 3;
	}

	const bool _move = gizmoMode == 0 || gizmoMode == 1;
	const bool _rotate = gizmoMode == 0 || gizmoMode == 2;
	const bool _scale = gizmoMode == 0 || gizmoMode == 3;

	// ---- Where the selection is on the screen.
	const auto _toWorld = [&](const ImVec2& _imguiPosition)
	{
		const float _x = (_imguiPosition.x - _min.x) * _windowSize.x / _rectSize.x;
		const float _y = (_imguiPosition.y - _min.y) * _windowSize.y / _rectSize.y;
		return _view.getInverseTransform().transformPoint(sf::Vector2f(-1.f + 2.f * _x / _windowSize.x, 1.f - 2.f * _y / _windowSize.y));
	};

	const FVector2 _position = _target->GetPosition();
	const sf::Vector2i _pixel = window.GetRenderWindow().mapCoordsToPixel(sf::Vector2f(_position.x, _position.y), _view);
	const ImVec2 _origin(_min.x + _pixel.x * _rectSize.x / _windowSize.x, _min.y + _pixel.y * _rectSize.y / _windowSize.y);

	const bool _visible = _origin.x >= _min.x && _origin.x <= _max.x && _origin.y >= _min.y && _origin.y <= _max.y;
	if (!_visible && gizmoAxis < 0) return;

	// ---- Sizes. In the full gizmo the ring is inside the arrows and the scale boxes are halfway on them.
	const float ARROW = editorSettings.gizmoSize;
	const float RING = gizmoMode == 2 ? ARROW : ARROW * 0.82f;
	const float SCALE_AT = gizmoMode == 3 ? ARROW : ARROW * 0.5f;
	constexpr float GRAB = 8.f;
	constexpr float PI = 3.14159265f;

	enum Handle { MoveX = 0, MoveY = 1, MoveFree = 2, Ring = 3, ScaleX = 4, ScaleY = 5, ScaleAll = 6, HandleCount = 7 };
	bool _over[HandleCount] = { };

	ImDrawList* _draw = ImGui::GetWindowDrawList();
	_draw->PushClipRect(_min, _max, true);

	const FVector2 _currentScale = _target->GetScale();
	const float _currentRotation = _target->GetRotation().ToDegrees();
	const bool _snap = _io.KeyShift;

	// A handle is an invisible button; returns true while it is dragged. The drag starts when it is pressed.
	const auto _handle = [&](const Handle& _id, const ImVec2& _center, const ImVec2& _size, const bool& _overlapped = false) -> bool
	{
		// ImGui gives a click to the first item under the mouse: a handle that others lie on top of must let them win.
		if (_overlapped) ImGui::SetNextItemAllowOverlap();
		ImGui::SetCursorScreenPos(ImVec2(_center.x - _size.x * 0.5f, _center.y - _size.y * 0.5f));
		ImGui::PushID(static_cast<int>(_id));
		ImGui::InvisibleButton("##gizmo", _size);
		ImGui::PopID();

		_over[_id] = ImGui::IsItemHovered();
		if (ImGui::IsItemActivated())
		{
			context.history.Record(context, _id >= ScaleX ? "Scale" : "Move");	// the state before the drag
			gizmoAxis = _id;
			gizmoStartMouse = _io.MouseClickedPos[ImGuiMouseButton_Left];	// where the button went down, not where the mouse is by now
			gizmoStartPosition = _position;
			gizmoStartRotation = _currentRotation;
			gizmoStartScale = _currentScale;
			LOG(VerbosityType::VeryVerbose, "Gizmo: drag started on handle " + std::to_string(static_cast<int>(_id)));
		}
		return gizmoAxis == _id && ImGui::IsItemActive();
	};

	// ---- Move (arrows and square). Submitted first: the scale boxes, on top of the arrows, win where they overlap.
	if (_move)
	{
		const ImVec2 _sizeX(ARROW - 12.f + 6.f, GRAB * 2.f);
		const ImVec2 _sizeY(GRAB * 2.f, ARROW - 12.f + 6.f);
		const bool _dragX = _handle(MoveX, ImVec2(_origin.x + 12.f + _sizeX.x * 0.5f, _origin.y), _sizeX, true);
		const bool _dragY = _handle(MoveY, ImVec2(_origin.x, _origin.y + 12.f + _sizeY.y * 0.5f), _sizeY, true);
		const bool _dragFree = _handle(MoveFree, _origin, ImVec2(16.f, 16.f));

		if (_dragX || _dragY || _dragFree)
		{
			const sf::Vector2f _start = _toWorld(gizmoStartMouse);
			const sf::Vector2f _now = _toWorld(_io.MousePos);
			FVector2 _next = gizmoStartPosition;
			if (!_dragY) _next.x += _now.x - _start.x;
			if (!_dragX) _next.y += _now.y - _start.y;
			_target->SetPosition(_next);
		}
	}

	// ---- Scale (boxes).
	if (_scale)
	{
		const ImVec2 _box(12.f, 12.f);
		const bool _dragX = _handle(ScaleX, ImVec2(_origin.x + SCALE_AT, _origin.y), _box);
		const bool _dragY = _handle(ScaleY, ImVec2(_origin.x, _origin.y + SCALE_AT), _box);
		const ImVec2 _allCenter = gizmoMode == 3 ? _origin : ImVec2(_origin.x + ARROW * 0.3f, _origin.y + ARROW * 0.3f);
		const bool _dragAll = _handle(ScaleAll, _allCenter, _box);

		if (_dragX || _dragY || _dragAll)
		{
			const ImVec2 _delta(_io.MousePos.x - gizmoStartMouse.x, _io.MousePos.y - gizmoStartMouse.y);
			// A drag of SCALE_AT pixels doubles the scale (or the whole ARROW length when the box is at the end).
			const float _reach = std::max(1.f, SCALE_AT);
			const auto _factor = [&](const float& _pixels)
			{
				float _f = std::max(0.01f, 1.f + _pixels / _reach);
				if (_snap) _f = std::max(0.1f, std::round(_f * 10.f) / 10.f);
				return _f;
			};

			FVector2 _next = gizmoStartScale;
			if (_dragX) _next.x *= _factor(_delta.x);
			if (_dragY) _next.y *= _factor(_delta.y);
			if (_dragAll)
			{
				const float _all = _factor((_delta.x + _delta.y) * 0.7071f);
				_next.x *= _all;
				_next.y *= _all;
			}
			_target->SetScale(_next);
		}
	}

	// ---- Rotate (ring): no button can have a round shape, the mouse distance to the center is tested instead.
	if (_rotate)
	{
		const ImVec2 _mouse = _io.MousePos;
		const float _distance = std::sqrt((_mouse.x - _origin.x) * (_mouse.x - _origin.x) + (_mouse.y - _origin.y) * (_mouse.y - _origin.y));
		const bool _inside = _mouse.x >= _min.x && _mouse.x <= _max.x && _mouse.y >= _min.y && _mouse.y <= _max.y;
		const bool _otherHandle = _over[MoveX] || _over[MoveY] || _over[MoveFree] || _over[ScaleX] || _over[ScaleY] || _over[ScaleAll];

		_over[Ring] = gizmoAxis < 0 && _inside && !_otherHandle && ImGui::IsWindowHovered() && std::fabs(_distance - RING) < 7.f;
		if (_over[Ring] && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			context.history.Record(context, "Rotate");
			gizmoAxis = Ring;
			gizmoStartMouse = _io.MouseClickedPos[ImGuiMouseButton_Left];
			gizmoStartPosition = _position;
			gizmoStartRotation = _currentRotation;
			gizmoStartScale = _currentScale;
			gizmoLastAngle = std::atan2(gizmoStartMouse.y - _origin.y, gizmoStartMouse.x - _origin.x);
			gizmoAccum = 0.f;
			LOG(VerbosityType::VeryVerbose, "Gizmo: drag started on the rotation ring");
		}

		if (gizmoAxis == Ring && ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			// Added up from one frame to the next, so that more than half a turn works.
			const float _angle = std::atan2(_mouse.y - _origin.y, _mouse.x - _origin.x);
			float _step = _angle - gizmoLastAngle;
			while (_step > PI) _step -= 2.f * PI;
			while (_step < -PI) _step += 2.f * PI;
			gizmoAccum += _step;
			gizmoLastAngle = _angle;

			float _degrees = gizmoStartRotation + gizmoAccum * 180.f / PI;
			if (_snap) _degrees = std::round(_degrees / 15.f) * 15.f;
			_target->SetRotation(Angle::FromDegrees(_degrees));
		}
	}

	// ---- End of a drag.
	if (gizmoAxis >= 0)
	{
		const bool _dragging = ImGui::IsMouseDown(ImGuiMouseButton_Left);
		if (!_dragging)
		{
			const FVector2 _endPosition = _target->GetPosition();
			const FVector2 _endScale = _target->GetScale();
			if (gizmoAxis == Ring)
				LOG(VerbosityType::Log, "Gizmo: rotated from " + std::to_string(gizmoStartRotation) + " to " + std::to_string(static_cast<float>(_target->GetRotation().ToDegrees())) + " degrees");
			else if (gizmoAxis >= ScaleX)
				LOG(VerbosityType::Log, "Gizmo: scaled from (" + std::to_string(gizmoStartScale.x) + ", " + std::to_string(gizmoStartScale.y) + ") to (" + std::to_string(_endScale.x) + ", " + std::to_string(_endScale.y) + ")");
			else
				LOG(VerbosityType::Log, "Gizmo: moved from (" + std::to_string(gizmoStartPosition.x) + ", " + std::to_string(gizmoStartPosition.y) + ") to (" + std::to_string(_endPosition.x) + ", " + std::to_string(_endPosition.y) + ")");
			gizmoAxis = -1;
		}
	}

	// ---- Drawing.
	const auto _hot = [&](const Handle& _id) { return _over[_id] || gizmoAxis == _id; };
	const ImU32 _baseRed = EditorTheme::GetU32(EditorTheme::Custom::GizmoX);
	const ImU32 _baseGreen = EditorTheme::GetU32(EditorTheme::Custom::GizmoY);
	const ImU32 _baseYellow = EditorTheme::GetU32(EditorTheme::Custom::GizmoFree);
	const ImU32 _baseRing = EditorTheme::GetU32(EditorTheme::Custom::GizmoRing);
	const ImU32 _red = _hot(MoveX) ? EditorTheme::Lighten(_baseRed) : _baseRed;
	const ImU32 _green = _hot(MoveY) ? EditorTheme::Lighten(_baseGreen) : _baseGreen;
	const ImU32 _yellow = _hot(MoveFree) ? EditorTheme::Lighten(_baseYellow) : _baseYellow;
	const ImU32 _black = IM_COL32(0, 0, 0, 200);

	if (_rotate)
	{
		const ImU32 _ring = _hot(Ring) ? EditorTheme::Lighten(_baseRing) : _baseRing;
		_draw->AddCircle(_origin, RING, IM_COL32(0, 0, 0, 120), 72, 5.f);
		_draw->AddCircle(_origin, RING, _ring, 72, _hot(Ring) ? 3.f : 2.f);

		// Where the selection points now.
		const float _rotation = _target->GetRotation().ToDegrees() * PI / 180.f;
		const ImVec2 _tick(_origin.x + std::cos(_rotation) * RING, _origin.y + std::sin(_rotation) * RING);
		_draw->AddCircleFilled(_tick, 4.f, _ring);
		_draw->AddCircle(_tick, 4.f, _black);

		if (gizmoAxis == Ring)
		{
			const float _start = gizmoLastAngle - gizmoAccum;
			_draw->PathLineTo(_origin);
			_draw->PathArcTo(_origin, RING, _start, gizmoLastAngle, 48);
			_draw->PathFillConvex(IM_COL32(255, 200, 60, 70));
			_draw->AddLine(_origin, _io.MousePos, IM_COL32(255, 200, 60, 200), 1.5f);
		}
	}

	if (_move)
	{
		const ImVec2 _endX(_origin.x + ARROW, _origin.y);
		_draw->AddLine(_origin, _endX, _red, 3.f);
		_draw->AddTriangleFilled(ImVec2(_endX.x + 10.f, _endX.y), ImVec2(_endX.x - 4.f, _endX.y - 6.f), ImVec2(_endX.x - 4.f, _endX.y + 6.f), _red);

		const ImVec2 _endY(_origin.x, _origin.y + ARROW);
		_draw->AddLine(_origin, _endY, _green, 3.f);
		_draw->AddTriangleFilled(ImVec2(_endY.x, _endY.y + 10.f), ImVec2(_endY.x - 6.f, _endY.y - 4.f), ImVec2(_endY.x + 6.f, _endY.y - 4.f), _green);

		_draw->AddRectFilled(ImVec2(_origin.x - 6.f, _origin.y - 6.f), ImVec2(_origin.x + 6.f, _origin.y + 6.f), _yellow, 2.f);
		_draw->AddRect(ImVec2(_origin.x - 6.f, _origin.y - 6.f), ImVec2(_origin.x + 6.f, _origin.y + 6.f), _black, 2.f);
	}

	if (_scale)
	{
		const ImU32 _redBox = _hot(ScaleX) ? EditorTheme::Lighten(_baseRed) : EditorTheme::Darken(_baseRed);
		const ImU32 _greenBox = _hot(ScaleY) ? EditorTheme::Lighten(_baseGreen) : EditorTheme::Darken(_baseGreen);
		const ImU32 _greyBox = _hot(ScaleAll) ? IM_COL32(255, 255, 255, 255) : IM_COL32(190, 190, 200, 255);

		const ImVec2 _x(_origin.x + SCALE_AT, _origin.y);
		const ImVec2 _y(_origin.x, _origin.y + SCALE_AT);
		const ImVec2 _all = gizmoMode == 3 ? _origin : ImVec2(_origin.x + ARROW * 0.3f, _origin.y + ARROW * 0.3f);

		if (gizmoMode == 3)	// alone, the scale tool needs its own axes
		{
			_draw->AddLine(_origin, _x, _redBox, 2.f);
			_draw->AddLine(_origin, _y, _greenBox, 2.f);
		}
		_draw->AddRectFilled(ImVec2(_x.x - 5.f, _x.y - 5.f), ImVec2(_x.x + 5.f, _x.y + 5.f), _redBox);
		_draw->AddRect(ImVec2(_x.x - 5.f, _x.y - 5.f), ImVec2(_x.x + 5.f, _x.y + 5.f), _black);
		_draw->AddRectFilled(ImVec2(_y.x - 5.f, _y.y - 5.f), ImVec2(_y.x + 5.f, _y.y + 5.f), _greenBox);
		_draw->AddRect(ImVec2(_y.x - 5.f, _y.y - 5.f), ImVec2(_y.x + 5.f, _y.y + 5.f), _black);
		_draw->AddRectFilled(ImVec2(_all.x - 5.f, _all.y - 5.f), ImVec2(_all.x + 5.f, _all.y + 5.f), _greyBox);
		_draw->AddRect(ImVec2(_all.x - 5.f, _all.y - 5.f), ImVec2(_all.x + 5.f, _all.y + 5.f), _black);
	}

	// The value being changed, next to the mouse.
	if (gizmoAxis >= 0)
	{
		char _text[96];
		if (gizmoAxis == Ring)
			std::snprintf(_text, sizeof(_text), "Rotation %.1f deg", _target->GetRotation().ToDegrees());
		else if (gizmoAxis >= ScaleX)
			std::snprintf(_text, sizeof(_text), "Scale %.2f x %.2f", _target->GetScale().x, _target->GetScale().y);
		else
			std::snprintf(_text, sizeof(_text), "Position %.0f, %.0f", _target->GetPosition().x, _target->GetPosition().y);

		const ImVec2 _size = ImGui::CalcTextSize(_text);
		const ImVec2 _at(_io.MousePos.x + 16.f, _io.MousePos.y - 22.f);
		_draw->AddRectFilled(ImVec2(_at.x - 4.f, _at.y - 2.f), ImVec2(_at.x + _size.x + 4.f, _at.y + _size.y + 2.f), IM_COL32(20, 20, 24, 220), 3.f);
		_draw->AddText(_at, IM_COL32(255, 255, 255, 255), _text);
	}

	_draw->PopClipRect();
}


// Shortcuts that work anywhere in the editor. The others are checked where they apply (menu, Viewport, Content).
void EditorLayer::HandleShortcuts()
{
	if (Shortcuts::Pressed("Undo"))
		context.history.Undo(context);
	if (Shortcuts::Pressed("Redo") || Shortcuts::Pressed("Redo (second key)"))
		context.history.Redo(context);

	if (Shortcuts::Pressed("Play / Stop"))
	{
		if (context.playing) Stop(context);
		else Play(context);
	}
	if (Shortcuts::Pressed("Compile") && !IsCompiling())
		StartCompile();
	if (Shortcuts::Pressed("Open Solution"))
		OpenSolution();
	if (Shortcuts::Pressed("Package Game") && !IsCompiling())
		StartShip();

	// Delete in the Viewport (the Hierarchy checks its own focus).
	if (viewportHovered && Shortcuts::Pressed("Delete Selection"))
		DeleteSelection(context);
}
