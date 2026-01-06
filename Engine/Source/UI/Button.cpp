#include "Button.h"
#include "Graphics/Mouse.h"
#include "Managers/InputManager.h"

Krampus::Button::Button(Level* _level, const float& _radius, const std::string& _path, const TextureExtensionType& _textureType, const IRect& _rect, const size_t& _pointCount)
	: Widget(_level)
{
	sprite = CreateComponent<SpriteComponent>(_radius, _path, _textureType, _rect, _pointCount);
	sprite->SetZOrder(ZOrder::Widgets);
	collision = CreateComponent<CollisionComponent>();

	M_INPUT.MouseMoved.AddListener(this, &Krampus::Button::OnMouseMoved);
	M_INPUT.MouseLeftClick.onPress.AddListener(this, &Krampus::Button::OnClick);
	M_INPUT.MouseLeftClick.onRelease.AddListener(this, &Krampus::Button::OnRelease);
}

Krampus::Button::Button(Level* _level, const FVector2& _size, const std::string& _path, const TextureExtensionType& _textureType, const IRect& _rect, const bool& _isRepeated)
	: Widget(_level)
{
	sprite = CreateComponent<SpriteComponent>(_size, _path, _textureType, _rect, _isRepeated);
	sprite->SetZOrder(ZOrder::Widgets);
	collision = CreateComponent<CollisionComponent>();

	M_INPUT.MouseMoved.AddListener(this, &Krampus::Button::OnMouseMoved);
	M_INPUT.MouseLeftClick.onPress.AddListener(this, &Krampus::Button::OnClick);
	M_INPUT.MouseLeftClick.onRelease.AddListener(this, &Krampus::Button::OnRelease);
}

void Krampus::Button::OnMouseMoved(const IVector2& _mousePos)
{
	CollisionInfo _info;
	bool _colliding = false;
	ShapeObject* _object = sprite->GetShapeObject();
	if (_object->GetShapeType() == ShapeType::Circle)
		_colliding = Physics::Contains(_mousePos, transform.position, _object->GetSizeData().radius, _info);
	else
		_colliding = Physics::Contains(_mousePos, transform.position, _object->GetSizeData().size, transform.rotation, _info);

	if (_colliding && !isHovered) onHover.Broadcast();
	if (!_colliding && isHovered) onUnhover.Broadcast();

	isHovered = _colliding;
}

void Krampus::Button::OnClick()
{
	if (!isPressed && isHovered) onPress.Broadcast();
	isPressed = isHovered;
}

void Krampus::Button::OnRelease()
{
	if (isPressed) onRelease.Broadcast();
	isPressed = false;
}
