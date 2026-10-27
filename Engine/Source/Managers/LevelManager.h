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

	public:
		/// <summary>
		/// Retrieves the currently loaded level, cast to the requested Level-derived class.
		/// </summary>
		/// <typeparam name="LevelType">Level derived class.</typeparam>
		/// <returns>Pointer to active LevelType, or nullptr if cast fails.</returns>
		template<typename LevelType = Level>
		inline LevelType* GetCurrentLevel() const
		{
			LevelType* _level = Cast<LevelType>(currentLevel.get());
			if (!_level)
			{
				LOG_ERROR("You tried to get the current level with the wrong type");
				return nullptr;
			}
			return _level;
		}

		/// <summary>
		/// Stages a new level instance to be loaded on the next level change.
		/// </summary>
		/// <typeparam name="LevelType">Level derived class to instantiate.</typeparam>
		/// <returns>Pointer to newly staged level.</returns>
		template<typename LevelType = Level>
		inline LevelType* SetLevel()
		{
			pendingAddLevel = std::make_unique<LevelType>(GetWorld());
			return Cast<LevelType>(pendingAddLevel.get());
		}

		/// <summary>
		/// Executes the transition to the staged pending level, unloading the current level and loading the new one.
		/// </summary>
		/// <returns>True if a level transition occurred, false if no pending level was staged.</returns>
		inline Bool ChangeLevel()
		{
			if (!pendingAddLevel.get())
				return Bool::False();

			if (currentLevel.get()) 
				currentLevel->Unload();

			currentLevel.reset();
			currentLevel = std::move(pendingAddLevel);
			pendingAddLevel = nullptr;

			currentLevel->Load();

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
