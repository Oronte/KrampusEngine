#include "DiegeticButtonWidget.h"
#include "Graphics/Mouse.h"
#include "Managers/InputManager.h"
#include "Utilities/Math/Physics.h"

Krampus::DiegeticButtonWidget::DiegeticButtonWidget(Level* _level)
	: DiegeticButtonWidget(_level, RectangleShapeData(FVector2(200.f, 80.f)))
{
}

Krampus::DiegeticButtonWidget::DiegeticButtonWidget(Level* _level, const CircleShapeData& _data)
	: DiegeticImageWidget(_level, _data)
{
	name = NAME_OF(DiegeticButtonWidget);
	Init();
}

Krampus::DiegeticButtonWidget::DiegeticButtonWidget(Level* _level, const RectangleShapeData& _data)
	: DiegeticImageWidget(_level, _data)
{
	name = NAME_OF(DiegeticButtonWidget);
	Init();
}

void Krampus::DiegeticButtonWidget::Tick(const Float& _deltaTime)
{
	DiegeticImageWidget::Tick(_deltaTime);

	if (isPressed) onPerform.Broadcast();
}

void Krampus::DiegeticButtonWidget::Init()
{
	InputManager* _inputManager = GetWorld()->GetInputManager();

	onMouseMovedHandle = _inputManager->MouseMovedScreen.AddListener(this, &Krampus::DiegeticButtonWidget::OnMouseMoved);
	onClickHandle = _inputManager->MouseLeftClick.onPress.AddListener(this, &Krampus::DiegeticButtonWidget::OnClick);
	onReleaseHandle = _inputManager->MouseLeftClick.onRelease.AddListener(this, &Krampus::DiegeticButtonWidget::OnRelease);
}

void Krampus::DiegeticButtonWidget::OnMouseMoved(const IVector2& _mousePos)
{
	// A hidden or dying button, or a level frozen by the editor (outside Play), does not react.
	if (!IsActive() || IsPendingDestroy() || GetWorld()->IsLevelPaused()) return;

	CollisionInfo _info;
	Bool _colliding = false;
	ShapeObject* _object = sprite->GetShapeObject();
	if (_object->GetShapeType() == ShapeType::Circle)
		_colliding = Physics::Contains(_mousePos, screenPosition, _object->GetSizeData().radius, _info);
	else if (_object->GetShapeType() == ShapeType::Polygon)
		_colliding = Polygon2D(Polygon2D::Transform(_object->GetOutline().points, screenPosition, localRotation))
			.Contains(FVector2(static_cast<float>(_mousePos.x), static_cast<float>(_mousePos.y)));
	else
		_colliding = Physics::Contains(_mousePos, screenPosition, _object->GetSizeData().size, localRotation, _info);

	if (_colliding && !isHovered) { onHover.Broadcast(); onHoverCalls.Invoke(this); }
	else if (!_colliding && isHovered) { onUnhover.Broadcast(); onUnhoverCalls.Invoke(this); }

	isHovered = _colliding;
}

void Krampus::DiegeticButtonWidget::OnClick()
{
	if (!IsActive() || IsPendingDestroy() || GetWorld()->IsLevelPaused()) return;
	if (!isPressed && isHovered) { onPress.Broadcast(); onPressCalls.Invoke(this); }
	isPressed = isHovered;
}

void Krampus::DiegeticButtonWidget::OnRelease()
{
	if (isPressed) { onRelease.Broadcast(); onReleaseCalls.Invoke(this); }
	isPressed = false;
}

std::string Krampus::DiegeticButtonWidget::ToString() const
{
	return name + " -> IsHovered = " + isHovered.ToString() + ", IsPressed = " + isPressed.ToString();
}
