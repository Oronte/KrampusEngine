#include "EditorLayer.h"
#include "Layers/ScreenSpace.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>

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
