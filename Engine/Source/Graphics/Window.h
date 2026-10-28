#pragma once
#include "Graphics/Ressources/Image.h"
#include "Utilities/System/Event.h"

namespace Krampus
{
	
	class ShapeObject;
	class CameraComponent;
	class Vertices;

	/// <summary>
	/// Manages the OS application window, rendering context, display loop, and view transformations.
	/// </summary>
	class Window
	{
		sf::RenderWindow	ownWindow;
		sf::RenderWindow*	window	= &ownWindow;	// points to ownWindow, or to a window owned by the host application
		sf::FloatRect		mouseViewport;				// zero size = the game fills the window

	public:
		/// <summary>
		/// Event triggered whenever the active camera view changes.
		/// </summary>
		Event<sf::View>		onViewChange;

	public:
		/// <summary>
		/// Gets a reference to the underlying SFML RenderWindow.
		/// </summary>
		/// <returns>Reference to sf::RenderWindow.</returns>
		virtual inline sf::RenderWindow& GetRenderWindow() noexcept
		{
			return *window;
		}

		/// <summary>
		/// Checks whether the window is currently open.
		/// </summary>
		/// <returns>True if open, false if closed.</returns>
		virtual inline Bool IsOpen() const noexcept
		{
			return window->isOpen();
		}

		/// <summary>
		/// Sets the target framerate limit in frames per second.
		/// </summary>
		/// <param name="_limit">Framerate limit (0 for unlimited).</param>
		virtual inline void SetFramerateLimit(const UInt& _limit)
		{
			window->setFramerateLimit(_limit);
		}

		/// <summary>
		/// Enables or disables vertical synchronization (VSync).
		/// </summary>
		/// <param name="_enabled">True to enable VSync, false to disable.</param>
		virtual inline void SetVerticalSyncEnabled(const Bool& _enabled)
		{
			window->setVerticalSyncEnabled(_enabled);
		}

		/// <summary>
		/// Sets the OS window title bar text.
		/// </summary>
		/// <param name="_title">Title string.</param>
		virtual inline void SetTitle(const String& _title)
		{
			window->setTitle(sf::String(_title.ToCharPtr()));
		}

		/// <summary>
		/// Sets the client size of the window in pixels.
		/// </summary>
		/// <param name="_size">Width and height in pixels.</param>
		virtual inline void SetSize(const UVector2& _size)
		{
			window->setSize(_size);
		}

		/// <summary>
		/// Gets the client size of the window in pixels.
		/// </summary>
		/// <returns>Dimensions as UVector2.</returns>
		virtual inline UVector2 GetSize() const noexcept
		{
			return UVector2(window->getSize());
		}

		/// <summary>
		/// Sets the desktop position of the window in pixels.
		/// </summary>
		/// <param name="_position">Desktop screen coordinates.</param>
		virtual inline void SetPosition(const IVector2& _position)
		{
			window->setPosition(_position);
		}

		/// <summary>
		/// Gets the desktop position of the window in pixels.
		/// </summary>
		/// <returns>Desktop screen coordinates.</returns>
		virtual inline IVector2 GetPosition() const
		{
			return IVector2(window->getPosition());
		}

		/// <summary>
		/// Gets the center coordinate of the window on the desktop.
		/// </summary>
		/// <returns>Center desktop coordinates.</returns>
		virtual inline IVector2 GetCenter() const
		{
			return GetPosition() + GetSize() / 2;
		}

		/// <summary>
		/// Sets the OS window application icon.
		/// </summary>
		/// <param name="_image">Source Image containing pixel data.</param>
		virtual inline void SetIcon(const Image& _image)
		{
			window->setIcon(_image);
		}

		/// <summary>
		/// Checks whether the window currently has keyboard/input focus.
		/// </summary>
		/// <returns>True if focused, false otherwise.</returns>
		virtual inline Bool HasFocus() const
		{
			return window->hasFocus();
		}

		/// <summary>
		/// Requests the operating system to give input focus to this window.
		/// </summary>
		virtual inline void RequestFocus()
		{
			window->requestFocus();
		}

		/// <summary>
		/// Gets the current active SFML view being rendered.
		/// </summary>
		/// <returns>Active sf::View.</returns>
		virtual inline sf::View GetView() const
		{
			return window->getView();
		}

		/// <summary>
		/// Converts world coordinates to window pixel coordinates using the current view.
		/// </summary>
		/// <param name="_worldPos">Position in world space.</param>
		/// <returns>Pixel position.</returns>
		inline IVector2 MapCoordsToPixel(const FVector2& _worldPos) const
		{
			return window->mapCoordsToPixel(_worldPos);
		}

