
#include "ButtonWidget.h"

Krampus::ButtonWidget::ButtonWidget(Level *_level, const RectangleShapeData &_data)
    : Widget(_level)
{
    name = NAME_OF(ButtonWidget);
    imageComponent = CreateComponent<ImageWidgetComponent>(_data);
    buttonComponent = CreateComponent<ButtonWidgetComponent>();
}

Krampus::ButtonWidget::ButtonWidget(Level *_level, const CircleShapeData &_data)
    : Widget(_level)
{
    name = NAME_OF(ButtonWidget);
    imageComponent = CreateComponent<ImageWidgetComponent>(_data);
    buttonComponent = CreateComponent<ButtonWidgetComponent>();
}

void Krampus::ButtonWidget::Construct()
{
    Widget::Construct();

    onHoverHandle = buttonComponent->onHover.AddListener([&]() {onHover.Broadcast();});
    onUnhoverHandle = buttonComponent->onUnhover.AddListener([&]() {onUnhover.Broadcast();});
    onPressHandle = buttonComponent->onPress.AddListener([&]() {onPress.Broadcast();});
    onPerformHandle = buttonComponent->onPerform.AddListener([&]() {onPerform.Broadcast();});
    onReleaseHandle = buttonComponent->onRelease.AddListener([&]() {onRelease.Broadcast();});
}
