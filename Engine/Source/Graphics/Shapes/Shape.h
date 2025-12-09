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
        INLINE sf::Shape* Get() const noexcept
        {
            return shape.get();
        }

        INLINE const Texture GetTexture() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return Texture();
            }
            return Texture(*(shape->getTexture()));
        }
        INLINE void SetTexture(const Texture* _texture, const bool& _resetRect = false)
        {
            if (!shape || !_texture)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape/texture (potential fake values)");
                return;
            }
            shape->setTexture(_texture->Get(), _resetRect);
        }

        INLINE const IRect& GetTextureRect() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return IRect();
            }
            return shape->getTextureRect();
        }
        INLINE void SetTextureRect(const IRect& _rect)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setTextureRect(_rect);
        }

        INLINE Color GetFillColor() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return Color();
            }
            return shape->getFillColor();
        }
        INLINE void SetFillColor(const Color& _color)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setFillColor(_color);
        }

        INLINE void SetOutlineColor(const Color& _color)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setOutlineColor(_color);
        }
        INLINE Color GetOutlineColor() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return Color();
            }
            return shape->getOutlineColor();
        }

        INLINE void SetOutlineThickness(const float& _thickness)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setOutlineThickness(_thickness);
        }
        INLINE float GetOutlineThickness() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return float();
            }
            return shape->getOutlineThickness();
        }

        INLINE std::size_t GetPointCount() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return std::size_t();
            }
            return shape->getPointCount();
        }
        INLINE FVector2 GetPoint(const std::size_t& _index) const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getPoint(_index));
        }

        INLINE FVector2 GetGeometricCenter() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getGeometricCenter());
        }

        INLINE void SetPosition(const FVector2& _position)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setPosition(_position);
        }
        INLINE FVector2 GetPosition() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getPosition());
        }
        INLINE void Move(const FVector2& _offset)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->move(_offset);
        }

        INLINE void SetRotation(const Angle& _angle)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setRotation(_angle);
        }
        INLINE Angle GetRotation() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return Angle();
            }
            return Angle(shape->getRotation());
        }
        INLINE void Rotate(const Angle& _angle)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->rotate(_angle);
        }

        INLINE void SetScale(const FVector2& _scale)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setScale(_scale);
        }
        INLINE FVector2 GetScale() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getScale());
        }
        INLINE void Scale(const FVector2& _factor)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->scale(_factor);
        }

        INLINE void SetOrigin(const FVector2& _origin)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setOrigin(_origin);
        }
        INLINE FVector2 GetOrigin() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getOrigin());
        }

        INLINE FRect GetLocalBounds() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FRect();
            }
            return shape->getLocalBounds();
        }
        INLINE FRect GetGlobalBounds() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FRect();
            }
            return shape->getGlobalBounds();
        }

        INLINE const Transform& GetTransform() const
        {
            return Transform(
                GetOrigin(),
                GetPosition(),
                GetRotation(),
                GetScale());
        }
        INLINE void SetTransform(const Transform& _transform)
        {
            SetOrigin(_transform.origin);
            SetPosition(_transform.position);
            SetRotation(_transform.rotation);
            SetScale(_transform.scale);
        }
	};

}

