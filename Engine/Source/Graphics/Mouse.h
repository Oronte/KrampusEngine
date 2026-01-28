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

		inline IVector2 GetPosition() const noexcept
		{
			const MainWindow& _mainWindow = MAIN_WINDOW;
			return _mainWindow.MapCoordsToPixel(position, _mainWindow.GetView());
		}
		inline IVector2 GetScreenPosition() const noexcept
		{
			return position;
		}
		inline void SetPosition(const IVector2& _position) noexcept
		{
			const MainWindow& _mainWindow = MAIN_WINDOW;
			SetScreenPosition(_mainWindow.MapCoordsToPixel(_position, _mainWindow.GetView()));
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
			MAIN_WINDOW.GetRenderWindow().setMouseCursorVisible(true);
		}
		inline void Hide()
		{
			MAIN_WINDOW.GetRenderWindow().setMouseCursorVisible(false);
		}

		inline void Update()
		{
			lastPosition = position;
			position = sf::Mouse::getPosition();
		}
	};


}
