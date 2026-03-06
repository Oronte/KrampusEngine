#include "AnimationComponentSM.h"

Krampus::AnimationSM::AnimationSM(Engine* _engine, const std::string& _name, ShapeObject* _shape, const AnimationData& _data)
	: Animation(_engine, _name, _shape, _data)
{
}

void Krampus::AnimationSM::CreateLink(AnimationSM* _animation, std::function<bool()> _check)
{
	if (links.contains(_animation))
	{
		LOG_WARNING("You already have a link to this animation !");
		return;
	}

	links[_animation] = _check;
}

Krampus::AnimationSM* Krampus::AnimationSM::GetNextAnimation()
{
	for (const std::pair<AnimationSM*, std::function<bool()>>& _pair : links)
	{
		AnimationSM* _anim = _pair.first;
		std::function<bool()> _check = _pair.second;
		if (_check())
		{
			AnimationSM* _lastAnim = _anim->GetNextAnimation();
			return _lastAnim ? _lastAnim : _anim;
		}
	}

	return nullptr;
}
