#include "HUD.h"

sf::View Krampus::HUD::GetView() const
{
	return view;
}

void Krampus::HUD::SetView(const sf::View& _view)
{
	view = _view;
}

void Krampus::HUD::BeginPlay()
{
	for (const std::unique_ptr<Widget>& _widget : widgets)
		if (_widget->IsActive()) _widget->BeginPlay();
}

void Krampus::HUD::Update(const Float& _deltaTime)
{
	for (const std::unique_ptr<Widget>& _widget : widgets)
		if (_widget->IsActive()) _widget->Tick(_deltaTime);
}

void Krampus::HUD::Deconstruct()
{
	for (const std::unique_ptr<Widget>& _widget : widgets)
		if (_widget->IsActive()) _widget->Deconstruct();
}

void Krampus::HUD::BeginDestroy()
{
	for (const std::unique_ptr<Widget>& _widget : widgets)
		if (_widget->IsActive()) _widget->BeginDestroy();
}