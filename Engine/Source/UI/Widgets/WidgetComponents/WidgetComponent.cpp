#include "WidgetComponent.h"
#include "UI/Widgets/Widget.h"
#include "GameFramework/Level.h"

class Krampus::Level* Krampus::WidgetComponent::GetLevel() const
{
    return owner->GetLevel();
}

Krampus::Engine* Krampus::WidgetComponent::GetWorld() const
{
    return GetLevel()->GetWorld();
}

Float Krampus::WidgetComponent::GetDeltaTime() const
{
    return owner->GetDeltaTime();
}

Krampus::WidgetComponent::WidgetComponent(Widget* _owner)
{
    name = NAME_OF(Component);
    owner = _owner;
}

Krampus::Timer* Krampus::WidgetComponent::CreateTimer(const std::function<void()>& _callback, const Float& _duration,
    const Bool& _isLoop, const Bool& _startRunning)
{
	return owner->CreateTimer(_callback, _duration, _isLoop, _startRunning);
}

void Krampus::WidgetComponent::Construct()
{
    SetActive(true);
}

void Krampus::WidgetComponent::Deconstruct()
{
    SetActive(false);
}

std::string Krampus::WidgetComponent::ToString() const
{
    if (!owner) return "WidgetComponent with no owner";
    return "WidgetComponent : owner = " + owner->ToString();
}

void Krampus::WidgetComponent::SetPosition(const FVector2& _newPosition)
{
    GetOwner()->SetPosition(_newPosition);
}

Krampus::FVector2 Krampus::WidgetComponent::GetPosition() const
{
	return GetOwner()->GetPosition();
}

void Krampus::WidgetComponent::SetRotation(const Angle& _newRotation)
{
	GetOwner()->SetRotation(_newRotation);
}

Krampus::Angle Krampus::WidgetComponent::GetRotation() const
{
	return GetOwner()->GetRotation();
}

void Krampus::WidgetComponent::SetScale(const FVector2& _newScale)
{
	GetOwner()->SetScale(_newScale);
}

Krampus::FVector2 Krampus::WidgetComponent::GetScale() const
{
	return GetOwner()->GetScale();
}
