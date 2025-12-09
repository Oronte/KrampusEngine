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
		virtual INLINE sf::RenderWindow& GetRenderWindow() noexcept
		{
			return window;
		}
		virtual INLINE bool IsOpen() const noexcept
		{
			return window.isOpen();
		}
		virtual INLINE void SetFramerateLimit(const unsigned int& _limit)
		{
			window.setFramerateLimit(_limit);
		}
		virtual INLINE void SetVerticalSyncEnabled(const bool& _enabled)
		{
			window.setVerticalSyncEnabled(_enabled);
		}
		virtual INLINE void SetTitle(const std::string& _title)
		{
			window.setTitle(sf::String(_title.c_str()));
		}
		virtual INLINE void SetSize(const UVector2& _size)
		{
			window.setSize(_size);
		}
		virtual INLINE UVector2 GetSize() const noexcept
		{
			return UVector2(window.getSize());
		}
		virtual INLINE void SetPosition(const IVector2& _position)
		{
			window.setPosition(_position);
		}
		virtual INLINE IVector2 GetPosition() const
		{
			return IVector2(window.getPosition());
		}
		virtual INLINE void SetIcon(const Image& _image)
		{
			window.setIcon(_image);
		}
		virtual INLINE bool HasFocus() const
		{
			return window.hasFocus();
		}
		virtual INLINE void RequestFocus()
		{
			window.requestFocus();
		}
		virtual void SetView(const CameraComponent* _camera);

		Window() = default;

		virtual void Create(const std::string& _name, const UVector2& _windowSize, const bool& _isFullscreen);
		virtual void Clear(const Color& _color);
		virtual void Display();
		virtual void Close();

		virtual std::optional<sf::Event> PollEvent();

		virtual void Draw(const sf::Drawable& _drawable);
		virtual void Draw(const ShapeObject& _drawable);
	};

}