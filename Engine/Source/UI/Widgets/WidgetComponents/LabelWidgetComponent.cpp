#include "LabelWidgetComponent.h"
#include "UI/Widgets/Widget.h"
#include "Graphics/Window.h"
#include "GameFramework/Level.h"

Krampus::LabelWidgetComponent::LabelWidgetComponent(Widget *_owner, const TextData &_data)
    : WidgetComponent(_owner), text(_data)
{
    name = NAME_OF(LabelComponent);

    onOwnerMoveHandle = _owner->onMove.AddListener([this](FVector2 _newPos)
        {
            text.SetPosition(_newPos);
        });
    onOwnerRotateHandle = _owner->onRotate.AddListener([this](Angle _newRot)
        {
            text.SetRotation(_newRot);
        });
    onOwnerScaleHandle = _owner->onScale.AddListener([this](FVector2 _newScale)
        {
            text.SetScale(_newScale);
        });
    // Start where the owner is: its onMove / onRotate / onScale only fire when it changes afterwards.
    text.SetPosition(_owner->GetPosition());
    text.SetRotation(_owner->GetRotation());
    text.SetScale(_owner->GetScale());
}

void Krampus::LabelWidgetComponent::Draw(Window &_window)
{
    if (!isActive) return;
    _window.Draw(text);
}

void Krampus::LabelWidgetComponent::DrawDebug()
{
#ifdef DEBUG
    if (!IsDebugging()) return;
    { const FRect _bounds = GetGlobalBounds(); Debug::DrawDebugRect(GetLevel(), _bounds.GetPosition() + _bounds.GetSize() / 2.0f, _bounds.GetSize(), Angle(), Color::Green()); }
#endif
}

void Krampus::LabelWidgetComponent::Construct()
{
    WidgetComponent::Construct();

	GetLevel()->GetCameraManager().AddToScreen(this);
}

void Krampus::LabelWidgetComponent::Deconstruct()
{
    WidgetComponent::Deconstruct();

	GetLevel()->GetCameraManager().RemoveToScreen(this);
}

void Krampus::LabelWidgetComponent::SetZOrder(const uint8_t &_zOrder)
{
    const uint8_t& _oldZOrder = GetZOrder();
    IDrawable::SetZOrder(_zOrder);
    GetLevel()->GetCameraManager().ChangeScreenZOrder(this, _oldZOrder);
}

void Krampus::LabelWidgetComponent::SetZOrder(const ZOrder &_zOrder)
{
    const uint8_t& _oldZOrder = GetZOrder();
    IDrawable::SetZOrder(_zOrder);
    GetLevel()->GetCameraManager().ChangeScreenZOrder(this, _oldZOrder);
}

std::string Krampus::LabelWidgetComponent::ToString() const
{
	return name + " -> Label = \"" + text.GetString() + "\"";
}
