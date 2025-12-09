#pragma once
#include "Shape.h"


namespace Krampus
{

    class RectangleShape : public Shape
    {
    public:
        INLINE void SetSize(const FVector2& _size)
        {
            Cast<sf::RectangleShape>(shape.get())->setSize(_size);
        }

        INLINE FVector2 GetSize() const
        {
            return FVector2(Cast<const sf::RectangleShape>(shape.get())->getSize());
        }

        RectangleShape(const FVector2& _size);
    };

}

