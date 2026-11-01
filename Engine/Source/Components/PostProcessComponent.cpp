#include "PostProcessComponent.h"
#include "Actors/Actor.h"
#include <algorithm>

using namespace Krampus;

namespace
{
	std::vector<PostProcessComponent*>& All()
	{
		static std::vector<PostProcessComponent*> _all;
		return _all;
	}
}

PostProcessComponent::PostProcessComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(PostProcessComponent);
}

void PostProcessComponent::Construct()
{
	Component::Construct();
	if (std::find(All().begin(), All().end(), this) == All().end()) All().push_back(this);
}

void PostProcessComponent::Deconstruct()
{
	Component::Deconstruct();
	std::erase(All(), this);
}

std::vector<PostProcessComponent*> PostProcessComponent::EffectsOf(const Level* _level, const bool& _editing)
{
	std::vector<PostProcessComponent*> _effects;
	for (PostProcessComponent* _effect : All())
		if (_effect->GetLevel() == _level && _effect->IsActive() && _effect->GetOwner()->IsActive() && _effect->material.HasShader() && (!_editing || _effect->applyInEditor))
			_effects.push_back(_effect);
	std::stable_sort(_effects.begin(), _effects.end(), [](const PostProcessComponent* _a, const PostProcessComponent* _b) { return _a->priority < _b->priority; });
	return _effects;
}
