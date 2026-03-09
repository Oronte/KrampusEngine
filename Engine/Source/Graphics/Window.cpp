#include "Window.h"
#include "Graphics/Shapes/ShapeObject.h"
#include "Components/CameraComponent.h"
#include "Graphics/Ressources/Vertices.h"

void Krampus::Window::SetView(const CameraComponent* _camera)
{
	sf::View _view = *_camera->GetView();
	window.setView(_view);
	onViewChange.Broadcast(_view);
}

void Krampus::Window::Create(const std::string& _name, const UVector2& _windowSize)
{
	window.create(sf::VideoMode(_windowSize),
		CAST(sf::String, _name.c_str()), sf::Style::None);
}

void Krampus::Window::Create(const std::string& _name)
{
	window.create(sf::VideoMode::getDesktopMode(),
		CAST(sf::String, _name.c_str()), sf::Style::None);
}

void Krampus::Window::Clear(const Color& _color)
{
	window.clear(_color);
}

void Krampus::Window::Display()
{
	window.display();
}

void Krampus::Window::Close()
{
	window.close();
}

std::optional<sf::Event> Krampus::Window::PollEvent()
{
	return window.pollEvent();
}

void Krampus::Window::Draw(const sf::Drawable& _drawable)
{
	window.draw(_drawable);
}

void Krampus::Window::Draw(const ShapeObject& _drawable)
{
	Draw(*(_drawable.GetShape()->Get()));
}