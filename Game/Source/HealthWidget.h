#pragma once
#include "UI/LabelWidget.h"
#include "HealthComponent.h"

using namespace Krampus;

class HealthWidget : public LabelWidget
{
	HealthComponent*	targetComponent			= nullptr;
	Float				colorChangePercentage	= 0.3;

	Event<Int>::ListenerHandle handle;

public:
	HealthWidget(Level* _level, HealthComponent* _target);

private:
	void UpdateString() const;
};

