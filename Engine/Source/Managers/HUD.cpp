#include "HUD.h"

void Krampus::HUD::BeginPlay()
{
	for (const std::unique_ptr<Widget>& _widget : widgets)
		if (_widget->IsActive()) _widget->BeginPlay();
}

void Krampus::HUD::Update(const float& _deltaTime)
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