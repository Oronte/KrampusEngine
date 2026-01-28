#pragma once
#include "Utilities/System/Singleton.h"
#include "GameFramework/Level.h"

#define M_LEVEL Krampus::LevelManager::GetInstance()

namespace Krampus
{

	class LevelManager : public Singleton<LevelManager>
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
			currentLevel = std::make_unique<LevelType>();
			currentLevel->Load();
			return GetCurrentLevel<LevelType>();
		}

		LevelManager() = default;
	};

}