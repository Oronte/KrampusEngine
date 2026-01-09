#pragma once
#include "Widget.h"
#include "Components/SpriteComponent.h"
#include "Components/CollisionComponent.h"

namespace Krampus
{

	class Button : public Widget
	{
		SpriteComponent* sprite;
		CollisionComponent* collision;

		bool isPressed = false;
		bool isHovered = false;

	public:
		Event<> onHover;
		Event<> onUnhover;
		Event<> onPress;
		Event<> onPerform;
		Event<> onRelease;

		Button(Level* _level, const float& _radius, const std::string& _path = "Default",
			const TextureExtensionType& _textureType = TextureExtensionType::PNG, const IRect& _rect = IRect(), const size_t& _pointCount = 30);
		Button(Level* _level, const FVector2& _size, const std::string& _path = "Default",
			const TextureExtensionType& _textureType = TextureExtensionType::PNG, const IRect& _rect = IRect(), const bool& _isRepeated = false);
	
	protected:
		virtual void Tick(const float& _deltaTime) override;

	private:
		void Init();

		void OnMouseMoved(const IVector2& _mousePos);
		void OnClick();
		void OnRelease();

		virtual void BeginDestroy() override;
	};

}

