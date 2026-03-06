#pragma once
#include "Component.h"
#include "Graphics/Ressources/Animation.h"

namespace Krampus
{

	struct AnimationSM : public Animation
	{
	private:
		std::map<AnimationSM*, std::function<bool()>> links;

	public:
		AnimationSM(Engine* _engine, const std::string& _name, ShapeObject* _shape, const AnimationData& _data);
	
		void CreateLink(AnimationSM* _animation, std::function<bool()> _check);
		AnimationSM* GetNextAnimation();
	};

	class AnimationComponentSM : Component
	{

	};

}

