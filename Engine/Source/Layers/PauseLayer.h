#pragma once
#include "Layers/ScreenSpace.h"

namespace Krampus
{

	/// <summary>
	/// Overlay toggled by a key (Escape by default). While paused, the layers below are not updated, the InputManager is cut off, and a dimmed screen with a pause icon is drawn.
	/// </summary>
	class PauseLayer : public Layer
	{
		sf::Keyboard::Key	toggleKey	= sf::Keyboard::Key::Escape;
		bool				paused		= false;
		bool				enabled		= true;

	public:
		PauseLayer() = default;

		PauseLayer(const sf::Keyboard::Key& _toggleKey)
			: toggleKey(_toggleKey) { }

		void SetPaused(const bool& _paused)
		{
			if (paused != _paused) LOG_MSG(_paused ? "Pause: game paused" : "Pause: game resumed");
			paused = _paused;
		}

		bool IsPaused() const
		{
			return paused;
		}

		/// <summary>A game without pause (a network game, or Escape used for its own menu): the key does nothing, a pause stops.</summary>
		void SetEnabled(const bool& _enabled)
		{
			enabled = _enabled;
			if (!enabled) SetPaused(false);
		}

		bool IsEnabled() const
		{
			return enabled;
		}

		Bool PausesLayersBelow() const override
		{
			return paused;
		}

		Bool BlocksInput() const override
		{
			return paused;
		}

		Bool OnEvent(const sf::Event& _event) override
		{
			if (!enabled) return false;
			if (const sf::Event::KeyPressed* _key = _event.getIf<sf::Event::KeyPressed>())
				if (_key->code == toggleKey)
				{
					paused = !paused;
					return true;
				}

			return paused; // swallow everything else while paused
		}

		void OnRender(Window& _window) override
		{
			if (!paused) return;

		}
	};
}
