#pragma once
#include "UI/Diegetic/DiegeticLabelWidget.h"
#include "HealthComponent.h"

using namespace Krampus;

class HealthWidget : public DiegeticLabelWidget
{
	HealthComponent*	targetComponent			= nullptr;
	Float				colorChangePercentage	= 0.3;

	Event<Int>::ListenerHandle handle;

public:
	HealthWidget(Level* _level, HealthComponent* _target);

private:
	void UpdateString() const;
};

