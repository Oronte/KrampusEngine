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


void EditorLayer::OnAttach()
{
	LOG_MSG("Editor: starting");
	MoveOldEditorFiles();	// first: everything below reads its file in Content/_internal/Editor
	Shortcuts::Initialize();
	context.history.Clear();	// files of a previous session
	EngineSettings::LoadSections(EditorSettings::GetPath(), { &editorSettings });
	std::filesystem::create_directories(std::filesystem::path(iniPath).parent_path());
	ShapeAssets::CreateStarterShapes();	// a first project gets a few shapes for the "polygon" properties

	initialized = ImGui::SFML::Init(window.GetRenderWindow());
	if (!initialized)
	{
		LOG_ERROR("Editor: ImGui-SFML failed to initialize");
		Close();
		return;
	}

	FileDrop::Install(reinterpret_cast<void*>(window.GetRenderWindow().getNativeHandle()));	// files dropped from the Explorer go to Content

	ImGuiIO& _io = ImGui::GetIO();
	_io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	_io.ConfigDragClickToInputText = true;	// a click on a drag field types the value
	_io.IniFilename = iniPath.c_str();
	EditorTheme::Load();

	context.engine->SetLevelPaused(true); // edit mode: the level is shown but not running until Play
	Logger::collectEntries = true;
}

void EditorLayer::OnDetach()
{
	LOG_MSG(std::string("Editor: stopping") + (IsHotReloading() ? " (hot reload)" : ""));
	FileDrop::Uninstall();	// first: Windows must not call into this module once it is unloaded
	Debug::displayScale = 1.f;
	if (IsHotReloading()) SaveSettings();	// the settings changed and not saved yet survive a rebuild
	if (IsHotReloading()) SaveShapeEditorChanges();
	if (IsHotReloading()) SaveShaderEditorChanges();
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

	// Closing with unsaved changes: the editor asks first (EditorMenu.cpp). The event is kept from the engine, which would quit.
	// A second close request while the question is shown (X clicked again, Alt+F4) is taken as "quit": the editor must always close.
	if (_event.is<sf::Event::Closed>() && context.SceneDirty() && !context.quitAsked)
	{
		context.askQuit = true;
		context.quitAsked = true;
		return true;
	}

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
	PROFILE_SCOPE("Editor");

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

	// Font and sizes made again for a new interface scale (once the slider is let go: making the font takes a moment).
	if (appliedUiScale != editorSettings.uiScale && (appliedUiScale == 0.f || !ImGui::IsMouseDown(ImGuiMouseButton_Left)))
	{
		appliedUiScale = editorSettings.uiScale;
		EditorTheme::LoadFonts(appliedUiScale);
		EditorTheme::ApplyStyle(appliedUiScale);
	}
	ImGui::SFML::Update(_renderWindow, clock.restart());
	context.history.Update(context);	// finishes an undo that swapped the scene
	context.SyncSelection();
	ShaderLibrary::CheckChanges();		// a shader file changed by another program is compiled again
	HandleShortcuts();

	DrawMainMenuBar(context);
	DrawStatusBar(context);	// before the dock space: it takes the bottom of the window
	DrawSettingsWindow();
	const ImGuiID _dockspace = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
	BuildDefaultLayout(_dockspace);

	DrawViewport();
	DrawConsole();
	DrawHierarchyPanel(context);
	DrawInspectorPanel(context);
	DrawContentPanel(context);
	DrawAnimationEditor(context);
	DrawAnimationGraph(context);
	DrawShaderEditor(context);
	DrawNetworkWindow(context);
	DrawGameplayTagsWindow(context);
	DrawCommandPalette(context);
	DrawProfiler(context);
	DrawShapeEditor(context);

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
		StartShip(context);

	if (Shortcuts::Pressed("Copy")) CopySelection(context);
	if (Shortcuts::Pressed("Paste")) PasteActors(context);
	if (Shortcuts::Pressed("Duplicate")) DuplicateSelection(context);

	// Delete in the Viewport (the Hierarchy checks its own focus).
	// Show Navigation: the P key, the Tools menu (it changes NavigationSystem::showNavigation) and the Settings window agree.
	if (NavigationSystem::showNavigation != appliedNavigation)
	{
		editorSettings.showNavigation = NavigationSystem::showNavigation;
		settingsDirty = true;
	}
	if (viewportHovered && !ImGui::GetIO().WantTextInput && Shortcuts::Pressed("Show Navigation"))
	{
		editorSettings.showNavigation = !editorSettings.showNavigation;
		settingsDirty = true;
	}
	NavigationSystem::showNavigation = editorSettings.showNavigation;
	appliedNavigation = editorSettings.showNavigation;

	if (viewportHovered && Shortcuts::Pressed("Delete Selection"))
		DeleteSelection(context);
}
