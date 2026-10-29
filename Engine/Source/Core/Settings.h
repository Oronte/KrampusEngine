#pragma once
#include "Core/GameObject.h"
#include "Utilities/Math/Vector2D.h"

// Engine settings: the Logger, the physics world, time, window and audio. They live in a file (Content/Settings/EngineSettings.cfg),
// are loaded when the engine starts and applied to every level when it starts. The editor edits them in its Settings window,
// through the same reflection as the Inspector: each section is an object whose properties are described in Reflection/EngineClasses.cpp.

namespace Krampus
{

	class Engine;
	class Level;
	class WorldPhysics;

	/// <summary>
	/// A group of settings (Logger, Physics...). Properties are reflected, so a window can draw and save them without knowing them.
	/// </summary>
	class SettingsSection : public IGameObject
	{
	protected:
		Engine*		engine		= nullptr;

	public:
		/// <summary>Name shown by the settings window.</summary>
		virtual const char* Title() const = 0;

		/// <summary>One line telling what the section is for.</summary>
		virtual const char* Description() const { return ""; }

		/// <summary>Gives the engine the section applies to.</summary>
		void Attach(Engine* _engine) { engine = _engine; }

		/// <summary>Pushes the values to the running engine (and to the level in progress).</summary>
		virtual void Apply() { }

		/// <summary>Pushes the values to a level that starts.</summary>
		virtual void ApplyToLevel(Level& _level) { }

		/// <summary>Puts the values back to the ones of a new project.</summary>
		virtual void ResetToDefaults() { }

		void Construct() override { }
		void Deconstruct() override { }
		void BeginPlay() override { }
		void Tick(const Float& _deltaTime) override { }
		void BeginDestroy() override { }
	};

	/// <summary>
	/// Logger. The values are the static fields of Logger; changing them takes effect at once.
	/// </summary>
	class LoggerSettings : public SettingsSection
	{
	public:
		const char* Title() const override { return "Logger"; }
		const char* Description() const override { return "What is written to the log file (Content/Logs/log.txt) and the Console, and the sounds that go with it."; }
		void ResetToDefaults() override;
	};

	/// <summary>
	/// Physics world (box2d). Applied to each level when it starts and to the level in progress when changed.
	/// </summary>
	class PhysicsSettings : public SettingsSection
	{
	public:
		FVector2	gravity					= FVector2(0.f, 1000.f);	// pixels per second squared, y down
		int			subSteps				= 8;
		bool		sleeping				= true;						// resting bodies stop being simulated
		bool		continuous				= true;						// fast bodies do not go through thin ones
		bool		warmStarting			= true;
		float		maxLinearSpeed			= 400.f;					// meters per second
		float		restitutionThreshold	= 1.f;						// below this speed (m/s) nothing bounces
		float		hitEventThreshold		= 1.f;						// m/s
		float		contactHertz			= 30.f;						// contact stiffness
		float		contactDampingRatio		= 10.f;
		float		maxContactPushSpeed		= 3.f;						// m/s, how fast an overlap is pushed apart

		PhysicsSettings();

		const char* Title() const override { return "Physics"; }
		const char* Description() const override { return "The box2d world every level creates: gravity, solver quality and thresholds."; }
		void Apply() override;
		void ApplyToLevel(Level& _level) override;
		void ResetToDefaults() override;

	private:
		void ApplyTo(WorldPhysics& _physics) const;
	};

	/// <summary>
	/// Time of the game.
	/// </summary>
	class TimeSettings : public SettingsSection
	{
	public:
		float		timeScale		= 1.f;		// 0.5 = slow motion, 2 = twice as fast
		int			maxFrameRate	= 60;		// 0 = unlimited

		const char* Title() const override { return "Time"; }
		const char* Description() const override { return "Speed of the game and frame rate cap."; }
		void Apply() override;
		void ApplyToLevel(Level& _level) override;
		void ResetToDefaults() override;
	};

