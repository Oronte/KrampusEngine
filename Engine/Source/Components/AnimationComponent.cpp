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


void Krampus::AnimationComponent::ApplyClips()
{
	if (!owner->GetComponent<SpriteComponent>()) return;

	// Take away what the previous clips made.
	for (const String& _old : appliedClips)
	{
		const auto _found = animations.find(_old);
		if (_found == animations.end()) continue;
		if (current == _found->second.get())
		{
			current->Stop();
			current = nullptr;
		}
		animations.erase(_found);
	}
	appliedClips.clear();

	for (const AnimationClip& _clip : clips)
	{
		if (_clip.frameCount <= 0) continue;
		if (animations.contains(_clip.name))
		{
			LOG_WARNING("AnimationComponent: an animation named \"" + _clip.name + "\" already exists, the clip was not applied");
			continue;
		}
		AddAnimation(_clip.name, _clip.ToData());
		appliedClips.push_back(_clip.name);
	}

	// The animation the level starts with: startAnimation, else the first clip.
	const String& _first = appliedClips.empty() ? startAnimation : (std::find(appliedClips.begin(), appliedClips.end(), startAnimation) != appliedClips.end() ? startAnimation : appliedClips.front());
	const auto _start = animations.find(_first);
	if (_start != animations.end())
	{
		if (current && current != _start->second.get()) current->Stop();
		current = _start->second.get();
	}
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
