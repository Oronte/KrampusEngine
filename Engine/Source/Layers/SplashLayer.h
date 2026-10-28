#pragma once
#include "Layers/ScreenSpace.h"
#include "Core/AssetPack.h"

namespace Krampus
{

	/// <summary>
	/// Overlay that shows a logo on a black screen (fade in, hold, fade out), then removes itself.
	/// Pauses the layers below and cuts the InputManager meanwhile. Any key or click skips it.
	/// </summary>
	class SplashLayer : public Layer
	{
		static constexpr float	FADE_TIME	= 0.5f;

		sf::Texture				texture;
		std::string				path;
		float					holdTime	= 2.f;
		float					elapsed		= 0.f;

	public:
		/// <param name="_path">Image path, relative like the other Content paths.</param>
		/// <param name="_holdTime">Seconds the logo stays fully visible.</param>
		SplashLayer(const std::string& _path, const float& _holdTime = 2.f)
			: path(_path), holdTime(_holdTime) { }

		Bool PausesLayersBelow() const override
		{
			return true;
		}

		Bool BlocksInput() const override
		{
			return true;
		}

		void OnAttach() override
		{
			const std::uint8_t* _data = nullptr;
			std::size_t _size = 0;
			const bool _loaded = AssetPack::Find(path, _data, _size) ? texture.loadFromMemory(_data, _size) : texture.loadFromFile(path);
			if (!_loaded)
			{
				LOG_WARNING("Splash image not found: " + path);
				Close();
			}
			else
				LOG(VerbosityType::Log, "Splash: showing " + path);
		}

		void OnUpdate(const Float& _deltaTime) override
		{
			elapsed += static_cast<float>(_deltaTime);
			if (elapsed >= holdTime + 2.f * FADE_TIME)
			{
				if (!IsClosed()) LOG(VerbosityType::Log, "Splash: finished");
				Close();
			}
		}

		Bool OnEvent(const sf::Event& _event) override
		{
			if (_event.is<sf::Event::KeyPressed>() || _event.is<sf::Event::MouseButtonPressed>())
			{
				if (!IsClosed()) LOG(VerbosityType::Log, "Splash: skipped");
				Close();
			}
			return true;
		}

		void OnRender(Window& _window) override
		{
			const sf::Vector2f _size = UseScreenView(_window);
			DrawScreenFill(_window, _size, sf::Color::Black);

			float _alpha = 1.f;
			if (elapsed < FADE_TIME)						_alpha = elapsed / FADE_TIME;
			else if (elapsed > holdTime + FADE_TIME)		_alpha = std::max(0.f, (holdTime + 2.f * FADE_TIME - elapsed) / FADE_TIME);

			const sf::Vector2f _texSize(texture.getSize());
			const float _scale = std::min(_size.x * 0.6f / _texSize.x, _size.y * 0.6f / _texSize.y);

			sf::Sprite _sprite(texture);
			_sprite.setOrigin(_texSize / 2.f);
			_sprite.setPosition(_size / 2.f);
			_sprite.setScale({ _scale, _scale });
			_sprite.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(_alpha * 255.f)));
			_window.Draw(_sprite);
		}
	};
}
