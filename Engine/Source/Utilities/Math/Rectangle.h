#pragma once
#include "Vector2D.h"

namespace Krampus
{
	template<typename T>
	class Rectangle : public IPrintable
	{
		static_assert(std::is_arithmetic_v<T>, "Rectangle<T> requires an arithmetic type T");

		sf::Rect<T> rect;

	public:
		inline constexpr Vector2D<T> GetPosition() const noexcept
		{
			return Vector2D<T>(rect.position);
		}
		inline void SetPosition(const Vector2D<T>& _position) noexcept
		{
			rect.position = _position;
		}
		inline constexpr Vector2D<T> GetSize() const noexcept
		{
			return Vector2D<T>(rect.size);
		}
		inline void SetSize(const Vector2D<T>& _position) noexcept
		{
			rect.size = _position;
		}
		inline constexpr Vector2D<T> GetCenter() const
		{
			return Vector2D<T>(rect.getCenter());
		}

		Rectangle() = default;
		Rectangle(const Vector2D<T>& _position, const Vector2D<T>& _size)
		{
			rect = sf::Rect<T>(_position, _size);
		}
		Rectangle(const sf::Rect<T>& _rect)
		{
			rect = _rect;
		}

		inline constexpr bool Contains(const Vector2D<T>& _point) const
		{
			return rect.contains(_point);
		}
		inline constexpr std::optional<Rectangle<T>> FindIntersection(const Rectangle<T> _rect) const
		{
			return rect.findIntersection(_rect);
		}

		inline bool operator == (const Rectangle<T> _other)
		{
			return _other.rect == rect;
		}

		inline operator sf::Rect<T>() const noexcept
		{
			return rect;
		}

		inline virtual std::string ToString() const override
		{
			return std::format("Position : {} , Size : {} , Center : {}",
				GetPosition().ToString(), 
				GetPosition().ToString(), 
				GetCenter().ToString());
		}
	};

	using IRect = Krampus::Rectangle<int>;
	using FRect = Krampus::Rectangle<float>;
}



