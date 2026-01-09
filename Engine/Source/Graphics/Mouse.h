#pragma once
#include "Utilities/Math/Vector2D.h"
#include "Utilities/System/Singleton.h"
#include "Graphics/Window/MainWindow.h"

#define MOUSE Krampus::Mouse::GetInstance()

namespace Krampus
{

	class Mouse : public Singleton<Mouse>
	{
		IVector2 position;
		IVector2 lastPosition;

	public:
		Mouse() = default;

		INLINE IVector2 GetPosition() const noexcept
		{
			const MainWindow& _mainWindow = MAIN_WINDOW;
			return _mainWindow.MapCoordsToPixel(position, _mainWindow.GetView());
		}
		INLINE IVector2 GetScreenPosition() const noexcept
		{
			return position;
		}
		INLINE void SetPosition(const IVector2& _position) noexcept
		{
			const MainWindow& _mainWindow = MAIN_WINDOW;
			SetScreenPosition(_mainWindow.MapCoordsToPixel(_position, _mainWindow.GetView()));
		}
		INLINE void SetScreenPosition(const IVector2& _position) noexcept
		{
			sf::Mouse::setPosition(_position);
			Update();
		}
		INLINE bool HasMoved() const noexcept
		{
			return position != lastPosition;
		}

		INLINE void Show()
		{
			MAIN_WINDOW.GetRenderWindow().setMouseCursorVisible(true);
		}
		INLINE void Hide()
		{
			MAIN_WINDOW.GetRenderWindow().setMouseCursorVisible(false);
		}

		INLINE void Update()
		{
			lastPosition = position;
			position = sf::Mouse::getPosition();
		}
	};


}
