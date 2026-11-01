#include "BasicWidgets.h"

Krampus::ImageWidget::ImageWidget(Level* _level, const RectangleShapeData& _data)
	: Widget(_level)
{
	name = NAME_OF(ImageWidget);
	image = CreateComponent<ImageWidgetComponent>(_data);
}

Krampus::TextWidget::TextWidget(Level* _level, const TextData& _data)
	: Widget(_level)
{
	name = NAME_OF(TextWidget);
	label = CreateComponent<LabelWidgetComponent>(_data);
}
