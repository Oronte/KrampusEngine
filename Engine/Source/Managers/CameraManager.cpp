#include "CameraManager.h"
#include "Graphics/Window/MainWindow.h"
#include "Graphics/Window/MainWindow.h"

void Krampus::CameraManager::UpdateView()
{
	if (!current) return;
	MAIN_WINDOW.SetView(current);
}

void Krampus::CameraManager::Render(Window& _window, const bool& splitScreen)
{
	UpdateView();

	for (std::pair<int, std::vector<IDrawable*>> _pair : drawer)
	{
		const std::vector<IDrawable*>& _sprites = _pair.second;

		for (IDrawable* _sprite : _sprites) _sprite->Draw(_window);
	}
	
}
