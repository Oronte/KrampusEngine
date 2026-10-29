#include "HUD.h"

sf::View Krampus::HUD::GetView() const
{
	return view;
}

void Krampus::HUD::SetView(const sf::View& _view)
{
	view = _view;
}

// A widget may create or destroy widgets from its callbacks: the list is walked on a copy (a destroyed widget stays alive until
// the end of the frame, in pendingDeleteWidgets).
static std::vector<Krampus::Widget*> Snapshot(const std::vector<std::unique_ptr<Krampus::Widget>>& _widgets)
{
	std::vector<Krampus::Widget*> _snapshot;
	_snapshot.reserve(_widgets.size());
	for (const std::unique_ptr<Krampus::Widget>& _widget : _widgets)
		_snapshot.push_back(_widget.get());
	return _snapshot;
}

void Krampus::HUD::BeginPlay()
{
	for (Widget* _widget : Snapshot(widgets))
		if (_widget->IsActive() && !_widget->IsPendingDestroy()) _widget->BeginPlay();
}

void Krampus::HUD::Update(const Float& _deltaTime)
{
	for (Widget* _widget : Snapshot(widgets))
		if (_widget->IsActive() && !_widget->IsPendingDestroy()) _widget->Tick(_deltaTime);
}

void Krampus::HUD::Deconstruct()
{
	for (Widget* _widget : Snapshot(widgets))
		if (_widget->IsActive() && !_widget->IsPendingDestroy()) _widget->Deconstruct();
}

void Krampus::HUD::BeginDestroy()
{
	for (Widget* _widget : Snapshot(widgets))
		if (_widget->IsActive()) _widget->BeginDestroy();
}