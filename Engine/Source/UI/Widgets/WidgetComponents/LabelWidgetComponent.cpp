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
}

void Krampus::LabelWidgetComponent::Draw(Window &_window)
{
    if (!isActive) return;
    _window.Draw(text);
    DrawDebug();
}

void Krampus::LabelWidgetComponent::DrawDebug()
{
#ifdef DEBUG
    if (!IsDebugging()) return;
    Debug::DrawDebugRect(GetLevel(), GetPosition(), GetLocalBounds().GetSize(), GetRotation(), Color::Green());
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
