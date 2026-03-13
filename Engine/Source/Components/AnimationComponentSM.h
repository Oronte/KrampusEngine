#pragma once
#include "Component.h"
#include "Graphics/Ressources/Animation.h"

namespace Krampus
{

	class AnimationComponentSM;

	struct AnimationSM : public Animation
	{
	private:
		AnimationComponentSM*							component	= nullptr;
		std::map<AnimationSM*, std::function<Bool()>>	links;

	public:
		AnimationSM(Engine* _engine, AnimationComponentSM* _component, const String& _name, ShapeObject* _shape, const AnimationData& _data);
	
		Bool CreateLink(AnimationSM* _animation, std::function<Bool()> _check);
		AnimationSM* GetNextAnimation();
	};

	class AnimationComponentSM : public Component
	{
		AnimationSM*												current		= nullptr;
		std::unordered_map<String, std::unique_ptr<AnimationSM>>	animations;

	public:
		inline AnimationSM* GetCurrentAnimation() const
		{
			return current;
		}
		inline AnimationSM* GetAnimation(const String& _animationName) const
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
		AnimationComponentSM(Actor* _owner);

		AnimationSM* AddAnimation(const String& _name, const AnimationData& _data);
		AnimationSM* AddAnimation(const String& _name, const AnimationData& _data, std::vector<std::pair<AnimationSM*, std::function<Bool()>>> _links);

		virtual void Tick(const Float& _deltaTime) override;

		virtual std::string ToString() const override;

		friend struct AnimationSM;
	};

}

