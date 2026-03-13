#include "ImageWidget.h"

Krampus::ImageWidget::ImageWidget(Level* _level, const CircleShapeData& _data)
	: Widget(_level)
{
	name = NAME_OF(ImageWidget);
	sprite = CreateComponent<SpriteComponent>(_data);
	sprite->SetZOrder(ZOrder::Widgets);
}

Krampus::ImageWidget::ImageWidget(Level* _level, const RectangleShapeData& _data)
	: Widget(_level)
{
	name = NAME_OF(ImageWidget);
	sprite = CreateComponent<SpriteComponent>(_data);
	sprite->SetZOrder(ZOrder::Widgets);
}

std::string Krampus::ImageWidget::ToString() const
{
	return name + " -> Size/Radius = " + (sprite->GetShapeType() == ShapeType::Circle ? sprite->GetRadius().ToString() : sprite->GetSize().ToString());
}
