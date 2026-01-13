#pragma once
#include "Component.h"
#include "Graphics/Ressources/Animation.h"


namespace Krampus
{

	class AnimationComponent : public Component
	{
		Animation* current;
		std::unordered_map<std::string, std::unique_ptr<Animation>> animations;

	public:
		INLINE void SetCurrentAnimation(const std::string& _name)
		{
			if (!animations.contains(_name)) return;
			current = animations[_name].get();
		}
		INLINE Animation* GetCurrentAnimation() const
		{
			return current;
		}
		INLINE Animation* GetAnimation(const std::string _animationName) const
		{
			auto _iterator = animations.find(_animationName);
			if (_iterator == animations.end())
			{
				LOG_ERROR("There is no animation with the name \"" + _animationName + "\"");
				return nullptr;
			}
			return _iterator->second.get();
		}
		INLINE void StartAnimation()
		{
			if (!current) return;
			current->Start();
		}
		INLINE void StopAnimation()
		{
			if (!current) return;
			current->Stop();
		}
		INLINE void ResumeAnimation()
		{
			if (!current) return;
			current->Resume();
		}
		INLINE void PauseAnimation()
		{
			if (!current) return;
			current->Pause();
		}

		// The owner require a SpriteComponent
		AnimationComponent(Actor* _owner);

		void AddAnimation(const std::string& _name, const AnimationData& _data);
	};


}

