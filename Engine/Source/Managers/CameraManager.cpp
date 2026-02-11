#include "CameraManager.h"

void Krampus::CameraManager::UpdateView()
{
	if (!current) return;
	GetWorld()->GetWindowRef().SetView(current);
}

void Krampus::CameraManager::Render(const bool& splitScreen)
{
	Window& _window = GetWorld()->GetWindowRef();

	for (std::pair<int, std::vector<IDrawable*>> _pair : drawer)
	{
		const std::vector<IDrawable*>& _sprites = _pair.second;

		for (IDrawable* _sprite : _sprites) _sprite->Draw(_window);
	}

	for (const std::shared_ptr<Vertices> _vert : debugs)
	{
		_window.Draw(*_vert);
	}
	
	debugs.clear();
}
