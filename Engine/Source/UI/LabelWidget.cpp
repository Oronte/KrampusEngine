#include "LabelWidget.h"

Krampus::LabelWidget::LabelWidget(Level* _level, const TextData& _data)
	: Widget(_level)
{
	label = CreateComponent<LabelComponent>(_data);
	label->SetZOrder(ZOrder::Widgets);
}
