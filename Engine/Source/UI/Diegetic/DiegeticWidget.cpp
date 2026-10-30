#include "DiegeticWidget.h"
#include "GameFramework/Level.h"
#include "Graphics/Window.h"

Krampus::DiegeticWidget::DiegeticWidget(Level* _level)
	: Actor(_level)
{
	name = NAME_OF(DiegeticWidget);
	onViewChangeHandle = level->GetWindow().onViewChange.AddListener(this, &DiegeticWidget::UpdateWorldPos);
}

void Krampus::DiegeticWidget::SetScreenPosition(const FVector2& _screenPos)
{
	screenPosition = _screenPos;
	UpdateWorldPos(level->GetWindow().GetView());
}

void Krampus::DiegeticWidget::SetLocalRotation(const Angle& _rotation)
{
	localRotation = _rotation;
	UpdateWorldPos(level->GetWindow().GetView());
}

void Krampus::DiegeticWidget::BeginDestroy()
{
	// Destroy() is the one of every actor (deconstruct now, delete at the end of the frame, once). The view stops moving it here.
	onViewChangeHandle.Disconnect();
	Actor::BeginDestroy();
}

void Krampus::DiegeticWidget::UpdateWorldPos(const sf::View& _view)
{
	if (IsPendingDestroy()) return;
	SetPosition(level->GetWindow().MapPixelToCoords(screenPosition, _view));
	SetRotation(localRotation + Angle(_view.getRotation()));
}

std::string Krampus::DiegeticWidget::ToString() const
{
	return name + " -> Screen Position = " + screenPosition.ToString();
}
