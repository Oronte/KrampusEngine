#include "LabelWidget.h"

Krampus::LabelWidget::LabelWidget(Level* _level, const TextData& _data)
	: Widget(_level)
{
	name = NAME_OF(GIFWidget);
	label = CreateComponent<LabelComponent>(_data);
	label->SetZOrder(ZOrder::Widgets);
}

std::string Krampus::LabelWidget::ToString() const
{
	return name + " -> Label = " + label->GetString();
}
