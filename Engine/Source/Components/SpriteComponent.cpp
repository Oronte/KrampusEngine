#include "SpriteComponent.h"
#include "Graphics/Window.h"
#include "Actors/Actor.h"
#include "GameFramework/Level.h"
#include "Managers/CameraManager.h"
#include "StainComponent.h"

Krampus::SpriteComponent::SpriteComponent(Actor* _owner, const CircleShapeData& _data)
	: Component(_owner)
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

Krampus::SpriteComponent::SpriteComponent(Actor* _owner, const RectangleShapeData& _data)
	: Component(_owner)
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

void Krampus::SpriteComponent::Construct()
{
	Component::Construct();

	shape->GetShape()->SetPosition(GetPosition());
	shape->GetShape()->SetRotation(GetRotation());
	shape->GetShape()->SetScale(GetScale());
	GetLevel()->GetCameraManager().AddToWorld(this);
}

void Krampus::SpriteComponent::Deconstruct()
{
	Component::Deconstruct();

	GetLevel()->GetCameraManager().RemoveToWorld(this);
}


void Krampus::SpriteComponent::Draw(Window& _window)
{
	if (!isActive) return;
	_window.Draw(*shape);
	if (stainLayer) stainLayer->DrawOver(_window, *shape->GetShape()->Get());	// right after: the next sprites cover the marks too
}

void Krampus::SpriteComponent::DrawDebug()
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

void Krampus::SpriteComponent::SetZOrder(const uint8_t& _zOrder)
{
	const uint8_t& _oldZOrder = GetZOrder();
	IDrawable::SetZOrder(_zOrder);
	GetLevel()->GetCameraManager().ChangeWorldZOrder(this, _oldZOrder);
}

void Krampus::SpriteComponent::SetZOrder(const ZOrder& _zOrder)
{
	const uint8_t& _oldZOrder = GetZOrder();
	IDrawable::SetZOrder(_zOrder);
	GetLevel()->GetCameraManager().ChangeWorldZOrder(this, _oldZOrder);
}

std::string Krampus::SpriteComponent::ToString() const
{
	if (GetShapeType() == ShapeType::Polygon) return name + " -> Polygon = " + shape->GetPolygonName();
	return name + " -> Size/Radius = " + (GetShapeType() == ShapeType::Rectangle ? GetSize().ToString() : GetRadius().ToString());
}

