#include "ImageWidgetComponent.h"
#include "Graphics/Window.h"
#include "UI/Widgets/Widget.h"
#include "GameFramework/Level.h"
#include "Managers/CameraManager.h"

Krampus::ImageWidgetComponent::ImageWidgetComponent(Widget *_owner, const CircleShapeData &_data)
    : WidgetComponent(_owner)
{
    name = NAME_OF(SpriteComponent);
    shape = std::make_unique<ShapeObject>(_data);
    SetZOrder(ZOrder::Actors);

    onOwnerMoveHandle = _owner->onMove.AddListener([this](FVector2 _newPos)
        {
            shape->GetShape()->SetPosition(_newPos);
        });
    onOwnerRotateHandle = _owner->onRotate.AddListener([this](Angle _newRot)
        {
            shape->GetShape()->SetRotation(_newRot);
        });
    onOwnerScaleHandle = _owner->onScale.AddListener([this](FVector2 _newScale)
        {
            shape->GetShape()->SetScale(_newScale);
        });
}

Krampus::ImageWidgetComponent::ImageWidgetComponent(Widget *_owner, const RectangleShapeData &_data)
    : WidgetComponent(_owner)
{
    name = NAME_OF(SpriteComponent);
    shape = std::make_unique<ShapeObject>(_data);
    SetZOrder(ZOrder::Actors);

    onOwnerMoveHandle = _owner->onMove.AddListener([this](FVector2 _newPos)
        {
            shape->GetShape()->SetPosition(_newPos);
        });
    onOwnerRotateHandle = _owner->onRotate.AddListener([this](Angle _newRot)
        {
            shape->GetShape()->SetRotation(_newRot);
        });
    onOwnerScaleHandle = _owner->onScale.AddListener([this](FVector2 _newScale)
        {
            shape->GetShape()->SetScale(_newScale);
        });
}

void Krampus::ImageWidgetComponent::Construct()
{
    WidgetComponent::Construct();

    GetLevel()->GetCameraManagerRef().AddToScreen(this);
}

void Krampus::ImageWidgetComponent::Deconstruct()
{
    WidgetComponent::Deconstruct();

    GetLevel()->GetCameraManagerRef().RemoveToScreen(this);
}

void Krampus::ImageWidgetComponent::Draw(Window &_window)
{
    if (!isActive) return;
    _window.Draw(*shape);
    DrawDebug();
}

void Krampus::ImageWidgetComponent::DrawDebug()
{
#ifdef DEBUG
    if (!IsDebugging()) return;

    switch (shape->GetShapeType())
    {
        case ShapeType::Circle:
            Debug::DrawDebugCircle(GetLevel(), GetPosition(), shape->GetSizeData().radius, 15, Color::Green());
            break;

        case ShapeType::Rectangle:
            Debug::DrawDebugRect(GetLevel(), GetPosition(), shape->GetSizeData().size, GetRotation(), Color::Green());
            break;
    };
#endif
}

void Krampus::ImageWidgetComponent::SetZOrder(const uint8_t &_zOrder)
{
    const uint8_t& _oldZOrder = GetZOrder();
    IDrawable::SetZOrder(_zOrder);
    GetLevel()->GetCameraManagerRef().ChangeScreenZOrder(this, _oldZOrder);
}

void Krampus::ImageWidgetComponent::SetZOrder(const ZOrder &_zOrder)
{
    const uint8_t& _oldZOrder = GetZOrder();
    IDrawable::SetZOrder(_zOrder);
    GetLevel()->GetCameraManagerRef().ChangeScreenZOrder(this, _oldZOrder);
}

std::string Krampus::ImageWidgetComponent::ToString() const
{
	return name + " -> Size/Radius = " + (GetShapeType() == ShapeType::Rectangle ? GetSize().ToString() : GetRadius().ToString());
}
