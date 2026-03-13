#pragma once
#include "Shape.h"


namespace Krampus
{

	class CircleShape : public Shape
	{
	public:
        inline void SetRadius(const Float& _radius)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            Cast<sf::CircleShape>(shape.get())->setRadius(_radius);
        }
        inline Float GetRadius() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return float();
            }
            return Cast<const sf::CircleShape>(shape.get())->getRadius();
        }

        inline void SetPointCount(const Int& _count)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            Cast<sf::CircleShape>(shape.get())->setPointCount(_count);
        }
        inline Int GetPointCount() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return std::size_t();
            }
            return Cast<const sf::CircleShape>(shape.get())->getPointCount();
        }

		CircleShape(const Float& _radius = 0, const Int& _pointCount = 30);
	};

}