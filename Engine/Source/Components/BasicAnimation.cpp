#include "BasicAnimation.h"
#include "SpriteComponent.h"
#include "Actors/Actor.h"
#include "UI/Widgets/Widget.h"
#include "UI/Widgets/WidgetComponents/ImageWidgetComponent.h"

template <typename Base, typename Image>
void Krampus::BasicAnimation<Base, Image>::AddAnimation(const String& _name, const AnimationData& _data)
{
	if (animations.contains(_name)) return;

	Image* _image = this->owner->template GetComponent<Image>();
	if (!_image)
	{
		LOG_ERROR(this->name + ": \"" + _name + "\" needs an image component (SpriteComponent / ImageWidgetComponent) on the same owner");
		return;
	}

	animations[_name] = std::make_unique<Animation>(this->GetWorld(), _name, _image->GetShapeObject(), _data);
	if (!current) SetCurrentAnimation(_name);
}

template <typename Base, typename Image>
void Krampus::BasicAnimation<Base, Image>::SetCurrentAnimation(const String& _name)
{
	const auto _found = animations.find(_name);
	if (_found == animations.end()) return;
	if (current) current->Stop();
	current = _found->second.get();
	current->Update();
}

template <typename Base, typename Image>
Krampus::Animation* Krampus::BasicAnimation<Base, Image>::GetAnimation(const String& _name) const
{
	const auto _found = animations.find(_name);
	if (_found != animations.end()) return _found->second.get();
	LOG_ERROR("There is no animation with the name \"" + _name + "\"");
	return nullptr;
}

template <typename Base, typename Image>
void Krampus::BasicAnimation<Base, Image>::ApplyClips()
{
	if (!this->owner->template GetComponent<Image>()) return;

	ApplyAnimationClips(clips, startAnimation, appliedClips, animations, current, [this](const String& _name, const AnimationData& _data) { AddAnimation(_name, _data); }, this->name.c_str());
	clipStarted = false;
}

template <typename Base, typename Image>
void Krampus::BasicAnimation<Base, Image>::Tick(const Float& _deltaTime)
{
	Base::Tick(_deltaTime);

	// The first frame that runs (not the level load): in the editor the level is frozen until Play.
	if (clipStarted) return;
	clipStarted = true;

	if (animations.empty()) ApplyClips();
	if (playOnBeginPlay) StartAnimation();
}

template <typename Base, typename Image>
void Krampus::BasicAnimation<Base, Image>::Deconstruct()
{
	Base::Deconstruct();
	StopAnimation();
	current = nullptr;
}

template <typename Base, typename Image>
std::string Krampus::BasicAnimation<Base, Image>::ToString() const
{
	String _names;
	for (const auto& [_name, _animation] : animations)
		_names += _name + ", ";
	return this->name + std::format(" -> Animations Count = {} : ", animations.size()) + _names;
}

template class Krampus::BasicAnimation<Krampus::Component, Krampus::SpriteComponent>;
template class Krampus::BasicAnimation<Krampus::WidgetComponent, Krampus::ImageWidgetComponent>;
