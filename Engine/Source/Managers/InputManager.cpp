#include "InputManager.h"
#include "Graphics/Window/Window.h"

void Krampus::InputManager::Update(Window& _window)
{
	const std::optional<sf::Event> _event = _window.PollEvent();
	if (!_event.has_value()) return;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) A.Broadcast();
}
