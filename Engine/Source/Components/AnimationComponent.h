#pragma once
#include "Component.h"
#include "Graphics/Ressources/Animation.h"


namespace Krampus
{

	class SpriteComponent;

	class AnimationComponent : public Component
	{
		Animation* current;
		std::unordered_map<std::string, std::unique_ptr<Animation>> animations;
		SpriteComponent* sprite;

	public:
		FORCEINLINE void SetCurrentAnimation(const std::string& _name)
		{
			if (!animations.contains(_name)) return;
			current = animations[_name].get();
		}
		//FORCEINLINE void SetCurrentAnimation(Animation* _animation)
		//{
		//	if (!_animation) return;
		//	current = _animation;
		//}
		FORCEINLINE void StartAnimation()
		{
			if (!current) return;

			//current->onAnimationEnded.AddListener([&]()
			//	{
			//		if (Animation* _animation = current->GetNextAnimation())
			//		{
			//			SetCurrentAnimation(_animation);
			//			StartAnimation();
			//		}
			//	});
			current->Start();
		}
		FORCEINLINE void StopAnimation()
		{
			if (!current) return;
			current->Stop();
		}

		AnimationComponent(Actor* _owner);

		void AddAnimation(const std::string& _name, const AnimationData& _data);
	};


}

