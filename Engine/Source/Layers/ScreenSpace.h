#pragma once
#include "Core/Layer.h"

namespace Krampus
{

	/// <summary>
	/// Switches the window to a pixel-sized view so overlays draw in screen coordinates, and returns the window size in pixels.
	/// The next CameraManager::Render sets its own view again.
	/// </summary>
	inline sf::Vector2f UseScreenView(Window& _window)
	{
		sf::RenderWindow& _renderWindow = _window.GetRenderWindow();
		const sf::Vector2f _size(_renderWindow.getSize());
		_renderWindow.setView(sf::View(sf::FloatRect({ 0.f, 0.f }, _size)));
		return _size;
	}

	/// <summary>
	/// Draws a rectangle covering the whole window. Call UseScreenView first.
	/// </summary>
	inline void DrawScreenFill(Window& _window, const sf::Vector2f& _size, const sf::Color& _color)
	{
		sf::RectangleShape _rect(_size);
		_rect.setFillColor(_color);
		_window.Draw(_rect);
	}
}
