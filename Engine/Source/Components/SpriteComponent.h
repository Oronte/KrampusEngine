#pragma once
#include "Component.h"
#include "Graphics/Shapes/ShapeObject.h"
#include "Utilities/System/Debuggable.h"
#include "Utilities/System/Drawable.h"
#include "Graphics/Ressources/Text.h"

namespace Krampus
{
	class Window;

	class SpriteComponent : public Component, public IDebuggable, public IDrawable
	{
		std::unique_ptr<ShapeObject> shape;

	public:
		INLINE ShapeObject* GetShapeObject()
		{
			return shape.get();
		}
		INLINE ShapeSizeData& GetShapeSizeData()
		{
			return shape->GetSizeData();
		}
		INLINE ShapeType GetShapeType() const
		{
			return shape->GetShapeType();
		}

		SpriteComponent(Actor* _owner, const CircleShapeData& _data);
		SpriteComponent(Actor* _owner, const RectangleShapeData& _data);

		virtual void Construct() override;
		virtual void Deconstruct() override;

	protected:
		virtual void Tick(const float& _deltaTime) override;

	public:
		virtual void Draw(Window& _window) override;
		virtual void DrawDebug() override;
		virtual void SetZOrder(const uint8_t& _zOrder) override;
		virtual void SetZOrder(const ZOrder& _zOrder) override;
	};

}
