#pragma once
#include "GameFramework/Level.h"
#include "Core/KrampusObject.h"

namespace Krampus
{

	class LevelManager : public KrampusObject
	{
		std::unique_ptr<Level> currentLevel = nullptr;

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
			if (currentLevel.get()) currentLevel->Unload();
			currentLevel = std::make_unique<LevelType>(GetWorld());
			currentLevel->Load();
			return GetCurrentLevel<LevelType>();
		}

		LevelManager(Engine* _engine)
			: KrampusObject(_engine) { }
	};

}