#pragma once
#include "GameFramework/Level.h"
#include "Core/KrampusObject.h"

namespace Krampus
{

	/// <summary>
	/// Manages level loading, unloading, transitions, and the currently active Level instance.
	/// </summary>
	class LevelManager : public KrampusObject
	{
		std::unique_ptr<Level>	currentLevel	=	nullptr;
		std::unique_ptr<Level>	pendingAddLevel =	nullptr;
		std::function<std::unique_ptr<Level>()>	levelFactory;	// builds a new instance of the last level given to SetLevel
		int						levelSerial		= 0;		// counts the levels loaded so far
		bool					silentChange	= false;	// the next level change is a technical one (undo): no fade, quiet logs

	public:
		/// <summary>
		/// Event broadcast right after a pending level has been loaded and become the current one.
		/// </summary>
		Event<>					onLevelChanged;

		/// <summary>
		/// Retrieves the currently loaded level, cast to the requested Level-derived class.
		/// </summary>
		/// <typeparam name="LevelType">Level derived class.</typeparam>
		/// <returns>Pointer to active LevelType, or nullptr if cast fails.</returns>
		template<typename LevelType = Level>
		inline LevelType* GetCurrentLevel() const
		{
			if (!currentLevel) return nullptr;	// no level yet (start, or between two levels): not an error

			LevelType* _level = Cast<LevelType>(currentLevel.get());
			if (!_level)
			{
				LOG_ERROR("You tried to get the current level with the wrong type");
				return nullptr;
			}
			return _level;
		}

		/// <summary>
		/// Number of levels loaded so far: changes each time a level is swapped in.
		/// </summary>
		inline int GetLevelSerial() const
		{
			return levelSerial;
		}

		/// <summary>
		/// Makes the next level change silent: no fade in, and its messages go to the log file only. Used by the editor to swap a scene in place (undo).
		/// </summary>
		inline void SetNextChangeSilent()
		{
			silentChange = true;
		}

		/// <returns>True while a silent level change is in progress.</returns>
		inline bool IsChangeSilent() const
		{
			return silentChange;
		}

		/// <summary>
		/// The level in progress, or nullptr when there is none (no error is logged, unlike GetCurrentLevel).
		/// </summary>
		inline Level* Peek() const
		{
			return currentLevel.get();
		}

		/// <summary>
		/// Stages a new level instance to be loaded on the next level change.
		/// </summary>
		/// <typeparam name="LevelType">Level derived class to instantiate.</typeparam>
		/// <returns>Pointer to newly staged level.</returns>
		template<typename LevelType = Level>
		inline LevelType* SetLevel()
		{
			levelFactory = [this]() -> std::unique_ptr<Level> { return std::make_unique<LevelType>(GetWorld()); };
			LOG(VerbosityType::Log, "LevelManager: next level will be " + std::string(typeid(LevelType).name()));
			pendingAddLevel = std::make_unique<LevelType>(GetWorld());
			return Cast<LevelType>(pendingAddLevel.get());
		}

		/// <summary>
		/// Stages a fresh instance of the last level given to SetLevel (restart).
		/// </summary>
		inline void ReloadLevel()
		{
			LOG(VerbosityType::Log, "LevelManager: reload requested");
			if (levelFactory)
				pendingAddLevel = levelFactory();
		}

		/// <summary>
		/// Executes the transition to the staged pending level, unloading the current level and loading the new one.
		/// </summary>
		/// <returns>True if a level transition occurred, false if no pending level was staged.</returns>
		inline Bool ChangeLevel()
		{
			if (!pendingAddLevel.get())
				return Bool::False();

			LOG(silentChange ? VerbosityType::Log : VerbosityType::Display, "LevelManager: switching level");

			if (currentLevel.get()) 
				currentLevel->Unload();

			currentLevel.reset();
			currentLevel = std::move(pendingAddLevel);
			pendingAddLevel = nullptr;

			++levelSerial;
			currentLevel->Load();
			onLevelChanged.Broadcast();
			silentChange = false;

			return Bool::True();
		}

		/// <summary>
		/// Constructs the LevelManager associated with the engine world.
		/// </summary>
		/// <param name="_engine">Pointer to Engine world.</param>
		LevelManager(Engine* _engine)
			: KrampusObject(_engine) { }
	};

}
