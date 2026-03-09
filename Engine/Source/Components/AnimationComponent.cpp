#include "AnimationComponent.h"
#include "SpriteComponent.h"
#include "Actors/Actor.h"
#include "AnimationComponentSM.h"

Krampus::AnimationComponent::AnimationComponent(Actor* _owner)
	: Component(_owner)
{  }

void Krampus::AnimationComponent::AddAnimation(const std::string& _name, const AnimationData& _data)
{
	if (animations.contains(_name)) return;

	SpriteComponent* _sprite = owner->GetComponent<SpriteComponent>();
	if (!_sprite)
		LOG(VerbosityType::Error, "You need to have a sprite component for this Component");

	animations[_name] = std::make_unique<Animation>(GetWorld(), _name, _sprite->GetShapeObject(), _data);

	if (!current) SetCurrentAnimation(_name);
}

void Krampus::AnimationComponent::Deconstruct()
{
	Component::Deconstruct();

	StopAnimation();
	current = nullptr;
}
