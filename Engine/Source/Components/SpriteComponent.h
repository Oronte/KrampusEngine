#pragma once
#include "Component.h"
#include "Graphics/Shapes/ShapeObject.h"
#include "Utilities/System/Debuggable.h"
#include "Utilities/System/Drawable.h"

namespace Krampus
{
	class Window;
	class StainComponent;

	/// <summary>
	/// Component that renders 2D shapes (rectangles, circles) with textures and colors attached to an actor.
	/// </summary>
	class SpriteComponent : public Component, public IDebuggable, public IDrawable
	{
		std::unique_ptr<ShapeObject>	shape;

		Event<FVector2>::ListenerHandle onOwnerMoveHandle;
		Event<Angle>::ListenerHandle onOwnerRotateHandle;
		Event<FVector2>::ListenerHandle onOwnerScaleHandle;
		StainComponent*					stainLayer	= nullptr;	// marks drawn over the sprite (StainComponent)

	public:
		/// <summary>The StainComponent whose marks are drawn right after the sprite (it sets itself).</summary>
		void SetStainLayer(StainComponent* _stains) { stainLayer = _stains; }
		StainComponent* GetStainLayer() const { return stainLayer; }

		/// <summary>
		/// Gets a pointer to the internal ShapeObject managing rendering primitives.
		/// </summary>
		/// <returns>Pointer to ShapeObject.</returns>
		inline ShapeObject* GetShapeObject()
		{
			return shape.get();
		}

		inline const ShapeObject* GetShapeObject() const
		{
			return shape.get();
		}

		/// <summary>
		/// Gets a reference to the shape size dimension data.
		/// </summary>
		/// <returns>Reference to ShapeSizeData.</returns>
		inline ShapeSizeData& GetShapeSizeData()
		{
			return shape->GetSizeData();
		}

		/// <summary>
		/// Gets the geometric shape type (Rectangle or Circle).
		/// </summary>
		/// <returns>ShapeType enum value.</returns>
		inline ShapeType GetShapeType() const
		{
			return shape->GetShapeType();
		}

		/// <summary>
		/// Sets the dimensions of the rectangular shape.
		/// </summary>
		/// <param name="_size">Width and height vector.</param>
		void SetSize(const FVector2& _size)
		{
			shape->SetSize(_size);
		}

		/// <summary>
		/// Gets the dimensions of the rectangular shape.
		/// </summary>
		/// <returns>Width and height vector.</returns>
		FVector2 GetSize() const
		{
			return shape->GetSize();
		}

		/// <summary>
		/// Sets the radius of the circular shape.
		/// </summary>
		/// <param name="_radius">Radius value.</param>
		void SetRadius(const Float& _radius)
		{
			shape->SetRadius(_radius);
		}

		/// <summary>
		/// Gets the radius of the circular shape.
		/// </summary>
		/// <returns>Radius value.</returns>
		Float GetRadius() const
		{
			return shape->GetRadius();
		}

		/// <summary>
		/// Sets the point count approximation for the circle polygon.
		/// </summary>
		/// <param name="_pointCount">Number of vertices.</param>
		void SetPointCount(const size_t& _pointCount)
		{
			shape->SetPointCount(_pointCount);
		}

		/// <summary>
		/// Gets the point count of the shape polygon.
		/// </summary>
		/// <returns>Number of points.</returns>
		size_t GetPointCount() const
		{
			return shape->GetPointCount();
		}

		/// <summary>
		/// Loads and applies a texture image to the shape.
		/// </summary>
		/// <param name="_path">Texture file path or identifier.</param>
		/// <param name="_textureType">Texture file format extension.</param>
		/// <param name="_rect">Texture sub-rectangle (empty for full texture).</param>
		/// <param name="_isRepeated">Whether texture repeating/tiling is enabled.</param>
		/// <param name="_smooth">Whether bilinear filtering is enabled.</param>
		void SetTexture(const std::string& _path, TextureExtensionType _textureType = TextureExtensionType::PNG, const IRect& _rect = IRect(), const Bool& _isRepeated = false, const Bool& _smooth = true)
		{
			shape->SetTexture(_path, _textureType, _rect, _isRepeated, _smooth);
		}

		/// <summary>
		/// Constructs a SpriteComponent initialized with circle shape parameters.
		/// </summary>
		/// <param name="_owner">Pointer to owning Actor.</param>
		/// <param name="_data">Configuration settings for circle shape.</param>
		SpriteComponent(Actor* _owner, const CircleShapeData& _data);

		/// <summary>
		/// Constructs a SpriteComponent initialized with rectangle shape parameters.
		/// </summary>
		/// <param name="_owner">Pointer to owning Actor.</param>
		/// <param name="_data">Configuration settings for rectangle shape.</param>
		SpriteComponent(Actor* _owner, const RectangleShapeData& _data);

		/// <summary>
		/// Lifecycle callback invoked when the component is created.
		/// </summary>
		virtual void Construct() override;

		/// <summary>
		/// Lifecycle callback invoked when the component is deconstructed.
		/// </summary>
		virtual void Deconstruct() override;

	public:
		/// <summary>
		/// Renders the shape into the given window.
		/// </summary>
		/// <param name="_window">Reference to target Window.</param>
		virtual void Draw(Window& _window) override;

		/// <summary>
		/// Renders debug overlay visualization for the sprite bounds.
		/// </summary>
		virtual void DrawDebug() override;

		/// <summary>
		/// Sets the numeric Z-order sorting layer.
		/// </summary>
		/// <param name="_zOrder">Numeric Z-order value.</param>
		virtual void SetZOrder(const uint8_t& _zOrder) override;

		/// <summary>
		/// Sets the Z-order sorting layer using predefined ZOrder enumeration values.
		/// </summary>
		/// <param name="_zOrder">ZOrder enum value.</param>
		virtual void SetZOrder(const ZOrder& _zOrder) override;

		/// <summary>
		/// Returns a string representation of the component.
		/// </summary>
		/// <returns>String representation.</returns>
		virtual std::string ToString() const override;
	};

}
