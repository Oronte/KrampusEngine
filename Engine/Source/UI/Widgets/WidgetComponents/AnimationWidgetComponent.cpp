#include "AnimationWidgetComponent.h"
#include "ImageWidgetComponent.h"
#include "UI/Widgets/Widget.h"

Krampus::AnimationWidgetComponent::AnimationWidgetComponent(Widget* _owner)
    : WidgetComponent(_owner)
{
    name = NAME_OF(AnimationWidgetComponent);
}

void Krampus::AnimationWidgetComponent::AddAnimation(const String& _name, const AnimationData& _data)
{
    if (animations.contains(_name)) return;

    ImageWidgetComponent* _sprite = owner->GetComponent<ImageWidgetComponent>();
    if (!_sprite)
    {
        LOG_ERROR("AnimationWidgetComponent: \"" + _name + "\" needs an ImageWidgetComponent on the same widget");
        return;
    }

    animations[_name] = std::make_unique<Animation>(GetWorld(), _name, _sprite->GetShapeObject(), _data);

    if (!current) SetCurrentAnimation(_name);
}

void Krampus::AnimationWidgetComponent::Deconstruct()
{
    WidgetComponent::Deconstruct();

    StopAnimation();
    current = nullptr;
}

std::string Krampus::AnimationWidgetComponent::ToString() const
{
    String _animationsName;
    for (auto& [_key, _anim] : animations)
        _animationsName += _key + ", ";
    return name + std::format(" -> Animations Count = {} : ", animations.size()) + _animationsName;
}


void Krampus::AnimationWidgetComponent::ApplyClips()
{
	if (!owner->GetComponent<ImageWidgetComponent>()) return;

	ApplyAnimationClips(clips, startAnimation, appliedClips, animations, current, [this](const String& _name, const AnimationData& _data) { AddAnimation(_name, _data); }, "AnimationWidgetComponent");
	clipStarted = false;
}

void Krampus::AnimationWidgetComponent::Tick(const Float& _deltaTime)
{
    WidgetComponent::Tick(_deltaTime);

    if (clipStarted) return;
    clipStarted = true;

    if (animations.empty()) ApplyClips();
    if (playOnBeginPlay) StartAnimation();
}
