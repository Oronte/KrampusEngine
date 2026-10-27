#pragma once
#include "Shape.h"

namespace Krampus
{

	/// <summary>
	/// Represents a 2D circular geometric shape.
	/// </summary>
	class CircleShape : public Shape
	{
	public:
		/// <summary>
		/// Sets the radius of the circle shape.
		/// </summary>
		/// <param name="_radius">Radius value.</param>
        inline void SetRadius(const Float& _radius)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            Cast<sf::CircleShape>(shape.get())->setRadius(_radius);
        }

		/// <summary>
		/// Gets the radius of the circle shape.
		/// </summary>
		/// <returns>Radius value.</returns>
        inline Float GetRadius() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return float();
            }
            return Cast<const sf::CircleShape>(shape.get())->getRadius();
        }

		/// <summary>
		/// Sets the number of polygon points approximating the circle circumference.
		/// </summary>
		/// <param name="_count">Point count.</param>
        inline void SetPointCount(const Int& _count)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            Cast<sf::CircleShape>(shape.get())->setPointCount(_count);
        }

		/// <summary>
		/// Gets the number of points approximating the circle circumference.
		/// </summary>
		/// <returns>Point count.</returns>
        inline Int GetPointCount() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return std::size_t();
            }
            return Cast<const sf::CircleShape>(shape.get())->getPointCount();
        }

		/// <summary>
		/// Constructs a CircleShape with optional radius and point count.
		/// </summary>
		/// <param name="_radius">Radius of the circle.</param>
		/// <param name="_pointCount">Number of polygon points.</param>
		CircleShape(const Float& _radius = 0, const Int& _pointCount = 30);
	};

}
