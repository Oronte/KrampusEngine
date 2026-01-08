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
	const Angle& _viewRot = Angle(_view.getRotation());
	const FVector2& _viewHalfSize = _view.getSize() / 2.0f;

	transform.position = screenPosition + _view.getCenter() - _viewHalfSize.Rotated(_viewRot);
	transform.rotation = localRotation + _viewRot;
}
