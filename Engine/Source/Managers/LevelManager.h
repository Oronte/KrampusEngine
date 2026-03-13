#pragma once
#include "GameFramework/Level.h"
#include "Core/KrampusObject.h"

namespace Krampus
{

	class LevelManager : public KrampusObject
	{
		std::unique_ptr<Level>	currentLevel	=	nullptr;
		std::unique_ptr<Level>	pendingAddLevel =	nullptr;

	public:
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

		template<typename LevelType = Level>
		inline LevelType* SetLevel()
		{
			pendingAddLevel = std::make_unique<LevelType>(GetWorld());
			return Cast<LevelType>(pendingAddLevel.get());
		}

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

		LevelManager(Engine* _engine)
			: KrampusObject(_engine) { }
	};

}