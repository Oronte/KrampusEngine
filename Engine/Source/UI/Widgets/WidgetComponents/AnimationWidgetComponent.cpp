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
        LOG(VerbosityType::Error, "You need to have a sprite component for this Component");

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


void Krampus::AnimationWidgetComponent::ApplyClip()
{
    if (clip.frameCount <= 0 || !owner->GetComponent<ImageWidgetComponent>()) return;

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
        LOG_WARNING("AnimationWidgetComponent: an animation named \"" + clip.name + "\" already exists, the clip was not applied");
        return;
    }

    AddAnimation(clip.name, clip.ToData());
    appliedClip = clip.name;
    SetCurrentAnimation(clip.name);
    clipStarted = false;
}

void Krampus::AnimationWidgetComponent::Tick(const Float& _deltaTime)
{
    WidgetComponent::Tick(_deltaTime);

    if (clipStarted) return;
    clipStarted = true;

    if (animations.empty()) ApplyClip();
    if (playOnBeginPlay) StartAnimation();
}
