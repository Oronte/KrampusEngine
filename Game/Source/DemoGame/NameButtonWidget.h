#pragma once
#include "UI/ButtonWidget.h"
#include "Components/LabelComponent.h"

using namespace Krampus;

class NameButtonWidget : public ButtonWidget
{
	LabelComponent*			label			= nullptr;
	Event<>::ListenerHandle hoverHandle;
	Event<>::ListenerHandle unhoverHandle;
	Sound					sound;

public:
	NameButtonWidget(Level* _level, const RectangleShapeData& _data, const String& _label);

	virtual void Construct() override;

	void OnUnhover();
	void OnHover();
};

