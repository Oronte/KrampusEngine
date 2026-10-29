
#include "ButtonWidgetComponent.h"

#include "ImageWidgetComponent.h"
#include "Managers/InputManager.h"
#include "Utilities/Math/Physics.h"
#include "Graphics/Mouse.h"
#include "UI/Widgets/Widget.h"
#include "GameFramework/Level.h"

Krampus::ButtonWidgetComponent::ButtonWidgetComponent(Widget* _owner)
    : WidgetComponent(_owner)
{
    name = NAME_OF(ButtonWidgetComponent);
    Init();
    ImageWidgetComponent* _image = owner->GetComponent<ImageWidgetComponent>();
    if (!_image)
    {
        LOG_ERROR("This Constructor of ButtonWidgetComponent requires an ImageWidgetComponent");
        return;
    }
    shape = _image->GetShapeObject();
}

void Krampus::ButtonWidgetComponent::Tick(const Float &_deltaTime)
{
    WidgetComponent::Tick(_deltaTime);

    if (isPressed) onPerform.Broadcast();
}

void Krampus::ButtonWidgetComponent::Init()
{
    InputManager* _inputManager = GetWorld()->GetInputManager();

    onMouseMovedHandle = _inputManager->MouseMovedScreen.AddListener(this, &ButtonWidgetComponent::OnMouseMoved);
    onClickHandle = _inputManager->MouseLeftClick.onPress.AddListener(this, &ButtonWidgetComponent::OnClick);
    onReleaseHandle = _inputManager->MouseLeftClick.onRelease.AddListener(this, &ButtonWidgetComponent::OnRelease);
}

void Krampus::ButtonWidgetComponent::OnMouseMoved(const IVector2 &_screenPos)
{
    // Widgets live in the HUD view (not in window pixels): map the cursor through it, whatever the window size is.
    const IVector2 _mousePos = GetWorld()->GetWindowRef().MapPixelToCoords(_screenPos, owner->GetLevel()->GetHUD().GetView());

    if (!shape)
    {
        LOG_ERROR("This Constructor of ButtonWidgetComponent requires an ShapeObject");
        return;
    }
    CollisionInfo _info;
    Bool _colliding = false;
    if (shape->GetShapeType() == ShapeType::Circle)
        _colliding = Physics::Contains(_mousePos, GetPosition(), shape->GetSizeData().radius, _info);
    else
        _colliding = Physics::Contains(_mousePos, GetPosition(), shape->GetSizeData().size, GetRotation(), _info);

    if (_colliding && !isHovered) { onHover.Broadcast(); onHoverCalls.Invoke(this); }
    else if (!_colliding && isHovered) { onUnhover.Broadcast(); onUnhoverCalls.Invoke(this); }

    isHovered = _colliding;
}

void Krampus::ButtonWidgetComponent::OnClick()
{
    if (isHovered) LOG(VerbosityType::Verbose, "Button: pressed on " + owner->GetName().StdString());
    if (!isPressed && isHovered) { onPress.Broadcast(); onPressCalls.Invoke(this); }
    isPressed = isHovered;
}

void Krampus::ButtonWidgetComponent::OnRelease()
{
    if (isPressed) { onRelease.Broadcast(); onReleaseCalls.Invoke(this); }
    isPressed = false;
}

std::string Krampus::ButtonWidgetComponent::ToString() const
{
	return name + " -> IsHovered = " + isHovered.ToString() + ", IsPressed = " + isPressed.ToString();
}
