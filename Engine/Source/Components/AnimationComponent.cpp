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
	{
		LOG_ERROR("AnimationComponent: \"" + _name + "\" needs a SpriteComponent on the same actor");
		return;
	}

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


void Krampus::AnimationComponent::ApplyClips()
{
	if (!owner->GetComponent<SpriteComponent>()) return;

	ApplyAnimationClips(clips, startAnimation, appliedClips, animations, current, [this](const String& _name, const AnimationData& _data) { AddAnimation(_name, _data); }, "AnimationComponent");
	clipStarted = false;
}

void Krampus::AnimationComponent::Tick(const Float& _deltaTime)
{
	Component::Tick(_deltaTime);

	// The first frame that runs (not the level load): in the editor the level is frozen until Play.
	if (clipStarted) return;
	clipStarted = true;

	if (animations.empty()) ApplyClips();
	if (playOnBeginPlay) StartAnimation();
}
