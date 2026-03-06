#pragma once
#include "Utilities/Math/Vector2D.h"
#include "Core/KrampusObject.h"


namespace Krampus
{

	class Mouse : public KrampusObject
	{
		IVector2 position;
		IVector2 lastPosition;

	public:
		Mouse(Engine* _engine)
			: KrampusObject(_engine) { }

		inline IVector2 GetPosition() const noexcept
		{
			const Window& _window = GetWorld()->GetWindowRef();
			return _window.MapPixelToCoords(position);
		}
		inline IVector2 GetScreenPosition() const noexcept
		{
			return position;
		}
		inline void SetPosition(const IVector2& _position) noexcept
		{
			const Window& _window = GetWorld()->GetWindowRef();
			SetScreenPosition(_window.MapCoordsToPixel(_position, _window.GetView()));
		}
		inline void SetScreenPosition(const IVector2& _position) noexcept
		{
			sf::Mouse::setPosition(_position);
			Update();
		}
		inline bool HasMoved() const noexcept
		{
			return position != lastPosition;
		}

		inline void Show()
		{
			GetWorld()->GetWindowRef().GetRenderWindow().setMouseCursorVisible(true);
		}
		inline void Hide()
		{
			GetWorld()->GetWindowRef().GetRenderWindow().setMouseCursorVisible(false);
		}

		inline void Update()
		{
			lastPosition = position;
			position = sf::Mouse::getPosition();
		}
	};


}
