#include "SpriteComponent.h"
#include "Graphics/Window/Window.h"
#include "Actors/Actor.h"
#include "GameFramework/Level.h"
#include "Managers/CameraManager.h"

Krampus::SpriteComponent::SpriteComponent(Actor* _owner, const float& _radius, const std::string& _path, const TextureExtensionType& _textureType, const IRect& _rect, const size_t& _pointCount)
	: Component(_owner)
{
	shape = std::make_unique<ShapeObject>(_radius, _path, _textureType, _rect, _pointCount);
}

Krampus::SpriteComponent::SpriteComponent(Actor* _owner, const FVector2& _size, const std::string& _path, const TextureExtensionType& _textureType, const IRect& _rect, const bool& _isRepeated)
	: Component(_owner)
{
	shape = std::make_unique<ShapeObject>(_size, _path, _textureType, _rect, _isRepeated);
}

void Krampus::SpriteComponent::Construct()
{
	Component::Construct();

	SetOriginAtMiddle();
	owner->GetLevel()->GetCameraManager().AddToWindow(this); // TODO zOrder
}

void Krampus::SpriteComponent::Deconstruct()
{
	Component::Deconstruct();

	owner->GetLevel()->GetCameraManager().RemoveToWindow(this);
}

void Krampus::SpriteComponent::Tick(const float& _deltaTime)
{
	Component::Tick(_deltaTime);

	shape->GetShape()->SetTransform(owner->transform);
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

	const FVector2& _position = owner->transform.position;

	switch (shape->GetShapeType())
	{
	case ShapeType::Circle:
		Debug::DrawDebugCircle(_window, _position, shape->GetSizeData().radius, 15, Color::Green());
		break;

	case ShapeType::Rectangle:
		Debug::DrawDebugRect(_window, _position, shape->GetSizeData().size, owner->transform.rotation, Color::Green());
		break;
	};
#endif
}

void Krampus::SpriteComponent::SetOriginAtMiddle()
{
	owner->transform.origin = shape->GetShape()->GetGeometricCenter();
}

