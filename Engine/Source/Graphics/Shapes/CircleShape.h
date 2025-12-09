#pragma once
#include "Shape.h"


namespace Krampus
{

	class CircleShape : public Shape
	{
	public:
        INLINE void SetRadius(const float& _radius)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            Cast<sf::CircleShape>(shape.get())->setRadius(_radius);
        }
        INLINE float GetRadius() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return float();
            }
            return Cast<const sf::CircleShape>(shape.get())->getRadius();
        }

        INLINE void SetPointCount(const std::size_t& _count)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            Cast<sf::CircleShape>(shape.get())->setPointCount(_count);
        }
        INLINE std::size_t GetPointCount() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return std::size_t();
            }
            return Cast<const sf::CircleShape>(shape.get())->getPointCount();
        }

		explicit CircleShape(const float& _radius = 0, const std::size_t& _pointCount = 30);
	};

}