		/// <summary>
		/// Converts world coordinates to window pixel coordinates using a specific view.
		/// </summary>
		/// <param name="_worldPos">Position in world space.</param>
		/// <param name="_view">Specific view to map through.</param>
		/// <returns>Pixel position.</returns>
		inline IVector2 MapCoordsToPixel(const FVector2& _worldPos, const sf::View& _view) const
		{
			return window->mapCoordsToPixel(_worldPos, _view);
		}

		/// <summary>
		/// Converts window pixel coordinates to world space coordinates using the current view.
		/// </summary>
		/// <param name="_screenPos">Pixel coordinates in window.</param>
		/// <returns>World coordinates.</returns>
		inline FVector2 MapPixelToCoords(const IVector2& _screenPos) const
		{
			return window->mapPixelToCoords(_screenPos);
		}

		/// <summary>
		/// Converts window pixel coordinates to world space coordinates using a specific view.
		/// </summary>
		/// <param name="_screenPos">Pixel coordinates in window.</param>
		/// <param name="_view">Specific view to map through.</param>
		/// <returns>World coordinates.</returns>
		inline FVector2 MapPixelToCoords(const IVector2& _screenPos, const sf::View& _view) const
		{
			return window->mapPixelToCoords(_screenPos, _view);
		}

		/// <summary>
		/// Applies a new view for subsequent rendering operations.
		/// </summary>
		/// <param name="_view">The view to activate.</param>
		virtual void SetView(sf::View _view);

		/// <summary>
		/// Applies the view from a CameraComponent for subsequent rendering.
		/// </summary>
		/// <param name="_camera">Pointer to the CameraComponent.</param>
		virtual void SetView(const CameraComponent* _camera);

		/// <summary>
		/// Default constructor for Window.
		/// </summary>
		Window() = default;

		/// <summary>
		/// Creates and opens a window with the given title and client dimensions.
		/// </summary>
		/// <param name="_name">Title of the window.</param>
		/// <param name="_windowSize">Client dimensions in pixels.</param>
		virtual void Create(const String& _name, const UVector2& _windowSize);

		/// <summary>
		/// Creates and opens a window with default dimensions and the given title.
		/// </summary>
		/// <param name="_name">Title of the window.</param>
		virtual void Create(const String& _name = "KrampusEngine");

		/// <summary>
		/// Uses a window created and owned by the host application. Close() then leaves it open.
		/// The window must outlive this object.
		/// </summary>
		/// <param name="_hostWindow">Already opened window.</param>
		inline void Attach(sf::RenderWindow* _hostWindow)
		{
			LOG_MSG("Window: using the window of the host application");
			window = _hostWindow;
		}

		/// <summary>
		/// Declares the region of the window (in window pixels) that shows the game, e.g. the editor Viewport.
		/// GetMousePosition then maps that region to the full window size, as if the game filled the window.
		/// </summary>
		/// <param name="_region">Region in window pixels.</param>
		inline void SetMouseViewport(const sf::FloatRect& _region)
		{
			mouseViewport = _region;
		}

		/// <summary>
		/// The game fills the window again.
		/// </summary>
		inline void ResetMouseViewport()
		{
			mouseViewport = sf::FloatRect();
		}

		/// <summary>
		/// Gets the mouse position in game window pixels (relative to the window, corrected by the mouse viewport).
		/// </summary>
		IVector2 GetMousePosition() const;

		/// <summary>
		/// Moves the OS cursor to a position given in game window pixels (inverse of GetMousePosition).
		/// </summary>
		void SetMousePosition(const IVector2& _position);

		/// <summary>
		/// Clears the window render target with a background color.
		/// </summary>
		/// <param name="_color">Background clear color.</param>
		virtual void Clear(const Color& _color);

		/// <summary>
		/// Presents the back buffer to the screen.
		/// </summary>
		virtual void Display();

		/// <summary>
		/// Closes the window and destroys its OpenGL context.
		/// </summary>
		virtual void Close();

		/// <summary>
		/// Polls for pending OS window and input events.
		/// </summary>
		/// <returns>Optional sf::Event containing event data if available.</returns>
		virtual std::optional<sf::Event> PollEvent();

		/// <summary>
		/// Renders an SFML drawable primitive into the window.
		/// </summary>
		/// <param name="_drawable">Drawable object.</param>
		virtual void Draw(const sf::Drawable& _drawable);

		/// <summary>
		/// Renders a Krampus ShapeObject into the window.
		/// </summary>
		/// <param name="_drawable">ShapeObject to draw.</param>
		virtual void Draw(const ShapeObject& _drawable);
	};

}
