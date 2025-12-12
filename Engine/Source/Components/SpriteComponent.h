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

		SpriteComponent(Actor* _owner, const float& _radius, const std::string& _path = "Default", 
			const TextureExtensionType& _textureType = TextureExtensionType::PNG, const IRect& _rect = IRect(), const size_t& _pointCount = 30);
		SpriteComponent(Actor* _owner, const FVector2& _size, const std::string& _path = "Default", 
			const TextureExtensionType& _textureType = TextureExtensionType::PNG, const IRect& _rect = IRect(), const bool& _isRepeated = false);

		virtual void Construct() override;
		virtual void Deconstruct() override;
		virtual void Tick(const float& _deltaTime) override;

		virtual void Draw(Window& _window) override;
		virtual void DrawDebug(Window& _window) override;
		void SetOriginAtMiddle();
	};

}
