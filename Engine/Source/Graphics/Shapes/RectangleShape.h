#pragma once
#include "Shape.h"


namespace Krampus
{

    class RectangleShape : public Shape
    {
    public:
        inlin void SetSize(const FVector2& _size)
        {
            Cast<sf::RectangleShape>(shape.get())->setSize(_size);
        }

        inlin FVector2 GetSize() const
        {
            return FVector2(Cast<const sf::RectangleShape>(shape.get())->getSize());
        }

        RectangleShape(const FVector2& _size);
    };

}

