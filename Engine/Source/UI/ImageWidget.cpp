#include "ImageWidget.h"

Krampus::ImageWidget::ImageWidget(Level* _level, const CircleShapeData& _data)
	: Widget(_level)
{
	sprite = CreateComponent<SpriteComponent>(_data);
	sprite->SetZOrder(ZOrder::Widgets);
}

Krampus::ImageWidget::ImageWidget(Level* _level, const RectangleShapeData& _data)
	: Widget(_level)
{
	sprite = CreateComponent<SpriteComponent>(_data);
	sprite->SetZOrder(ZOrder::Widgets);
}
