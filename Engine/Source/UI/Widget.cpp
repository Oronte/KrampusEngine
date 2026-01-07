#include "Widget.h"
#include "GameFramework/Level.h"

Krampus::Widget::Widget(Level* _level)
	: Actor(_level)
{
}

void Krampus::Widget::Destroy()
{
	BeginDestroy();
	level->GetHUD().DeleteWidget(this);
}
