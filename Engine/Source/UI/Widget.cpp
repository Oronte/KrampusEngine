#include "Widget.h"
#include "GameFramework/Level.h"
#include "Graphics/Window.h"

Krampus::Widget::Widget(Level* _level)
	: Actor(_level)
{
	name = NAME_OF(Widget);
	onViewChangeHandle = level->GetWindowRef().onViewChange.AddListener(this, &Widget::UpdateWorldPos);
}

void Krampus::Widget::SetScreenPosition(const FVector2& _screenPos)
{
	screenPosition = _screenPos;
	UpdateWorldPos(level->GetWindowRef().GetView());
}

void Krampus::Widget::SetLocalRotation(const Angle& _rotation)
{
	localRotation = _rotation;
	UpdateWorldPos(level->GetWindowRef().GetView());
}

void Krampus::Widget::Destroy()
{
	Deconstruct();
	BeginDestroy();
	level->GetHUDRef().MarkForDeleteWidget(this);
}

void Krampus::Widget::UpdateWorldPos(const sf::View& _view)
{
	SetActorPosition(level->GetWindowRef().MapPixelToCoords(screenPosition, _view));
	SetActorRotation(localRotation + Angle(_view.getRotation()));
}

std::string Krampus::Widget::ToString() const
{
	return name + " -> Screan Position = " + screenPosition.ToString();
}
