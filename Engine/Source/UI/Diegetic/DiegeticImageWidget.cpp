#include "DiegeticImageWidget.h"

Krampus::DiegeticImageWidget::DiegeticImageWidget(Level* _level)
	: DiegeticImageWidget(_level, RectangleShapeData(FVector2(200.f, 80.f)))
{
}

Krampus::DiegeticImageWidget::DiegeticImageWidget(Level* _level, const CircleShapeData& _data)
	: DiegeticWidget(_level)
{
	name = NAME_OF(DiegeticImageWidget);
	sprite = CreateComponent<SpriteComponent>(_data);
	sprite->SetZOrder(ZOrder::Widgets);
}

Krampus::DiegeticImageWidget::DiegeticImageWidget(Level* _level, const RectangleShapeData& _data)
	: DiegeticWidget(_level)
{
	name = NAME_OF(DiegeticImageWidget);
	sprite = CreateComponent<SpriteComponent>(_data);
	sprite->SetZOrder(ZOrder::Widgets);
}

std::string Krampus::DiegeticImageWidget::ToString() const
{
	if (sprite->GetShapeType() == ShapeType::Polygon) return name + " -> Polygon = " + sprite->GetShapeObject()->GetPolygonName();
	return name + " -> Size/Radius = " + (sprite->GetShapeType() == ShapeType::Circle ? sprite->GetRadius().ToString() : sprite->GetSize().ToString());
}
