#pragma once
#include "Layers/ScreenSpace.h"

namespace Krampus
{

	/// <summary>
	/// Overlay that saves the window as a PNG when a key is pressed (F12 by default).
	/// Push it last so it sits on top and captures every other layer.
	/// </summary>
	class ScreenshotLayer : public Layer
	{
		sf::Keyboard::Key	captureKey	= sf::Keyboard::Key::F12;
		std::string			directory	= "Content/Screenshots/";
		bool				pending		= false;

	public:
		ScreenshotLayer() = default;

		ScreenshotLayer(const sf::Keyboard::Key& _captureKey, const std::string& _directory)
			: captureKey(_captureKey), directory(_directory) { }

		Bool OnEvent(const sf::Event& _event) override
		{
			if (const sf::Event::KeyPressed* _key = _event.getIf<sf::Event::KeyPressed>())
				if (_key->code == captureKey)
					pending = true;
			return false; // never consume: the key still reaches the game
		}

		// Capture here, before Display(), while the frame is complete in the back buffer.
		void OnRender(Window& _window) override
		{
			if (!pending) return;
			pending = false;

			sf::RenderWindow& _renderWindow = _window.GetRenderWindow();
			sf::Texture _texture;
			if (!_texture.resize(_renderWindow.getSize()))
			{
				LOG_ERROR("Screenshot failed: cannot create texture");
				return;
			}
			_texture.update(_renderWindow);

			std::filesystem::create_directories(directory);
			char _stamp[32];
			const std::time_t _now = std::time(nullptr);
			std::strftime(_stamp, sizeof(_stamp), "%Y%m%d_%H%M%S", std::localtime(&_now));
			const std::string _file = directory + "screenshot_" + _stamp + ".png";

			if (_texture.copyToImage().saveToFile(_file))
				LOG_MSG("Screenshot saved: " + _file);
			else
				LOG_ERROR("Screenshot failed: cannot save " + _file);
		}
	};
}
