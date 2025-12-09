#include "CameraManager.h"

void Krampus::CameraManager::Render(Window& _window, const bool& splitScreen)
{
	for (std::pair<int, std::vector<IDrawable*>> _pair : drawer)
	{
		const std::vector<IDrawable*>& _sprites = _pair.second;

		for (IDrawable* _sprite : _sprites) _sprite->Draw(_window);
	}
	
}
