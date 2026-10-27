#pragma once
#include "WidgetComponent.h"
#include "Graphics/Shapes/ShapeObject.h"
#include "Utilities/System/Debuggable.h"
#include "Utilities/System/Drawable.h"

namespace Krampus
{
    /// <summary>
    /// Component that renders textured 2D shapes (rectangles, circles) within a screen-space HUD widget.
    /// </summary>
    class ImageWidgetComponent : public WidgetComponent, public IDebuggable, public IDrawable
    {
        std::unique_ptr<ShapeObject>	shape;

        Event<FVector2>::ListenerHandle onOwnerMoveHandle;
        Event<Angle>::ListenerHandle    onOwnerRotateHandle;
        Event<FVector2>::ListenerHandle onOwnerScaleHandle;

    public:
        /// <summary>Gets a pointer to the ShapeObject.</summary>
        /// <returns>Pointer to ShapeObject.</returns>
        inline ShapeObject* GetShapeObject()
        {
            return shape.get();
        }

        /// <summary>Gets a reference to the shape size dimension data.</summary>
        /// <returns>Reference to ShapeSizeData.</returns>
        inline ShapeSizeData& GetShapeSizeData()
        {
            return shape->GetSizeData();
        }

        /// <summary>Gets the shape type (Circle or Rectangle).</summary>
        /// <returns>ShapeType enum value.</returns>
        inline ShapeType GetShapeType() const
        {
            return shape->GetShapeType();
        }

        /// <summary>Sets rectangle dimensions.</summary>
        /// <param name="_size">Width and height.</param>
        void SetSize(const FVector2& _size)
        {
            shape->SetSize(_size);
        }

        /// <summary>Gets rectangle dimensions.</summary>
        /// <returns>Width and height.</returns>
        FVector2 GetSize() const
        {
            return shape->GetSize();
        }

        /// <summary>Sets circle radius.</summary>
        /// <param name="_radius">Radius value.</param>
        void SetRadius(const Float& _radius)
        {
            shape->SetRadius(_radius);
        }

        /// <summary>Gets circle radius.</summary>
        /// <returns>Radius value.</returns>
        Float GetRadius() const
        {
            return shape->GetRadius();
        }

        /// <summary>Sets point count for circle approximation.</summary>
        /// <param name="_pointCount">Number of points.</param>
        void SetPointCount(const size_t& _pointCount)
        {
            shape->SetPointCount(_pointCount);
        }

        /// <summary>Gets point count.</summary>
        /// <returns>Number of points.</returns>
        size_t GetPointCount() const
        {
            return shape->GetPointCount();
        }

        /// <summary>Loads and applies a texture resource.</summary>
        /// <param name="_path">Texture file path or key.</param>
        /// <param name="_textureType">Format type.</param>
        /// <param name="_rect">Texture sub-rectangle.</param>
        /// <param name="_isRepeated">Whether texture tiles.</param>
        /// <param name="_smooth">Whether smoothing is enabled.</param>
        void SetTexture(const std::string& _path, TextureExtensionType _textureType = TextureExtensionType::PNG, const IRect& _rect = IRect(), const Bool& _isRepeated = false, const Bool& _smooth = true)
        {
            shape->SetTexture(_path, _textureType, _rect, _isRepeated, _smooth);
        }

        /// <summary>Constructs a circular ImageWidgetComponent.</summary>
        /// <param name="_owner">Owning Widget.</param>
        /// <param name="_data">Circle parameters.</param>
        ImageWidgetComponent(Widget* _owner, const CircleShapeData& _data);

        /// <summary>Constructs a rectangular ImageWidgetComponent.</summary>
        /// <param name="_owner">Owning Widget.</param>
        /// <param name="_data">Rectangle parameters.</param>
        ImageWidgetComponent(Widget* _owner, const RectangleShapeData& _data);

        /// <summary>Lifecycle callback on component creation.</summary>
        virtual void Construct() override;
        /// <summary>Lifecycle callback on component deconstruction.</summary>
        virtual void Deconstruct() override;

    public:
        /// <summary>Renders the shape into the target window.</summary>
        /// <param name="_window">Target Window.</param>
        virtual void Draw(Window& _window) override;

        /// <summary>Renders debug visualization overlays.</summary>
        virtual void DrawDebug() override;

        /// <summary>Sets the numeric Z-order sorting layer.</summary>
        /// <param name="_zOrder">Numeric Z-order.</param>
        virtual void SetZOrder(const uint8_t& _zOrder) override;

        /// <summary>Sets the Z-order sorting layer using ZOrder enum.</summary>
        /// <param name="_zOrder">ZOrder enum value.</param>
        virtual void SetZOrder(const ZOrder& _zOrder) override;

        /// <summary>Returns a string representation of the component.</summary>
        /// <returns>String representation.</returns>
        virtual std::string ToString() const override;
    };
}
