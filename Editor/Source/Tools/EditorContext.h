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

		// Unsaved changes: every recorded edit (and undo / redo) counts; saving remembers the count.
		int							sceneChanges	= 0;
		int							savedChanges	= 0;
		bool SceneDirty() const { return sceneChanges != savedChanges; }

		// A scene to open, or the editor to close, once the user answered about the unsaved changes (EditorMenu.cpp).
		std::filesystem::path		pendingScene;
		bool						askUnsaved		= false;	// show the question for pendingScene
		bool						askQuit			= false;	// show the question before closing

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
	// Every file of the editor itself (preferences, layout, undo steps, session) is in Content/_internal/Editor: the Content panel
	// never shows the _internal folders. Files of an older version in Content/Editor are moved there at start (MoveOldEditorFiles).
	inline const std::filesystem::path		EDITOR_DATA_DIRECTORY	= "Content/_internal/Editor";
	inline const std::filesystem::path		OLD_EDITOR_DATA_DIRECTORY	= "Content/Editor";
	inline const std::filesystem::path		PLAY_SNAPSHOT_PATH	= EDITOR_DATA_DIRECTORY / "_play_snapshot.ka7";	// scene saved at Play, restored at Stop
	inline const std::filesystem::path		RELOAD_BACKUP_PATH	= EDITOR_DATA_DIRECTORY / "_reload_backup.ka7";	// scene being edited, saved before the game is reloaded
	inline const std::filesystem::path		SESSION_FILE		= EDITOR_DATA_DIRECTORY / "session.txt";			// path of that scene

	/// <returns>True for a path that has an _internal folder in it (engine and editor files, hidden from the Content panel).</returns>
	inline bool IsInternalPath(const std::filesystem::path& _path)
	{
		for (const std::filesystem::path& _part : _path)
			if (_part == "_internal") return true;
		return false;
	}

	/// <summary>Moves the files of Content/Editor (older versions) to Content/_internal/Editor. Called once when the editor starts.</summary>
	void MoveOldEditorFiles();

	// Scene commands (EditorMenu.cpp)
	void NewScene(EditorContext& _context);
	void OpenScene(EditorContext& _context, const std::filesystem::path& _path);
	bool SaveScene(EditorContext& _context, const std::filesystem::path& _path);
	/// <summary>Opens a scene (empty path: a new one), after asking what to do with the unsaved changes of the current one.</summary>
	void RequestOpenScene(EditorContext& _context, const std::filesystem::path& _path);

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
	/// <summary>The bar at the bottom of the window: scene and its unsaved changes, mode, selection, build, frame rate.</summary>
	void DrawStatusBar(EditorContext& _context);
	void DrawHierarchyPanel(EditorContext& _context);
	void DrawInspectorPanel(EditorContext& _context);
	void DrawContentPanel(EditorContext& _context);

	/// <summary>Opens the Animation Editor window (it edits the animation component of the selected actor or widget).</summary>
	void OpenAnimationEditor();
	void DrawAnimationEditor(EditorContext& _context);

	/// <summary>Opens the Network window (host, join, players, messages).</summary>
	void OpenNetworkWindow();
	void DrawNetworkWindow(EditorContext& _context);

	/// <summary>Opens the Shape Editor (the polygons of Content/Shapes), on _shape when it is given.</summary>
	void OpenShapeEditor(const std::string& _shape = "");
	void DrawShapeEditor(EditorContext& _context);
	/// <summary>Saves the shape being edited if it has changes (before a hot reload, which would lose them).</summary>
	void SaveShapeEditorChanges();
}
