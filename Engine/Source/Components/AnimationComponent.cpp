#include "AnimationComponent.h"
#include "SpriteComponent.h"
#include "Actors/Actor.h"

Krampus::AnimationComponent::AnimationComponent(Actor* _owner)
	: Component(_owner)
{
	sprite = owner->GetComponent<SpriteComponent>();
	if (!sprite)
		LOG(VerbosityType::Warning, "You need to have a sprite component for this Component");
}

void Krampus::AnimationComponent::AddAnimation(const std::string& _name, const AnimationData& _data)
{
	if (animations.contains(_name)) return;

	animations[_name] = std::make_unique<Animation>(_name, sprite->GetShapeObject(), _data);

	if (!current) SetCurrentAnimation(_name);
}
