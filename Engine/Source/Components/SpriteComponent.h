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
		inline ShapeObject* GetShapeObject()
		{
			return shape.get();
		}
		inline ShapeSizeData& GetShapeSizeData()
		{
			return shape->GetSizeData();
		}
		inline ShapeType GetShapeType() const
		{
			return shape->GetShapeType();
		}
		void SetSize(const FVector2& _size)
		{
			shape->SetSize(_size);
		}
		FVector2 GetSize() const
		{
			return shape->GetSize();
		}
		void SetRadius(const float& _radius)
		{
			shape->SetRadius(_radius);
		}
		float GetRadius() const
		{
			return shape->GetRadius();
		}
		void SetPointCount(const size_t& _pointCount)
		{
			shape->SetPointCount(_pointCount);
		}
		size_t GetPointCount() const
		{
			return shape->GetPointCount();
		}
		void SetTexture(const std::string& _path, TextureExtensionType _textureType = TextureExtensionType::PNG, const IRect& _rect = IRect(), const bool _isRepeated = false, const bool _smooth = true)
		{
			shape->SetTexture(_path, _textureType, _rect, _isRepeated, _smooth);
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
