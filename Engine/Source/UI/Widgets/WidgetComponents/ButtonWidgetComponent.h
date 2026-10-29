#pragma once
#include "WidgetComponent.h"
#include "Reflection/EventList.h"

namespace Krampus
{
    /// <summary>
    /// Component providing button interaction, cursor hit testing, and hover/click event dispatching for a widget.
    /// </summary>
    class ButtonWidgetComponent : public WidgetComponent
    {
    protected:
        Bool							isPressed			= false;
        Bool							isHovered			= false;

        ShapeObject*					shape				= nullptr;

        Event<IVector2>::ListenerHandle onMouseMovedHandle;
        Event<>::ListenerHandle			onClickHandle;
        Event<>::ListenerHandle			onReleaseHandle;

    public:
        /// <summary>Broadcast when mouse enters the button area.</summary>
        Event<>							onHover;
        /// <summary>Broadcast when mouse leaves the button area.</summary>
        Event<>							onUnhover;
        /// <summary>Broadcast when mouse clicks down on the button.</summary>
        Event<>							onPress;
        /// <summary>Broadcast every frame while mouse is pressed on button.</summary>
        Event<>							onPerform;
        /// <summary>Broadcast when mouse click is released.</summary>
        Event<>							onRelease;

        /// <summary>Functions called from the editor (event lists) at the same moments as onPress, onRelease, onHover and onUnhover.</summary>
        EventList                       onPressCalls;
        EventList                       onReleaseCalls;
        EventList                       onHoverCalls;
        EventList                       onUnhoverCalls;

        /// <summary>
        /// Constructs a ButtonWidgetComponent attached to a widget.
        /// Note: The owning widget requires an existing ImageWidgetComponent.
        /// </summary>
        /// <param name="_owner">Pointer to owning Widget.</param>
        ButtonWidgetComponent(Widget* _owner);

    protected:
        /// <summary>Updates button hover and perform state each frame.</summary>
        /// <param name="_deltaTime">Elapsed time in seconds.</param>
        virtual void Tick(const Float& _deltaTime) override;

    private:
        void Init();

        void OnMouseMoved(const IVector2& _mousePos);
        void OnClick();
        void OnRelease();

    public:
        /// <summary>Returns a string representation of the component.</summary>
        /// <returns>String representation.</returns>
        virtual std::string ToString() const override;
    };
}
