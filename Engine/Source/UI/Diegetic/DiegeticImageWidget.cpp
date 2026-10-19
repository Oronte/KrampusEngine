#include "DiegeticImageWidget.h"

Krampus::DiegeticImageWidget::DiegeticImageWidget(Level* _level, const CircleShapeData& _data)
	: DiegeticWidget(_level)
{
	name = NAME_OF(ImageWidget);
	sprite = CreateComponent<SpriteComponent>(_data);
	sprite->SetZOrder(ZOrder::Widgets);
}

Krampus::DiegeticImageWidget::DiegeticImageWidget(Level* _level, const RectangleShapeData& _data)
	: DiegeticWidget(_level)
{
	name = NAME_OF(ImageWidget);
	sprite = CreateComponent<SpriteComponent>(_data);
	sprite->SetZOrder(ZOrder::Widgets);
}

std::string Krampus::DiegeticImageWidget::ToString() const
{
	return name + " -> Size/Radius = " + (sprite->GetShapeType() == ShapeType::Circle ? sprite->GetRadius().ToString() : sprite->GetSize().ToString());
}
