#include "CircleShape.h"

Krampus::CircleShape::CircleShape(const Float& _radius, const Int& _pointCount)
{
	shape = std::make_unique<sf::CircleShape>((float)_radius, (std::size_t)_pointCount);
}
