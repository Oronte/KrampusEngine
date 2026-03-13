#include "HealthWidget.h"

HealthWidget::HealthWidget(Level* _level, HealthComponent* _target)
	: LabelWidget(_level, TextData("0 / 0")), targetComponent(_target)
{
	label->SetFillColor(Color::Green());
	UpdateString();
	handle = targetComponent->onHealthUpdate.AddListener([this](Int _health)
		{
			UpdateString();
		});
}

void HealthWidget::UpdateString() const
{
	Int _maxHealth = targetComponent->GetMaxHealth();
	Int _currentHealth = targetComponent->GetCurrentHealth();

	if (Float((int)_currentHealth) / Float((int)_maxHealth) <= colorChangePercentage)
		label->SetFillColor(Color::Red());
	else
		label->SetFillColor(Color::Green());

	label->SetString(_currentHealth.ToString() + " / " + _maxHealth.ToString());
}
