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

	SetTransform(Transform(GetPosition(), GetRotation(), GetScale()));
    GetLevel()->GetCameraManager().AddToScreen(this);
}

void Krampus::ImageWidgetComponent::Deconstruct()
{
    WidgetComponent::Deconstruct();

    GetLevel()->GetCameraManager().RemoveToScreen(this);
}

void Krampus::ImageWidgetComponent::Draw(Window &_window)
{
    if (!isActive) return;
    _window.Draw(*shape);
}

void Krampus::ImageWidgetComponent::DrawDebug()
{
#ifdef DEBUG
    if (!IsDebugging()) return;

    switch (shape->GetShapeType())
    {
        case ShapeType::Circle:
            Debug::DrawDebugCircle(GetLevel(), GetPosition(), shape->GetSizeData().radius * std::max(GetScale().x, GetScale().y), 15, Color::Green());
            break;

        case ShapeType::Rectangle:
            Debug::DrawDebugRect(GetLevel(), GetPosition(), FVector2(shape->GetSizeData().size.x * GetScale().x, shape->GetSizeData().size.y * GetScale().y), GetRotation(), Color::Green());
            break;

        case ShapeType::Polygon:
            Debug::DrawDebugPolygon(GetLevel(), Polygon2D::Transform(shape->GetOutline().points, GetPosition(), GetRotation(), GetScale()), Color::Green());
            break;
    };
#endif
}

void Krampus::ImageWidgetComponent::SetZOrder(const uint8_t &_zOrder)
{
    const uint8_t& _oldZOrder = GetZOrder();
    IDrawable::SetZOrder(_zOrder);
    GetLevel()->GetCameraManager().ChangeScreenZOrder(this, _oldZOrder);
}

void Krampus::ImageWidgetComponent::SetZOrder(const ZOrder &_zOrder)
{
    const uint8_t& _oldZOrder = GetZOrder();
    IDrawable::SetZOrder(_zOrder);
    GetLevel()->GetCameraManager().ChangeScreenZOrder(this, _oldZOrder);
}

std::string Krampus::ImageWidgetComponent::ToString() const
{
	if (GetShapeType() == ShapeType::Polygon) return name + " -> Polygon = " + shape->GetPolygonName();
	return name + " -> Size/Radius = " + (GetShapeType() == ShapeType::Rectangle ? GetSize().ToString() : GetRadius().ToString());
}
