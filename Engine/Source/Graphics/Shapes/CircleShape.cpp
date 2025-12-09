#include "CircleShape.h"

Krampus::CircleShape::CircleShape(const float& _radius, const std::size_t& _pointCount)
{
	shape = std::make_unique<sf::CircleShape>(_radius, _pointCount);
}
