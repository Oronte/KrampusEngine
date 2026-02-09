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
        virtual ~Shape() = default;

        inline sf::Shape* Get() const noexcept
        {
            return shape.get();
        }

        inline const Texture& GetTexture() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return Texture();
            }
            return Texture(*(shape->getTexture()));
        }
        inline void SetTexture(const Texture* _texture, const bool& _resetRect = false)
        {
            if (!shape || !_texture)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape/texture (potential fake values)");
                return;
            }
            shape->setTexture(_texture->Get(), _resetRect);
        }

        inline const IRect& GetTextureRect() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return IRect();
            }
            return shape->getTextureRect();
        }
        inline void SetTextureRect(const IRect& _rect)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setTextureRect(_rect);
        }

        inline Color GetFillColor() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return Color();
            }
            return shape->getFillColor();
        }
        inline void SetFillColor(const Color& _color)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setFillColor(_color);
        }

        inline void SetOutlineColor(const Color& _color)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setOutlineColor(_color);
        }
        inline Color GetOutlineColor() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return Color();
            }
            return shape->getOutlineColor();
        }

        inline void SetOutlineThickness(const float& _thickness)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setOutlineThickness(_thickness);
        }
        inline float GetOutlineThickness() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return float();
            }
            return shape->getOutlineThickness();
        }

        inline std::size_t GetPointCount() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return std::size_t();
            }
            return shape->getPointCount();
        }
        inline FVector2 GetPoint(const std::size_t& _index) const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getPoint(_index));
        }

        inline FVector2 GetGeometricCenter() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getGeometricCenter());
        }

        inline void SetPosition(const FVector2& _position)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setPosition(_position);
        }
        inline FVector2 GetPosition() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getPosition());
        }
        inline void Move(const FVector2& _offset)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->move(_offset);
        }

        inline void SetRotation(const Angle& _angle)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setRotation(_angle);
        }
        inline Angle GetRotation() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return Angle();
            }
            return Angle(shape->getRotation());
        }
        inline void Rotate(const Angle& _angle)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->rotate(_angle);
        }

        inline void SetScale(const FVector2& _scale)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setScale(_scale);
        }
        inline FVector2 GetScale() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getScale());
        }
        inline void Scale(const FVector2& _factor)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->scale(_factor);
        }

        inline void SetOrigin(const FVector2& _origin)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setOrigin(_origin);
        }
        inline void SetOriginAtMiddle()
        {
            SetOrigin(GetGeometricCenter());
        }
        inline FVector2 GetOrigin() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getOrigin());
        }

        inline FRect GetLocalBounds() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FRect();
            }
            return shape->getLocalBounds();
        }
        inline FRect GetGlobalBounds() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FRect();
            }
            return shape->getGlobalBounds();
        }

        inline const Transform& GetTransform() const
        {
            return Transform(
                GetPosition(),
                GetRotation(),
                GetScale());
        }
        inline void SetTransform(const Transform& _transform)
        {
            SetPosition(_transform.position);
            SetRotation(_transform.rotation);
            SetScale(_transform.scale);
        }
	};

}

