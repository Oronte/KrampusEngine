#include "EditorLayer.h"
#include "Layers/ScreenSpace.h"
#include "EditorThumbnails.h"
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
		case VerbosityType::Warning:	return ImVec4(1.f, 0.85f, 0.2f, 1.f);
		case VerbosityType::Error:		return ImVec4(1.f, 0.35f, 0.35f, 1.f);
		case VerbosityType::Fatal:		return ImVec4(1.f, 0.2f, 0.6f, 1.f);
		case VerbosityType::Display:	return ImVec4(1.f, 1.f, 1.f, 1.f);
		default:						return ImVec4(0.6f, 0.6f, 0.7f, 1.f);
		}
	}
}

void EditorLayer::OnAttach()
{
	LOG_MSG("Editor: starting");
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

	context.engine->SetLevelPaused(true); // edit mode: the level is shown but not running until Play
	Logger::collectEntries = true;
}

void EditorLayer::OnDetach()
{
	LOG_MSG(std::string("Editor: stopping") + (IsHotReloading() ? " (hot reload)" : ""));
	if (IsHotReloading()) SaveSession(context);
	ClearThumbnails();
	Logger::collectEntries = false;
	context.engine->SetLevelPaused(false);
	window.ResetMouseViewport();
	if (initialized)
		ImGui::SFML::Shutdown();
	initialized = false;
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
	_renderWindow.clear(sf::Color(25, 25, 28));

	ImGui::SFML::Update(_renderWindow, clock.restart());

	DrawMainMenuBar(context);
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
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void EditorLayer::DrawConsole()
{
	Logger::DrainEntries(consoleLines);
	if (consoleLines.size() > MAX_CONSOLE_LINES)
		consoleLines.erase(consoleLines.begin(), consoleLines.begin() + (consoleLines.size() - MAX_CONSOLE_LINES));

	if (ImGui::Begin("Console"))
	{
		if (ImGui::Button("Clear"))
			consoleLines.clear();
		ImGui::SameLine();
		ImGui::Checkbox("Auto-scroll", &autoScroll);
		ImGui::Separator();

		if (ImGui::BeginChild("ConsoleLines", ImVec2(0.f, 0.f), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
		{
			for (const Logger::Entry& _line : consoleLines)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ColorOf(_line.type));
				ImGui::TextUnformatted(_line.text.c_str());
				ImGui::PopStyleColor();
			}

			if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.f);
		}
		ImGui::EndChild();
	}
	ImGui::End();
}


// Move arrows on the selected actor (or HUD widget): drag the red arrow for X, the green one for Y, the square for both.
// The arrows are drawn in the Viewport; the position is converted back to the game world through the view the level is drawn with.
void EditorLayer::DrawGizmo(const ImVec2& _min, const ImVec2& _max)
{
	if (context.playing) return;
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
		CameraComponent* _camera = _level->GetCameraManager().GetCurrent();
		if (!_camera) _camera = _level->GetDefaultCamera();
		if (!_camera || !_camera->GetView()) return;
		_target = context.selected;
		_view = *_camera->GetView();
	}
	if (!_target) { gizmoAxis = -1; return; }

	const sf::Vector2f	_windowSize(window.GetRenderWindow().getSize());
	const ImVec2		_rectSize(_max.x - _min.x, _max.y - _min.y);
	if (_windowSize.x < 1.f || _windowSize.y < 1.f || _rectSize.x < 1.f || _rectSize.y < 1.f) return;

	// window pixel <-> ImGui position inside the picture
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

	constexpr float ARROW = 70.f;
	constexpr float GRAB = 8.f;

	// Handles: 0 = X arrow, 1 = Y arrow, 2 = center
	const ImVec2 _handleMin[3] = { ImVec2(_origin.x + 12.f, _origin.y - GRAB), ImVec2(_origin.x - GRAB, _origin.y + 12.f), ImVec2(_origin.x - 8.f, _origin.y - 8.f) };
	const ImVec2 _handleSize[3] = { ImVec2(ARROW - 12.f + 6.f, GRAB * 2.f), ImVec2(GRAB * 2.f, ARROW - 12.f + 6.f), ImVec2(16.f, 16.f) };
	bool _hovered[3] = { false, false, false };

	ImDrawList* _draw = ImGui::GetWindowDrawList();
	_draw->PushClipRect(_min, _max, true);

	for (int _i = 0; _i < 3; ++_i)
	{
		ImGui::SetCursorScreenPos(_handleMin[_i]);
		ImGui::PushID(_i);
		ImGui::InvisibleButton("##gizmo", _handleSize[_i]);
		ImGui::PopID();

		_hovered[_i] = ImGui::IsItemHovered() || (gizmoAxis == _i);

		if (ImGui::IsItemActivated())
		{
			gizmoAxis = _i;
			gizmoStartMouse = ImGui::GetIO().MousePos;
			LOG(VerbosityType::VeryVerbose, "Gizmo: drag started on axis " + std::to_string(_i));
			gizmoStartPosition = _position;
		}
		if (gizmoAxis == _i && ImGui::IsItemActive())
		{
			const sf::Vector2f _start = _toWorld(gizmoStartMouse);
			const sf::Vector2f _now = _toWorld(ImGui::GetIO().MousePos);
			FVector2 _next = gizmoStartPosition;
			if (_i != 1) _next.x += _now.x - _start.x;
			if (_i != 0) _next.y += _now.y - _start.y;
			_target->SetPosition(_next);
		}
		if (gizmoAxis == _i && !ImGui::IsItemActive())
		{
			const FVector2 _end = _target->GetPosition();
			LOG(VerbosityType::Log, "Gizmo: moved from (" + std::to_string(gizmoStartPosition.x) + ", " + std::to_string(gizmoStartPosition.y) + ") to (" + std::to_string(_end.x) + ", " + std::to_string(_end.y) + ")");
			gizmoAxis = -1;
		}
	}

	const ImU32 _red = _hovered[0] ? IM_COL32(255, 140, 140, 255) : IM_COL32(230, 60, 60, 255);
	const ImU32 _green = _hovered[1] ? IM_COL32(150, 255, 150, 255) : IM_COL32(70, 200, 80, 255);
	const ImU32 _yellow = _hovered[2] ? IM_COL32(255, 245, 160, 255) : IM_COL32(240, 200, 60, 255);

	const ImVec2 _endX(_origin.x + ARROW, _origin.y);
	_draw->AddLine(_origin, _endX, _red, 3.f);
	_draw->AddTriangleFilled(ImVec2(_endX.x + 10.f, _endX.y), ImVec2(_endX.x - 4.f, _endX.y - 6.f), ImVec2(_endX.x - 4.f, _endX.y + 6.f), _red);

	const ImVec2 _endY(_origin.x, _origin.y + ARROW);
	_draw->AddLine(_origin, _endY, _green, 3.f);
	_draw->AddTriangleFilled(ImVec2(_endY.x, _endY.y + 10.f), ImVec2(_endY.x - 6.f, _endY.y - 4.f), ImVec2(_endY.x + 6.f, _endY.y - 4.f), _green);

	_draw->AddRectFilled(ImVec2(_origin.x - 6.f, _origin.y - 6.f), ImVec2(_origin.x + 6.f, _origin.y + 6.f), _yellow, 2.f);
	_draw->AddRect(ImVec2(_origin.x - 6.f, _origin.y - 6.f), ImVec2(_origin.x + 6.f, _origin.y + 6.f), IM_COL32(0, 0, 0, 200), 2.f);

	_draw->PopClipRect();
}
