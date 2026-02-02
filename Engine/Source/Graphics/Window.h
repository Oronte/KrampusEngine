#pragma once
#include "Graphics/Ressources/Image.h"
#include "Utilities/System/Event.h"


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
		virtual inline sf::RenderWindow& GetRenderWindow() noexcept
		{
			return window;
		}
		virtual inline bool IsOpen() const noexcept
		{
			return window.isOpen();
		}
		virtual inline void SetFramerateLimit(const unsigned int& _limit)
		{
			window.setFramerateLimit(_limit);
		}
		virtual inline void SetVerticalSyncEnabled(const bool& _enabled)
		{
			window.setVerticalSyncEnabled(_enabled);
		}
		virtual inline void SetTitle(const std::string& _title)
		{
			window.setTitle(sf::String(_title.c_str()));
		}
		virtual inline void SetSize(const UVector2& _size)
		{
			window.setSize(_size);
		}
		virtual inline UVector2 GetSize() const noexcept
		{
			return UVector2(window.getSize());
		}
		virtual inline void SetPosition(const IVector2& _position)
		{
			window.setPosition(_position);
		}
		virtual inline IVector2 GetPosition() const
		{
			return IVector2(window.getPosition());
		}
		virtual inline void SetIcon(const Image& _image)
		{
			window.setIcon(_image);
		}
		virtual inline bool HasFocus() const
		{
			return window.hasFocus();
		}
		virtual inline void RequestFocus()
		{
			window.requestFocus();
		}
		virtual inline sf::View GetView() const
		{
			return window.getView();
		}
		inline IVector2 MapCoordsToPixel(const FVector2& _worldPos) const
		{
			return window.mapCoordsToPixel(_worldPos);
		}
		inline IVector2 MapCoordsToPixel(const FVector2& _worldPos, const sf::View& _view) const
		{
			return window.mapCoordsToPixel(_worldPos, _view);
		}
		inline FVector2 MapPixelToCoords(const IVector2& _screenPos) const
		{
			return window.mapPixelToCoords(_screenPos);
		}
		inline FVector2 MapPixelToCoords(const IVector2& _screenPos, const sf::View& _view) const
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