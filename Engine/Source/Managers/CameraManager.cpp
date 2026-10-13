#include "CameraManager.h"
#include "Managers/LevelManager.h"

void Krampus::CameraManager::UpdateView()
{
	if (!current) return;
	GetWorld()->GetWindowRef().SetView(current);
}

void Krampus::CameraManager::Render(const Bool& splitScreen)
{
	if (!current)
		current = GetWorld()->GetLevelManager()->GetCurrentLevel()->GetDefaultCamera();
	if (!current)
	{
		LOG_WARNING("There is no current camera ! Objects cannot be drawn on the screen.");
		return;
	}

	Window& _window = GetWorld()->GetWindowRef();

	UpdateView(); // World View
	RenderActors(_window, splitScreen);
	RenderDebugs(_window, splitScreen);

	GetWorld()->GetWindowRef().SetView(GetWorld()->GetLevelManager()->GetCurrentLevel()->GetHUDRef().GetView()); // Widget View
	RenderWidgets(_window, splitScreen);

	//UpdateView(); // World View

	debugs.clear();

	_window.Display();
}

void Krampus::CameraManager::RenderActors(Window& _window, const Bool& splitScreen)
{
	for (const auto& [_zOrder, _sprites] : actors)
		for (IDrawable* _sprite : _sprites)
			_sprite->Draw(_window);
}

void Krampus::CameraManager::RenderDebugs(Window& _window, const Bool& splitScreen)
{
	for (const std::shared_ptr<Vertices> _vert : debugs)
	{
		_window.Draw(*_vert);
	}
}

void Krampus::CameraManager::RenderWidgets(Window& _window, const Bool& splitScreen)
{
	for (const auto& [_zOrder, _sprites] : widgets)
		for (IDrawable* _sprite : _sprites)
			_sprite->Draw(_window);
}
