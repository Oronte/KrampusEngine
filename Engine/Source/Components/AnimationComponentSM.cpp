#include "AnimationComponentSM.h"
#include "Managers/TextureManager.h"
#include "SpriteComponent.h"
#include "Actors/Actor.h"


Krampus::AnimationSM::AnimationSM(Engine* _engine, AnimationComponentSM* _component, const String& _name, ShapeObject* _shape, const AnimationData& _data)
	: Animation(_engine, _name, _shape, _data)
{
	component = _component;
}

Bool Krampus::AnimationSM::CreateLink(AnimationSM* _animation, std::function<Bool()> _check)
{
	if (links.contains(_animation))
	{
		LOG_WARNING("You already have a link to this animation !");
		return false;
	}

	links[_animation] = _check;
	return true;
}

Krampus::AnimationSM* Krampus::AnimationSM::GetNextAnimation()
{
	for (const std::pair<AnimationSM*, std::function<Bool()>>& _pair : links)
	{
		AnimationSM* _anim = _pair.first;
		std::function<Bool()> _check = _pair.second;
		if (_check())
		{
			AnimationSM* _lastAnim = _anim->GetNextAnimation();
			return _lastAnim ? _lastAnim : _anim;
		}
	}

	return nullptr;
}

Krampus::AnimationComponentSM::AnimationComponentSM(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(AnimationComponentSM);
}

Krampus::AnimationSM* Krampus::AnimationComponentSM::AddAnimation(const String& _name, const AnimationData& _data)
{
	if (animations.contains(_name))
	{
		LOG_ERROR("Can't add a animation with the same name than a other");
		return nullptr;
	}

	SpriteComponent* _sprite = owner->GetComponent<SpriteComponent>();
	if (!_sprite)
		LOG(VerbosityType::Error, "You need to have a sprite component for this Component");

	animations[_name] = std::make_unique<AnimationSM>(GetWorld(), this, _name, _sprite->GetShapeObject(), _data);

	AnimationSM* _anim = animations[_name].get();
	if (!current) current = _anim;
	return _anim;
}

Krampus::AnimationSM* Krampus::AnimationComponentSM::AddAnimation(const String& _name, const AnimationData& _data, std::vector<std::pair<AnimationSM*, std::function<Bool()>>> _links)
{
	if (animations.contains(_name))
	{
		LOG_ERROR("Can't add a animation with the same name than a other");
		return nullptr;
	}

	SpriteComponent* _sprite = owner->GetComponent<SpriteComponent>();
	if (!_sprite)
		LOG(VerbosityType::Error, "You need to have a sprite component for this Component");

	animations[_name] = std::make_unique<AnimationSM>(GetWorld(), this, _name, _sprite->GetShapeObject(), _data);

	AnimationSM* _anim = animations[_name].get();
	if (!current) current = _anim;

	for (const std::pair<AnimationSM*, std::function<Bool()>>& _pair : _links)
		_anim->CreateLink(_pair.first, _pair.second);

	return _anim;
}

void Krampus::AnimationComponentSM::Tick(const Float& _deltaTime)
{
	Component::Tick(_deltaTime);

	if (!current) return;
	AnimationSM* _nextAnim = current->GetNextAnimation();
	if (!_nextAnim) return;
	current->Stop();
	current = _nextAnim;
	current->Start();
}

std::string Krampus::AnimationComponentSM::ToString() const
{
	String _animationsName;
	for (auto& [_key, _anim] : animations)
		_animationsName += _key + ", ";
	return name + std::format(" -> Animations Count = {} : ", animations.size()) + _animationsName;
}