	/// <summary>
	/// Window.
	/// </summary>
	class WindowSettings : public SettingsSection
	{
	public:
		bool		verticalSync	= true;

		const char* Title() const override { return "Window"; }
		const char* Description() const override { return "Display options of the game window."; }
		void Apply() override;
		void ResetToDefaults() override;
	};

	/// <summary>
	/// Audio.
	/// </summary>
	class AudioSettings : public SettingsSection
	{
	public:
		float		globalVolume	= 100.f;	// 0 - 100

		const char* Title() const override { return "Audio"; }
		const char* Description() const override { return "Volume of everything that is played."; }
		void Apply() override;
		void ResetToDefaults() override;
	};

	/// <summary>
	/// Network: the defaults of Host() and Join(), the ones the "Host" and "Join" functions of event lists use.
	/// </summary>
	class NetworkSettings : public SettingsSection
	{
	public:
		int			port			= 7777;				// TCP and UDP
		std::string	address			= "127.0.0.1";		// the host that Join() goes to
		std::string	playerName		= "Player";
		int			maxClients		= 8;
		float		timeout			= 10.f;				// seconds of silence before a player is dropped

		const char* Title() const override { return "Network"; }
		const char* Description() const override { return "Hosting and joining a network game (see NetworkManager). Port, the host address to join, player name."; }
		void Apply() override;
		void ResetToDefaults() override;
	};

	/// <summary>
	/// The packaged game (Ship): the scene it starts on and how its window looks. The editor also opens the start scene when it starts;
	/// the window options only matter for the exe made by File > Package Game.
	/// </summary>
	class ShipSettings : public SettingsSection
	{
	public:
		std::string	startScene		= "MainMenu";			// name of a scene of Content/Scenes (without .ka7)
		std::string	gameName		= "KrampusEngine";		// title of the window
		int			windowMode		= 0;						// 0 borderless at the desktop size, 1 window, 2 fullscreen
		int			windowWidth		= 1280;						// for the window and the fullscreen modes
		int			windowHeight	= 720;
		bool		showSplash		= true;						// the Krampus logo at the start

		const char* Title() const override { return "Ship"; }
		const char* Description() const override { return "The packaged game: first scene, window title and mode, splash screen. Saved in the settings file, so the package carries them."; }
		void ResetToDefaults() override;
	};

	/// <summary>
	/// Every section of the engine, and their file.
	/// </summary>
	class EngineSettings
	{
		std::vector<SettingsSection*>	sections;

	public:
		LoggerSettings		logger;
		PhysicsSettings		physics;
		TimeSettings		time;
		WindowSettings		window;
		AudioSettings		audio;
		NetworkSettings		network;
		ShipSettings		ship;

	public:
		EngineSettings();

		/// <returns>The file the settings are stored in.</returns>
		static std::filesystem::path GetPath();

		const std::vector<SettingsSection*>& GetSections() const { return sections; }

		/// <summary>Gives the engine to every section.</summary>
		void Attach(Engine* _engine);

		/// <summary>Reads the file (a missing file keeps the defaults) without applying anything: the window needs the Ship values before it exists.</summary>
		void LoadFile();

		/// <summary>Reads the file (a missing file keeps the defaults) and applies the values.</summary>
		void Load();

		/// <returns>False if the file could not be written.</returns>
		bool Save() const;

		void ResetAllToDefaults();
		void Apply();
		void ApplyToLevel(Level& _level);

		/// <summary>
		/// Writes the properties of sections as "[Name]" then "property = value" lines (also used for the editor's own settings).
		/// </summary>
		static bool SaveSections(const std::filesystem::path& _path, const std::vector<SettingsSection*>& _sections);

		/// <summary>
		/// Reads such a file into sections. Unknown names are skipped. Returns false if the file does not exist.
		/// </summary>
		static bool LoadSections(const std::filesystem::path& _path, const std::vector<SettingsSection*>& _sections);
	};
}
