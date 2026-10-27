#pragma once
#include "Widget.h"
#include "WidgetComponents/ButtonWidgetComponent.h"
#include "WidgetComponents/ImageWidgetComponent.h"

namespace Krampus
{
    /// <summary>
    /// Interactive HUD button widget with hover, click, and release event handling.
    /// </summary>
    class ButtonWidget : public Widget
    {
        Event<>::ListenerHandle			onHoverHandle;
        Event<>::ListenerHandle			onUnhoverHandle;
        Event<>::ListenerHandle			onPressHandle;
        Event<>::ListenerHandle			onPerformHandle;
        Event<>::ListenerHandle			onReleaseHandle;

    public:
        /// <summary>Broadcast when mouse pointer hovers over the button.</summary>
        Event<>							onHover;
        /// <summary>Broadcast when mouse pointer leaves the button.</summary>
        Event<>							onUnhover;
        /// <summary>Broadcast when button is clicked down.</summary>
        Event<>							onPress;
        /// <summary>Broadcast every frame while button is held down.</summary>
        Event<>							onPerform;
        /// <summary>Broadcast when click is released.</summary>
        Event<>							onRelease;

    protected:
        ButtonWidgetComponent*  buttonComponent = nullptr;
        ImageWidgetComponent*   imageComponent = nullptr;

    public:
        /// <summary>Gets the internal ButtonWidgetComponent.</summary>
        /// <returns>Pointer to ButtonWidgetComponent.</returns>
        ButtonWidgetComponent* GetButtonComponent() const
        {
            return buttonComponent;
        }

        /// <summary>Constructs a rectangular ButtonWidget.</summary>
        /// <param name="_level">Pointer to owning Level.</param>
        /// <param name="_data">Rectangle shape configuration.</param>
        ButtonWidget(Level* _level, const RectangleShapeData& _data);

        /// <summary>Constructs a circular ButtonWidget.</summary>
        /// <param name="_level">Pointer to owning Level.</param>
        /// <param name="_data">Circle shape configuration.</param>
        ButtonWidget(Level* _level, const CircleShapeData& _data);

        /// <summary>Constructs internal components and binds interaction callbacks.</summary>
        virtual void Construct() override;
    };
}
