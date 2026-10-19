#include "DiegeticLabelWidget.h"

Krampus::DiegeticLabelWidget::DiegeticLabelWidget(Level* _level, const TextData& _data)
	: DiegeticWidget(_level)
{
	name = NAME_OF(GIFWidget);
	label = CreateComponent<LabelComponent>(_data);
	label->SetZOrder(ZOrder::Widgets);
}

std::string Krampus::DiegeticLabelWidget::ToString() const
{
	return name + " -> Label = " + label->GetString();
}
