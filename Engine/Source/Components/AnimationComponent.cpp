#include "AnimationComponent.h"
#include "SpriteComponent.h"
#include "Actors/Actor.h"

Krampus::AnimationComponent::AnimationComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(AnimationComponent);
}

void Krampus::AnimationComponent::AddAnimation(const String& _name, const AnimationData& _data)
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

std::string Krampus::AnimationComponent::ToString() const
{
	String _animationsName;
	for (auto& [_key, _anim] : animations)
		_animationsName += _key + ", ";
	return name + std::format(" -> Animations Count = {} : ", animations.size()) + _animationsName;
}
