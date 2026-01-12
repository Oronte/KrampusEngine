#pragma once
#include "Component.h"
#include "Graphics/Shapes/ShapeObject.h"
#include "Utilities/System/Debuggable.h"
#include "Utilities/System/Drawable.h"
#include "Graphics/Ressources/Text.h"

namespace Krampus
{

	struct CircleShapeData
	{
		float radius;
		std::string texturePath;
		TextureExtensionType textureType;
		IRect textureRect;
		size_t pointCount;

		CircleShapeData(const float& _radius, const std::string& _path = "Default",
			const TextureExtensionType& _textureType = TextureExtensionType::PNG, 
			const IRect& _rect = IRect(), const size_t& _pointCount = 30)
			: radius(_radius), texturePath(_path), textureType(_textureType), textureRect(_rect), pointCount(_pointCount) { }
	};

	struct RectangleShapeData
	{
		FVector2 size;
		std::string texturePath;
		TextureExtensionType textureType;
		IRect textureRect;
		bool isRepeated;

		RectangleShapeData(const FVector2& _size, const std::string& _path = "Default",
			const TextureExtensionType& _textureType = TextureExtensionType::PNG, 
			const IRect& _rect = IRect(), const bool& _isRepeated = false)
			: size(_size), texturePath(_path), textureType(_textureType), textureRect(_rect), isRepeated(_isRepeated) { }
	};

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
	protected:
		virtual void Deconstruct() override;
		virtual void Tick(const float& _deltaTime) override;
		virtual void BeginDestroy() override;

	public:
		virtual void Draw(Window& _window) override;
		virtual void DrawDebug(Window& _window) override;
		virtual void SetZOrder(const uint8_t& _zOrder) override;
		virtual void SetZOrder(const ZOrder& _zOrder) override;
	};

}
