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


void Krampus::AnimationComponent::ApplyClip()
{
	if (clip.frameCount <= 0 || !owner->GetComponent<SpriteComponent>()) return;

	if (const auto _old = animations.find(appliedClip); _old != animations.end())
	{
		if (current == _old->second.get())
		{
			current->Stop();
			current = nullptr;
		}
		animations.erase(_old);
	}

	if (animations.contains(clip.name))
	{
		LOG_WARNING("AnimationComponent: an animation named \"" + clip.name + "\" already exists, the clip was not applied");
		return;
	}

	AddAnimation(clip.name, clip.ToData());
	appliedClip = clip.name;
	SetCurrentAnimation(clip.name);
	clipStarted = false;
}

void Krampus::AnimationComponent::Tick(const Float& _deltaTime)
{
	Component::Tick(_deltaTime);

	// The first frame that runs (not the level load): in the editor the level is frozen until Play.
	if (clipStarted) return;
	clipStarted = true;

	if (animations.empty()) ApplyClip();
	if (playOnBeginPlay) StartAnimation();
}
