#pragma once
#include "Core/Engine.h"
#include "EditHistory.h"

namespace Krampus
{

	class Actor;
	class Level;
	class Widget;

	/// <summary>
	/// State shared by the editor panels (Hierarchy, Inspector, Content...) and its menu.
	/// </summary>
	struct EditorContext
	{
		Engine*						engine		= nullptr;
		Actor*						selected	= nullptr;
		Widget*						selectedWidget	= nullptr;	// a HUD widget is selected instead of an actor
		bool						playing		= false;	// the level runs (Play) or is frozen (edit mode)
		bool						hasSnapshot	= false;	// the scene was saved at Play and is restored at Stop
		bool						settingsOpen	= false;	// the Settings window is shown
		int							settingsPage	= 0;		// section shown in it
		std::filesystem::path		scenePath;	// scene being edited, empty until it is saved once
		EditHistory					history;	// Undo / Redo

		// The editor's own camera, used to look at the scene when it is not playing (right mouse to move, wheel to zoom).
		// It is not an actor of the scene: it is never saved and the game does not see it.
		bool						cameraReady	= false;	// false until it has been put at the center of the window once
		FVector2					cameraCenter;			// world position seen at the middle of the Viewport
		float						cameraZoom	= 1.f;		// 2 = twice as close

		/// <returns>The level shown in the Viewport.</returns>
		Level* GetLevel() const;

		/// <returns>The view of the editor's camera for a window of this size.</returns>
		sf::View GetCameraView(const sf::Vector2f& _windowSize) const;
	};

	inline const std::filesystem::path		CONTENT_DIRECTORY	= "Content";
	inline const std::filesystem::path		SCENES_DIRECTORY	= "Content/Scenes";
	inline const std::filesystem::path		SCRIPTS_DIRECTORY	= "Content/Scripts";
	inline const std::filesystem::path		PLAY_SNAPSHOT_PATH	= "Content/Editor/_play_snapshot.ka7";	// scene saved at Play, restored at Stop
	inline const std::filesystem::path		RELOAD_BACKUP_PATH	= "Content/Editor/_reload_backup.ka7";	// scene being edited, saved before the game is reloaded
	inline const std::filesystem::path		SESSION_FILE		= "Content/Editor/session.txt";				// path of that scene

	// Scene commands (EditorMenu.cpp)
	void NewScene(EditorContext& _context);
	void OpenScene(EditorContext& _context, const std::filesystem::path& _path);
	bool SaveScene(EditorContext& _context, const std::filesystem::path& _path);

	// Play mode (PlayMode.cpp): Play runs the level, Stop freezes it and brings back the scene as it was before Play.
	void Play(EditorContext& _context);
	void Stop(EditorContext& _context);

	// Compile button (Compile.cpp): builds EditorModule in the background, messages go to the Console.
	void StartCompile();

	/// <summary>
	/// Builds the engine and the game in Ship mode and gathers the game to give away (exe, DLLs, Content) in GetPackageDirectory().
	/// Same background build as Compile.
	/// </summary>
	void StartShip();
	bool IsShipping();	// the build in progress is a package
	std::filesystem::path GetPackageDirectory();
	bool IsCompiling();
	void PumpCompile();	// every frame

	// Hot reload (Session.cpp): the scene being edited survives a rebuild of the game.
	bool IsHotReloading();
	void SaveSession(EditorContext& _context);
	void RestoreSession(EditorContext& _context);

	// Visual Studio (VisualStudio.cpp): the solution is how the game is compiled; the editor then reloads it by itself.

	/// <summary>
	/// Opens the solution in Visual Studio (generated, or updated with the newest scripts, by CMake first) and, if given, a source file of it.
	/// </summary>
	void OpenSolution(const std::filesystem::path& _file = { });

	/// <summary>
	/// Adds to the solution the scripts created since it was generated, and with _force drops the deleted ones (does not wait; does nothing if there is no solution yet).
	/// </summary>
	void RefreshSolution(const bool& _force = false);

	/// <summary>
	/// Runs CMake (and waits) if scripts were added or removed since the solution was generated, so that the next build sees them.
	/// </summary>
	void PrepareSolutionForBuild();

	/// <summary>
	/// Draws the properties of an object (the Inspector widgets). Returns true if one was edited this frame.
	/// </summary>
	bool DrawObjectProperties(IGameObject& _object);

	/// <summary>
	/// Deletes the selected actor or HUD widget (the default camera cannot be deleted). Undoable.
	/// </summary>
	void DeleteSelection(EditorContext& _context);

	// Panels and menu, drawn every frame between ImGui::NewFrame and Render.
	void DrawMainMenuBar(EditorContext& _context);
	void DrawHierarchyPanel(EditorContext& _context);
	void DrawInspectorPanel(EditorContext& _context);
	void DrawContentPanel(EditorContext& _context);
}
