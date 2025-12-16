#pragma once
#include "Utilities/System/Singleton.h"
#include "GameFramework/Level.h"

#define M_LEVEL Krampus::LevelManager::GetInstance()

namespace Krampus
{

	class LevelManager : public Singleton<LevelManager>
	{
		std::unordered_map<std::string, Level*> levels;
		Level* currentLevel;

	public:
		INLINE Level* GetCurrentLevel() const
		{
			return currentLevel;
		}
		INLINE void SetLevel(Level* _level)
		{
			if (_level == currentLevel) return;

			if (currentLevel) currentLevel->Unload();
			currentLevel = _level;
			if (currentLevel) currentLevel->Load();
			if (_level && !levels.contains(_level->GetName())) RegisterLevel(_level);
		}
		INLINE void SetLevel(const std::string& _name)
		{
			if (!levels.contains(_name))
				LOG(VerbosityType::Fatal, "Level \"" + _name + "\" not found !");

			SetLevel(levels[_name]);
		}
		INLINE void RegisterLevel(Level* _level)
		{
			if (!_level)
			{
				LOG_WARNING("You try to register a nullptr level");
				return;
			}
			if (levels.contains(_level->GetName()))
			{
				LOG_WARNING("You try to register a level with the same name than a other level");
				return;
			}
			levels.emplace(_level->GetName(), _level);
		}
		INLINE void UnregisterLevel(const std::string& _name)
		{
			levels.erase(_name);
		}
		INLINE void Destroy()
		{
			levels.clear();
		}


		LevelManager() = default;
	};

}