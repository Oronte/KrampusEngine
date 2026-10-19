#pragma once
#include "WidgetComponent.h"

namespace Krampus
{
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
        Event<>							onHover;
        Event<>							onUnhover;
        Event<>							onPress;
        Event<>							onPerform;
        Event<>							onRelease;

        // Require a ImageWidgetComponent
        ButtonWidgetComponent(Widget* _owner);
        //ButtonWidgetComponent(Level* _level, const CircleShapeData& _data);
        //ButtonWidgetComponent(Level* _level, const RectangleShapeData& _data);

    protected:
        virtual void Tick(const Float& _deltaTime) override;

    private:
        void Init();

        void OnMouseMoved(const IVector2& _mousePos);
        void OnClick();
        void OnRelease();

    public:
        virtual std::string ToString() const override;
    };
}
