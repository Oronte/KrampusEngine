#pragma once
#include "Utilities/Math/Transform.h"
#include "Graphics/Ressources/Texture.h"

namespace Krampus
{

	/// <summary>
	/// Base class wrapping 2D geometric drawable shapes with transform, outline, fill, and texture properties.
	/// </summary>
	class Shape
	{
	protected:
		std::unique_ptr<sf::Shape>  shape;

	public:
		/// <summary>
		/// Virtual destructor for Shape.
		/// </summary>
        virtual ~Shape() = default;

		/// <summary>
		/// Gets a raw pointer to the underlying SFML shape.
		/// </summary>
		/// <returns>Pointer to sf::Shape.</returns>
        inline sf::Shape* Get() const noexcept
        {
            return shape.get();
        }

		/// <summary>
		/// Gets the texture applied to this shape.
		/// </summary>
		/// <returns>Texture instance.</returns>
        inline const Texture& GetTexture() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return Texture();
            }
            return Texture(*(shape->getTexture()));
        }

		/// <summary>
		/// Assigns a texture to the shape.
		/// </summary>
		/// <param name="_texture">Pointer to Texture.</param>
		/// <param name="_resetRect">Whether to reset texture rect to full dimensions.</param>
        inline void SetTexture(const Texture* _texture, const Bool& _resetRect = false)
        {
            if (!shape || !_texture)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape/texture (potential fake values)");
                return;
            }
            shape->setTexture(_texture->Get(), _resetRect);
        }

		/// <summary>
		/// Removes the texture: the shape is drawn with its fill color only.
		/// </summary>
        inline void ClearTexture()
        {
            if (shape) shape->setTexture(nullptr);
        }

		/// <summary>
		/// Gets the sub-rectangle of the texture mapped onto the shape.
		/// </summary>
		/// <returns>Texture rectangle (a copy: SFML gives it by value).</returns>
        inline IRect GetTextureRect() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return IRect();
            }
            return shape->getTextureRect();
        }

		/// <summary>
		/// Sets the sub-rectangle of the texture mapped onto the shape.
		/// </summary>
		/// <param name="_rect">Texture coordinates sub-rectangle.</param>
        inline void SetTextureRect(const IRect& _rect)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setTextureRect(_rect);
        }

		/// <summary>
		/// Gets the fill color of the shape.
		/// </summary>
		/// <returns>Fill color.</returns>
        inline Color GetFillColor() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return Color();
            }
            return shape->getFillColor();
        }

		/// <summary>
		/// Sets the fill color of the shape interior.
		/// </summary>
		/// <param name="_color">Fill color.</param>
        inline void SetFillColor(const Color& _color)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setFillColor(_color);
        }

		/// <summary>
		/// Sets the outline color of the shape perimeter.
		/// </summary>
		/// <param name="_color">Outline color.</param>
        inline void SetOutlineColor(const Color& _color)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setOutlineColor(_color);
        }

		/// <summary>
		/// Gets the outline color of the shape perimeter.
		/// </summary>
		/// <returns>Outline color.</returns>
        inline Color GetOutlineColor() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return Color();
            }
            return shape->getOutlineColor();
        }

		/// <summary>
		/// Sets the thickness of the outline border in pixels.
		/// </summary>
		/// <param name="_thickness">Outline border thickness.</param>
        inline void SetOutlineThickness(const Float& _thickness)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setOutlineThickness(_thickness);
        }

		/// <summary>
		/// Gets the thickness of the outline border in pixels.
		/// </summary>
		/// <returns>Outline border thickness.</returns>
        inline Float GetOutlineThickness() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return float();
            }
            return shape->getOutlineThickness();
        }

		/// <summary>
		/// Gets the total number of polygon points defining the shape.
		/// </summary>
		/// <returns>Point count.</returns>
        inline Int GetPointCount() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return std::size_t();
            }
            return shape->getPointCount();
        }

		/// <summary>
		/// Gets the local coordinate of a point by index.
		/// </summary>
		/// <param name="_index">Point index.</param>
		/// <returns>Point coordinates.</returns>
        inline FVector2 GetPoint(const Int& _index) const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getPoint(_index));
        }

		/// <summary>
		/// Computes the geometric centroid center of the shape points.
		/// </summary>
		/// <returns>Center coordinates.</returns>
        inline FVector2 GetGeometricCenter() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getGeometricCenter());
        }

		/// <summary>
		/// Sets the position of the shape in 2D space.
		/// </summary>
		/// <param name="_position">Position coordinates.</param>
        inline void SetPosition(const FVector2& _position)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setPosition(_position);
        }

		/// <summary>
		/// Gets the position of the shape in 2D space.
		/// </summary>
		/// <returns>Position coordinates.</returns>
        inline FVector2 GetPosition() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getPosition());
        }

		/// <summary>
		/// Moves the shape by a 2D offset vector.
		/// </summary>
		/// <param name="_offset">Offset to translate.</param>
        inline void Move(const FVector2& _offset)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->move(_offset);
        }

		/// <summary>
		/// Sets the rotation angle of the shape.
		/// </summary>
		/// <param name="_angle">Rotation angle.</param>
        inline void SetRotation(const Angle& _angle)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setRotation(_angle);
        }

		/// <summary>
		/// Gets the rotation angle of the shape.
		/// </summary>
		/// <returns>Rotation angle.</returns>
        NO_DISCARD inline Angle GetRotation() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return Angle();
            }
            return Angle(shape->getRotation());
        }

		/// <summary>
		/// Rotates the shape by an angular offset.
		/// </summary>
		/// <param name="_angle">Angle offset.</param>
        inline void Rotate(const Angle& _angle)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->rotate(_angle);
        }

		/// <summary>
		/// Sets the 2D scale factor of the shape.
		/// </summary>
		/// <param name="_scale">Scale multipliers.</param>
        inline void SetScale(const FVector2& _scale)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setScale(_scale);
        }

		/// <summary>
		/// Gets the 2D scale factor of the shape.
		/// </summary>
		/// <returns>Scale multipliers.</returns>
        NO_DISCARD inline FVector2 GetScale() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getScale());
        }

		/// <summary>
		/// Scales the shape by multiplying current scale with factor.
		/// </summary>
		/// <param name="_factor">Scale factor.</param>
        inline void Scale(const FVector2& _factor)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->scale(_factor);
        }

		/// <summary>
		/// Sets the local origin (pivot point) of the shape.
		/// </summary>
		/// <param name="_origin">Origin coordinates.</param>
        inline void SetOrigin(const FVector2& _origin)
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return;
            }
            shape->setOrigin(_origin);
        }

		/// <summary>
		/// Centers the origin point at the geometric centroid of the shape.
		/// </summary>
        inline void SetOriginAtMiddle()
        {
            SetOrigin(GetGeometricCenter());
        }

		/// <summary>
		/// Gets the local origin (pivot point) of the shape.
		/// </summary>
		/// <returns>Origin coordinates.</returns>
        NO_DISCARD inline FVector2 GetOrigin() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FVector2();
            }
            return FVector2(shape->getOrigin());
        }

		/// <summary>
		/// Gets the local bounding box of the shape without transformations.
		/// </summary>
		/// <returns>Local bounds rectangle.</returns>
        NO_DISCARD inline FRect GetLocalBounds() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FRect();
            }
            return shape->getLocalBounds();
        }

		/// <summary>
		/// Gets the global bounding box of the shape in world space.
		/// </summary>
		/// <returns>Global bounds rectangle.</returns>
        NO_DISCARD inline FRect GetGlobalBounds() const
        {
            if (!shape)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize shape (potential fake values)");
                return FRect();
            }
            return shape->getGlobalBounds();
        }

		/// <summary>
		/// Gets the combined Transform structure (position, rotation, scale).
		/// </summary>
		/// <returns>Transform structure.</returns>
        NO_DISCARD inline Transform GetTransform() const
        {
            return Transform(
                GetPosition(),
                GetRotation(),
                GetScale());
        }

		/// <summary>
		/// Applies a combined Transform structure to the shape.
		/// </summary>
		/// <param name="_transform">Transform structure.</param>
        inline void SetTransform(const Transform& _transform)
        {
            SetPosition(_transform.position);
            SetRotation(_transform.rotation);
            SetScale(_transform.scale);
        }
	};

}
