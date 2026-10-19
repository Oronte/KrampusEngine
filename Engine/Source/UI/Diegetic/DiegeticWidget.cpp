#include "DiegeticWidget.h"
#include "GameFramework/Level.h"
#include "Graphics/Window.h"

Krampus::DiegeticWidget::DiegeticWidget(Level* _level)
	: Actor(_level)
{
	name = NAME_OF(Widget);
	onViewChangeHandle = level->GetWindowRef().onViewChange.AddListener(this, &DiegeticWidget::UpdateWorldPos);
}

void Krampus::DiegeticWidget::SetScreenPosition(const FVector2& _screenPos)
{
	screenPosition = _screenPos;
	UpdateWorldPos(level->GetWindowRef().GetView());
}

void Krampus::DiegeticWidget::SetLocalRotation(const Angle& _rotation)
{
	localRotation = _rotation;
	UpdateWorldPos(level->GetWindowRef().GetView());
}

void Krampus::DiegeticWidget::Destroy()
{
	Deconstruct();
	BeginDestroy();
	level->GetActorManagerRef().MarkForDeleteActor(this);
}

void Krampus::DiegeticWidget::UpdateWorldPos(const sf::View& _view)
{
	SetPosition(level->GetWindowRef().MapPixelToCoords(screenPosition, _view));
	SetRotation(localRotation + Angle(_view.getRotation()));
}

std::string Krampus::DiegeticWidget::ToString() const
{
	return name + " -> Screan Position = " + screenPosition.ToString();
}
