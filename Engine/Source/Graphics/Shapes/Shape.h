#pragma once
#include "Utilities/Math/Transform.h"
#include "Graphics/Ressources/Texture.h"

namespace Krampus
{

	class Shape
	{
	protected:
		std::unique_ptr<sf::Shape> shape;

	public:
        inlin sf::Shape* Get() const noexcept
        {
            return shape.get();
        }

        inlin const Texture GetTexture() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return Texture();
            }
            return Texture(*(shape->getTexture()));
        }
        inlin void SetTexture(const Texture* _texture, const bool& _resetRect = false)
        {
            if (!shape || !_texture)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape/texture (potential fake values)");
                return;
            }
            shape->setTexture(_texture->Get(), _resetRect);
        }

        inlin const IRect& GetTextureRect() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return IRect();
            }
            return shape->getTextureRect();
        }
        inlin void SetTextureRect(const IRect& _rect)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setTextureRect(_rect);
        }

        inlin Color GetFillColor() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return Color();
            }
            return shape->getFillColor();
        }
        inlin void SetFillColor(const Color& _color)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setFillColor(_color);
        }

        inlin void SetOutlineColor(const Color& _color)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setOutlineColor(_color);
        }
        inlin Color GetOutlineColor() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return Color();
            }
            return shape->getOutlineColor();
        }

        inlin void SetOutlineThickness(const float& _thickness)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setOutlineThickness(_thickness);
        }
        inlin float GetOutlineThickness() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return float();
            }
            return shape->getOutlineThickness();
        }

        inlin std::size_t GetPointCount() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return std::size_t();
            }
            return shape->getPointCount();
        }
        inlin FVector2 GetPoint(const std::size_t& _index) const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getPoint(_index));
        }

        inlin FVector2 GetGeometricCenter() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getGeometricCenter());
        }

        inlin void SetPosition(const FVector2& _position)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setPosition(_position);
        }
        inlin FVector2 GetPosition() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getPosition());
        }
        inlin void Move(const FVector2& _offset)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->move(_offset);
        }

        inlin void SetRotation(const Angle& _angle)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setRotation(_angle);
        }
        inlin Angle GetRotation() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return Angle();
            }
            return Angle(shape->getRotation());
        }
        inlin void Rotate(const Angle& _angle)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->rotate(_angle);
        }

        inlin void SetScale(const FVector2& _scale)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setScale(_scale);
        }
        inlin FVector2 GetScale() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getScale());
        }
        inlin void Scale(const FVector2& _factor)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->scale(_factor);
        }

        inlin void SetOrigin(const FVector2& _origin)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setOrigin(_origin);
        }
        inlin void SetOriginAtMiddle()
        {
            SetOrigin(GetGeometricCenter());
        }
        inlin FVector2 GetOrigin() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getOrigin());
        }

        inlin FRect GetLocalBounds() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FRect();
            }
            return shape->getLocalBounds();
        }
        inlin FRect GetGlobalBounds() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FRect();
            }
            return shape->getGlobalBounds();
        }

        inlin const Transform& GetTransform() const
        {
            return Transform(
                GetPosition(),
                GetRotation(),
                GetScale());
        }
        inlin void SetTransform(const Transform& _transform)
        {
            SetPosition(_transform.position);
            SetRotation(_transform.rotation);
            SetScale(_transform.scale);
        }
	};

}

