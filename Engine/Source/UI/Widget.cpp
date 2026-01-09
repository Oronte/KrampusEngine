#include "Widget.h"
#include "GameFramework/Level.h"
#include "Graphics/Window/MainWindow.h"

Krampus::Widget::Widget(Level* _level)
	: Actor(_level)
{
	MAIN_WINDOW.onViewChange.AddListener(this, &Widget::UpdateWorldPos);
}

void Krampus::Widget::Destroy()
{
	BeginDestroy();
	level->GetHUD().DeleteWidget(this);
}

void Krampus::Widget::UpdateWorldPos(const sf::View& _view)
{
	transform.position = MAIN_WINDOW.MapPixelToCoords(screenPosition, _view);
	transform.rotation = localRotation + Angle(_view.getRotation());
}
