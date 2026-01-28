#pragma once
#include "Shape.h"


namespace Krampus
{

    class RectangleShape : public Shape
    {
    public:
        inline void SetSize(const FVector2& _size)
        {
            Cast<sf::RectangleShape>(shape.get())->setSize(_size);
        }

        inline FVector2 GetSize() const
        {
            return FVector2(Cast<const sf::RectangleShape>(shape.get())->getSize());
        }

        RectangleShape(const FVector2& _size);
    };

}

