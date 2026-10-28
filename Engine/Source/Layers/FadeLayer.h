#pragma once
#include "Layers/ScreenSpace.h"
#include "Managers/LevelManager.h"

namespace Krampus
{

	/// <summary>
	/// Overlay that fades the screen to or from a color. Fades in from black on every level change.
	/// </summary>
	class FadeLayer : public Layer
	{
		Engine*						world			= nullptr;
		Event<>::ListenerHandle		levelHandle;
		sf::Color					color			= sf::Color::Black;
		float						alpha			= 0.f;	// 0 = clear screen, 1 = fully covered
		float						target			= 0.f;
		float						speed			= 0.f;	// alpha per second
		float						levelFadeTime	= 0.5f;

	public:
		FadeLayer(Engine* _engine, const float& _levelFadeTime = 0.5f)
			: world(_engine), levelFadeTime(_levelFadeTime) { }

		/// <summary>Goes from the current alpha to a clear screen.</summary>
		void FadeIn(const float& _duration)
		{
			FadeTo(0.f, _duration);
		}

		/// <summary>Goes from the current alpha to a fully covered screen.</summary>
		void FadeOut(const float& _duration)
		{
			FadeTo(1.f, _duration);
		}

		void SetColor(const sf::Color& _color)
		{
			color = _color;
		}

		void OnAttach() override
		{
			levelHandle = world->GetLevelManager()->onLevelChanged.AddListener(this, &FadeLayer::OnLevelChanged);
		}

		void OnDetach() override
		{
			levelHandle.Disconnect();
		}

		void OnUpdate(const Float& _deltaTime) override
		{
			const float _step = speed * static_cast<float>(_deltaTime);
			if (alpha < target) alpha = std::min(alpha + _step, target);
			else				alpha = std::max(alpha - _step, target);
		}

		void OnRender(Window& _window) override
		{
			if (alpha <= 0.f) return;

			const sf::Vector2f _size = UseScreenView(_window);
			sf::Color _drawColor = color;
			_drawColor.a = static_cast<std::uint8_t>(alpha * 255.f);
			DrawScreenFill(_window, _size, _drawColor);
		}

	private:
		void FadeTo(const float& _target, const float& _duration)
		{
			target = _target;
			speed = _duration > 0.f ? std::abs(_target - alpha) / _duration : 1000.f;
		}

		void OnLevelChanged()
		{
			LOG(VerbosityType::Log, "Fade: new level, fading in");
			alpha = 1.f;
			FadeIn(levelFadeTime);
		}
	};
}
