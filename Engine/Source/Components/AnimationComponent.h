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
		inlin void SetCurrentAnimation(const std::string& _name)
		{
			if (!animations.contains(_name)) return;
			current = animations[_name].get();
		}
		inlin Animation* GetCurrentAnimation() const
		{
			return current;
		}
		inlin Animation* GetAnimation(const std::string _animationName) const
		{
			auto _iterator = animations.find(_animationName);
			if (_iterator == animations.end())
			{
				LOG_ERROR("There is no animation with the name \"" + _animationName + "\"");
				return nullptr;
			}
			return _iterator->second.get();
		}
		inlin void StartAnimation()
		{
			if (!current) return;
			current->Start();
		}
		inlin void StopAnimation()
		{
			if (!current) return;
			current->Stop();
		}
		inlin void ResumeAnimation()
		{
			if (!current) return;
			current->Resume();
		}
		inlin void PauseAnimation()
		{
			if (!current) return;
			current->Pause();
		}

		// The owner require a SpriteComponent
		AnimationComponent(Actor* _owner);

		void AddAnimation(const std::string& _name, const AnimationData& _data);
	};


}

