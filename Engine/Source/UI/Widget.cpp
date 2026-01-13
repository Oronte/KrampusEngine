#include "Widget.h"
#include "GameFramework/Level.h"
#include "Graphics/Window/MainWindow.h"

Krampus::Widget::Widget(Level* _level)
	: Actor(_level)
{
	MAIN_WINDOW.onViewChange.AddListener(this, &Widget::UpdateWorldPos);
}

void Krampus::Widget::SetScreenPosition(const FVector2& _screenPos)
{
	screenPosition = _screenPos;
	UpdateWorldPos(MAIN_WINDOW.GetView());
}

void Krampus::Widget::SetLocalRotation(const Angle& _rotation)
{
	localRotation = _rotation;
	UpdateWorldPos(MAIN_WINDOW.GetView());
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

void Krampus::Widget::BeginDestroy()
{
	Actor::BeginDestroy();
	MAIN_WINDOW.onViewChange.RemoveAllFrom(this);
}
