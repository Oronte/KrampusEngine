#pragma once
#include "Shape.h"

namespace Krampus
{

    /// <summary>
    /// Represents a 2D rectangular geometric shape.
    /// </summary>
    class RectangleShape : public Shape
    {
    public:
        /// <summary>
        /// Sets the dimensions (width and height) of the rectangle.
        /// </summary>
        /// <param name="_size">Dimensions vector.</param>
        inline void SetSize(const FVector2& _size)
        {
            Cast<sf::RectangleShape>(shape.get())->setSize(_size);
        }

        /// <summary>
        /// Gets the dimensions of the rectangle.
        /// </summary>
        /// <returns>Dimensions vector.</returns>
        inline FVector2 GetSize() const
        {
            return FVector2(Cast<const sf::RectangleShape>(shape.get())->getSize());
        }

        /// <summary>
        /// Constructs a RectangleShape with the given size dimensions.
        /// </summary>
        /// <param name="_size">Width and height of the rectangle.</param>
        RectangleShape(const FVector2& _size);
    };

}
