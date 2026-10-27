#pragma once
#include "Utilities/Math/Vector2D.h"
#include "Core/KrampusObject.h"

namespace Krampus
{

	/// <summary>
	/// Manages mouse input coordinates, movement detection, and cursor visibility.
	/// </summary>
	class Mouse : public KrampusObject
	{
		IVector2	position;
		IVector2	lastPosition;

	public:
		/// <summary>
		/// Constructs a Mouse input tracker associated with the engine.
		/// </summary>
		/// <param name="_engine">Pointer to the Engine world.</param>
		Mouse(Engine* _engine)
			: KrampusObject(_engine) { }

		/// <summary>
		/// Gets the mouse position mapped into world coordinates based on the active window view.
		/// </summary>
		/// <returns>Mouse position in world coordinates.</returns>
		inline IVector2 GetPosition() const noexcept
		{
			const Window& _window = GetWorld()->GetWindowRef();
			return _window.MapPixelToCoords(position);
		}

		/// <summary>
		/// Gets the raw mouse coordinates in screen/window pixels.
		/// </summary>
		/// <returns>Screen pixel coordinates.</returns>
		inline IVector2 GetScreenPosition() const noexcept
		{
			return position;
		}

		/// <summary>
		/// Sets the mouse position using world coordinates, converting them to screen pixels.
		/// </summary>
		/// <param name="_position">Target position in world coordinates.</param>
		inline void SetPosition(const IVector2& _position) noexcept
		{
			const Window& _window = GetWorld()->GetWindowRef();
			SetScreenPosition(_window.MapCoordsToPixel(_position, _window.GetView()));
		}

		/// <summary>
		/// Sets the hardware cursor position in screen coordinates and updates internal position.
		/// </summary>
		/// <param name="_position">Target position in screen pixels.</param>
		inline void SetScreenPosition(const IVector2& _position) noexcept
		{
			sf::Mouse::setPosition(_position);
			Update();
		}

		/// <summary>
		/// Checks whether the mouse position changed since the last update.
		/// </summary>
		/// <returns>True if mouse moved, false otherwise.</returns>
		inline Bool HasMoved() const noexcept
		{
			return position != lastPosition;
		}

		/// <summary>
		/// Makes the operating system mouse cursor visible within the window.
		/// </summary>
		inline void Show()
		{
			GetWorld()->GetWindowRef().GetRenderWindow().setMouseCursorVisible(true);
		}

		/// <summary>
		/// Hides the operating system mouse cursor within the window.
		/// </summary>
		inline void Hide()
		{
			GetWorld()->GetWindowRef().GetRenderWindow().setMouseCursorVisible(false);
		}

		/// <summary>
		/// Updates the tracked mouse position from the operating system.
		/// </summary>
		inline void Update()
		{
			lastPosition = position;
			position = sf::Mouse::getPosition();
		}
	};

}
