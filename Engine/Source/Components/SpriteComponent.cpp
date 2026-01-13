#include "SpriteComponent.h"
#include "Graphics/Window/Window.h"
#include "Actors/Actor.h"
#include "GameFramework/Level.h"
#include "Managers/CameraManager.h"

Krampus::SpriteComponent::SpriteComponent(Actor* _owner, const CircleShapeData& _data)
	: Component(_owner)
{
	shape = std::make_unique<ShapeObject>(_data);
}

Krampus::SpriteComponent::SpriteComponent(Actor* _owner, const RectangleShapeData& _data)
	: Component(_owner)
{
	shape = std::make_unique<ShapeObject>(_data);
}

void Krampus::SpriteComponent::Construct()
{
	Component::Construct();

	level->GetCameraManager().AddToWindow(this);
}

void Krampus::SpriteComponent::Deconstruct()
{
	Component::Deconstruct();

	level->GetCameraManager().RemoveToWindow(this);
}

void Krampus::SpriteComponent::Tick(const float& _deltaTime)
{
	Component::Tick(_deltaTime);

	shape->GetShape()->SetTransform(transform);
}


void Krampus::SpriteComponent::Draw(Window& _window)
{
	if (!isActive) return;
	_window.Draw(*shape);
	DrawDebug(_window);
}

void Krampus::SpriteComponent::DrawDebug(Window& _window)
{
#ifdef DEBUG
	if (!useDebug) return;

	const FVector2& _position = transform.position;

	switch (shape->GetShapeType())
	{
	case ShapeType::Circle:
		Debug::DrawDebugCircle(_window, _position, shape->GetSizeData().radius, 15, Color::Green());
		break;

	case ShapeType::Rectangle:
		Debug::DrawDebugRect(_window, _position, shape->GetSizeData().size, transform.rotation, Color::Green());
		break;
	};
#endif
}

void Krampus::SpriteComponent::SetZOrder(const uint8_t& _zOrder)
{
	const uint8_t& _oldZOrder = GetZOrder();
	IDrawable::SetZOrder(_zOrder);
	level->GetCameraManager().ChangeZOrder(this, _oldZOrder);
}

void Krampus::SpriteComponent::SetZOrder(const ZOrder& _zOrder)
{
	const uint8_t& _oldZOrder = GetZOrder();
	IDrawable::SetZOrder(_zOrder);
	level->GetCameraManager().ChangeZOrder(this, _oldZOrder);
}

