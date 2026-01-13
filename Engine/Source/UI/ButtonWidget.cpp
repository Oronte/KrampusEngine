#include "ButtonWidget.h"
#include "Graphics/Mouse.h"
#include "Managers/InputManager.h"
#include "Utilities/Math/Physics.h"

Krampus::ButtonWidget::ButtonWidget(Level* _level, const CircleShapeData& _data)
	: ImageWidget(_level, _data)
{
	Init();
}

Krampus::ButtonWidget::ButtonWidget(Level* _level, const RectangleShapeData& _data)
	: ImageWidget(_level, _data)
{
	Init();
}

void Krampus::ButtonWidget::Tick(const float& _deltaTime)
{
	ImageWidget::Tick(_deltaTime);

	if (isPressed) onPerform.Broadcast();
}

void Krampus::ButtonWidget::Init()
{
	InputManager& _inputManager = M_INPUT;

	_inputManager.MouseMovedScreen.AddListener(this, &Krampus::ButtonWidget::OnMouseMoved);
	_inputManager.MouseLeftClick.onPress.AddListener(this, &Krampus::ButtonWidget::OnClick);
	_inputManager.MouseLeftClick.onRelease.AddListener(this, &Krampus::ButtonWidget::OnRelease);
}

void Krampus::ButtonWidget::OnMouseMoved(const IVector2& _mousePos)
{
	CollisionInfo _info;
	bool _colliding = false;
	ShapeObject* _object = sprite->GetShapeObject();
	if (_object->GetShapeType() == ShapeType::Circle)
		_colliding = Physics::Contains(_mousePos, screenPosition, _object->GetSizeData().radius, _info);
	else
		_colliding = Physics::Contains(_mousePos, screenPosition, _object->GetSizeData().size, localRotation, _info);

	if (_colliding && !isHovered) onHover.Broadcast();
	else if (!_colliding && isHovered) onUnhover.Broadcast();

	isHovered = _colliding;
}

void Krampus::ButtonWidget::OnClick()
{
	if (!isPressed && isHovered) onPress.Broadcast();
	isPressed = isHovered;
}

void Krampus::ButtonWidget::OnRelease()
{
	if (isPressed) onRelease.Broadcast();
	isPressed = false;
}

void Krampus::ButtonWidget::BeginDestroy()
{
	ImageWidget::BeginDestroy();

	InputManager& _inputManager = M_INPUT;

	_inputManager.MouseMovedScreen.RemoveAllFrom(this);
	_inputManager.MouseLeftClick.onPress.RemoveAllFrom(this);
	_inputManager.MouseLeftClick.onRelease.RemoveAllFrom(this);
}
