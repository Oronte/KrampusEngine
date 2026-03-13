#include "SpriteComponent.h"
#include "Graphics/Window.h"
#include "Actors/Actor.h"
#include "GameFramework/Level.h"
#include "Managers/CameraManager.h"

Krampus::SpriteComponent::SpriteComponent(Actor* _owner, const CircleShapeData& _data)
	: Component(_owner)
{
	name = NAME_OF(SpriteComponent);
	shape = std::make_unique<ShapeObject>(_data);
	SetZOrder(ZOrder::Actors);
}

Krampus::SpriteComponent::SpriteComponent(Actor* _owner, const RectangleShapeData& _data)
	: Component(_owner)
{
	name = NAME_OF(SpriteComponent);
	shape = std::make_unique<ShapeObject>(_data);
	SetZOrder(ZOrder::Actors);
}

void Krampus::SpriteComponent::Construct()
{
	Component::Construct();

	level->GetCameraManagerRef().AddToWindow(this);
}

void Krampus::SpriteComponent::Deconstruct()
{
	Component::Deconstruct();

	level->GetCameraManagerRef().RemoveToWindow(this);
}

void Krampus::SpriteComponent::Tick(const Float& _deltaTime)
{
	Component::Tick(_deltaTime);

	shape->GetShape()->SetTransform(transform);
}


void Krampus::SpriteComponent::Draw(Window& _window)
{
	if (!isActive) return;
	_window.Draw(*shape);
	DrawDebug();
}

void Krampus::SpriteComponent::DrawDebug()
{
#ifdef DEBUG
	if (!IsDebugging()) return;

	switch (shape->GetShapeType())
	{
	case ShapeType::Circle:
		Debug::DrawDebugCircle(level, transform.position, shape->GetSizeData().radius, 15, Color::Green());
		break;

	case ShapeType::Rectangle:
		Debug::DrawDebugRect(level, transform.position, shape->GetSizeData().size, transform.rotation, Color::Green());
		break;
	};
#endif
}

void Krampus::SpriteComponent::SetZOrder(const uint8_t& _zOrder)
{
	const uint8_t& _oldZOrder = GetZOrder();
	IDrawable::SetZOrder(_zOrder);
	level->GetCameraManagerRef().ChangeZOrder(this, _oldZOrder);
}

void Krampus::SpriteComponent::SetZOrder(const ZOrder& _zOrder)
{
	const uint8_t& _oldZOrder = GetZOrder();
	IDrawable::SetZOrder(_zOrder);
	level->GetCameraManagerRef().ChangeZOrder(this, _oldZOrder);
}

std::string Krampus::SpriteComponent::ToString() const
{
	return name + " -> Size/Radius = " + (GetShapeType() == ShapeType::Rectangle ? GetSize().ToString() : GetRadius().ToString());
}

