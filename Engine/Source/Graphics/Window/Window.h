#pragma once
#include "Core/CoreMinimal.h"
#include "Graphics/Ressources/Image.h"


namespace Krampus
{
	
	class ShapeObject;
	class CameraComponent;
	class Vertices;

	class Window
	{
		sf::RenderWindow window;

	public:
		Event<sf::View> onViewChange;

	public:
		virtual inlin sf::RenderWindow& GetRenderWindow() noexcept
		{
			return window;
		}
		virtual inlin bool IsOpen() const noexcept
		{
			return window.isOpen();
		}
		virtual inlin void SetFramerateLimit(const unsigned int& _limit)
		{
			window.setFramerateLimit(_limit);
		}
		virtual inlin void SetVerticalSyncEnabled(const bool& _enabled)
		{
			window.setVerticalSyncEnabled(_enabled);
		}
		virtual inlin void SetTitle(const std::string& _title)
		{
			window.setTitle(sf::String(_title.c_str()));
		}
		virtual inlin void SetSize(const UVector2& _size)
		{
			window.setSize(_size);
		}
		virtual inlin UVector2 GetSize() const noexcept
		{
			return UVector2(window.getSize());
		}
		virtual inlin void SetPosition(const IVector2& _position)
		{
			window.setPosition(_position);
		}
		virtual inlin IVector2 GetPosition() const
		{
			return IVector2(window.getPosition());
		}
		virtual inlin void SetIcon(const Image& _image)
		{
			window.setIcon(_image);
		}
		virtual inlin bool HasFocus() const
		{
			return window.hasFocus();
		}
		virtual inlin void RequestFocus()
		{
			window.requestFocus();
		}
		virtual inlin sf::View GetView() const
		{
			return window.getView();
		}
		inlin IVector2 MapCoordsToPixel(const FVector2& _worldPos) const
		{
			return window.mapCoordsToPixel(_worldPos);
		}
		inlin IVector2 MapCoordsToPixel(const FVector2& _worldPos, const sf::View& _view) const
		{
			return window.mapCoordsToPixel(_worldPos, _view);
		}
		inlin FVector2 MapPixelToCoords(const IVector2& _screenPos) const
		{
			return window.mapPixelToCoords(_screenPos);
		}
		inlin FVector2 MapPixelToCoords(const IVector2& _screenPos, const sf::View& _view) const
		{
			return window.mapPixelToCoords(_screenPos, _view);
		}

		virtual void SetView(const CameraComponent* _camera);

		Window() = default;

		virtual void Create(const std::string& _name, const UVector2& _windowSize);
		virtual void Clear(const Color& _color);
		virtual void Display();
		virtual void Close();

		virtual std::optional<sf::Event> PollEvent();

		virtual void Draw(const sf::Drawable& _drawable);
		virtual void Draw(const ShapeObject& _drawable);
	};

}