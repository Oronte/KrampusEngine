#pragma once
#include "Reflection/Reflection.h"

namespace Krampus
{

	/// <summary>
	/// Saves and loads the actors of a level as a .ka7 scene file (the SaveNode binary format).
	/// Only actors whose class is registered with KACTOR are saved, with their transform, hierarchy and KPROPERTY values
	/// (also for their components). The level's default camera is not saved: every level creates its own.
	/// </summary>
	class Scene
	{
	public:
		static constexpr const char*	EXTENSION	= ".ka7";

		/// <returns>False if the file could not be written.</returns>
		static bool Save(Level& _level, const std::filesystem::path& _path);

		/// <summary>
		/// Spawns the actors stored in the file into the level.
		/// </summary>
		/// <returns>False if the file could not be read.</returns>
		static bool Load(Level& _level, const std::filesystem::path& _path);

		/// <summary>
		/// Turns "MainMenu" into Content/Scenes/MainMenu.ka7. A name with a folder or an extension is kept as it is.
		/// </summary>
		static std::filesystem::path Resolve(const std::string& _nameOrPath);

		/// <summary>
		/// Makes a scene the current level (swapped at the start of the next frame, safe to call from game code).
		/// </summary>
		static void Open(Engine& _engine, const std::string& _nameOrPath);
	};

	/// <summary>
	/// A level made only of a scene file. The editor uses it to open and create scenes.
	/// </summary>
	class SceneLevel : public Level
	{
		std::filesystem::path	scenePath;

	public:
		SceneLevel(Engine* _engine)
			: Level(_engine) { }

		/// <param name="_path">Scene to load when the level starts. Empty = empty scene.</param>
		void SetScenePath(const std::filesystem::path& _path)
		{
			scenePath = _path;
		}

		/// <returns>The scene file this level was made from (empty for a new scene).</returns>
		inline const std::filesystem::path& GetScenePath() const
		{
			return scenePath;
		}

	protected:
		void InitLevel() override;
	};
}
