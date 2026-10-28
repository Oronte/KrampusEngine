#include "Window.h"
#include "Graphics/Shapes/ShapeObject.h"
#include "Components/CameraComponent.h"

void Krampus::Window::SetView(sf::View _view)
{
	window->setView(_view);
	onViewChange.Broadcast(_view);
}

void Krampus::Window::SetView(const CameraComponent* _camera)
{
	sf::View _view = *_camera->GetView();
	window->setView(_view);
	onViewChange.Broadcast(_view);
}

void Krampus::Window::Create(const String& _name, const UVector2& _windowSize)
{
	window->create(sf::VideoMode(_windowSize),
		CAST(sf::String, _name.ToCharPtr()), sf::Style::None);
}

void Krampus::Window::Create(const String& _name)
{
	window->create(sf::VideoMode::getDesktopMode(),
		CAST(sf::String, _name.ToCharPtr()), sf::Style::None);
}

Krampus::IVector2 Krampus::Window::GetMousePosition() const
{
	sf::Vector2f _position(sf::Mouse::getPosition(*window));
	if (mouseViewport.size.x > 0.f && mouseViewport.size.y > 0.f)
	{
		const sf::Vector2f _windowSize(window->getSize());
		_position.x = (_position.x - mouseViewport.position.x) * _windowSize.x / mouseViewport.size.x;
		_position.y = (_position.y - mouseViewport.position.y) * _windowSize.y / mouseViewport.size.y;
	}
	return IVector2(sf::Vector2i(_position));
}

void Krampus::Window::SetMousePosition(const IVector2& _position)
{
	const sf::Vector2i _pixel = _position;
	sf::Vector2f _target(_pixel);
	if (mouseViewport.size.x > 0.f && mouseViewport.size.y > 0.f)
	{
		const sf::Vector2f _windowSize(window->getSize());
		_target.x = _target.x * mouseViewport.size.x / _windowSize.x + mouseViewport.position.x;
		_target.y = _target.y * mouseViewport.size.y / _windowSize.y + mouseViewport.position.y;
	}
	sf::Mouse::setPosition(sf::Vector2i(_target), *window);
}

void Krampus::Window::Clear(const Color& _color)
{
	window->clear(_color);
}

void Krampus::Window::Display()
{
	window->display();
}

void Krampus::Window::Close()
{
	if (window == &ownWindow)
		window->close();
}

std::optional<sf::Event> Krampus::Window::PollEvent()
{
	return window->pollEvent();
}

void Krampus::Window::Draw(const sf::Drawable& _drawable)
{
	window->draw(_drawable);
}

void Krampus::Window::Draw(const ShapeObject& _drawable)
{
	Draw(*(_drawable.GetShape()->Get()));
}