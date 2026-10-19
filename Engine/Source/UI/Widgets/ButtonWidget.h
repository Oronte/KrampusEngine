#pragma once
#include "Widget.h"
#include "WidgetComponents/ButtonWidgetComponent.h"
#include "WidgetComponents/ImageWidgetComponent.h"

namespace Krampus
{
    class ButtonWidget : public Widget
    {
        Event<>::ListenerHandle			onHoverHandle;
        Event<>::ListenerHandle			onUnhoverHandle;
        Event<>::ListenerHandle			onPressHandle;
        Event<>::ListenerHandle			onPerformHandle;
        Event<>::ListenerHandle			onReleaseHandle;

    public:
        Event<>							onHover;
        Event<>							onUnhover;
        Event<>							onPress;
        Event<>							onPerform;
        Event<>							onRelease;

    protected:
        ButtonWidgetComponent*  buttonComponent = nullptr;
        ImageWidgetComponent*   imageComponent = nullptr;

    public:
        ButtonWidgetComponent* GetButtonComponent() const
        {
            return buttonComponent;
        }

        ButtonWidget(Level* _level, const RectangleShapeData& _data);
        ButtonWidget(Level* _level, const CircleShapeData& _data);

        virtual void Construct() override;
    };
}

