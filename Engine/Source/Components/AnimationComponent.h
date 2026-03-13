#pragma once
#include "Component.h"
#include "Graphics/Ressources/Animation.h"


namespace Krampus
{

	class AnimationComponent : public Component
	{
		Animation*														current		= nullptr;
		std::unordered_map<String, std::unique_ptr<Animation>>			animations;

	public:
		inline void SetCurrentAnimation(const String& _name)
		{
			if (!animations.contains(_name)) return;

			if (current) current->Stop();
			current = animations[_name].get();
		}
		inline Animation* GetCurrentAnimation() const
		{
			return current;
		}
		inline Animation* GetAnimation(const String& _animationName) const
		{
			auto _iterator = animations.find(_animationName);
			if (_iterator == animations.end())
			{
				LOG_ERROR("There is no animation with the name \"" + _animationName + "\"");
				return nullptr;
			}
			return _iterator->second.get();
		}
		inline void StartAnimation()
		{
			if (!current) return;
			current->Start();
		}
		inline void StopAnimation()
		{
			if (!current) return;
			current->Stop();
		}
		inline void ResumeAnimation()
		{
			if (!current) return;
			current->Resume();
		}
		inline void PauseAnimation()
		{
			if (!current) return;
			current->Pause();
		}

		// The owner require a SpriteComponent
		AnimationComponent(Actor* _owner);

		virtual void AddAnimation(const String& _name, const AnimationData& _data);

		virtual void Deconstruct() override;

		virtual std::string ToString() const override;
	};


}

