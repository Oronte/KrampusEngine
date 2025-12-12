#pragma once
#include "Actor.h"
#include "Components/SpriteComponent.h"
#include "Components/AnimationComponent.h"
#include "Managers/TextureManager.h"

namespace Krampus
{

	class SpriteActor : public Actor
	{
		SpriteComponent* sprite;
		AnimationComponent* animation;

	public:
		INLINE SpriteComponent* GetSprite() const
		{
			return sprite;
		}
		INLINE void SetTextureRect(const IVector2& _start, const IVector2& _size)
		{
			M_TEXTURE.SetTextureRect(sprite->GetShapeObject()->GetShape(), _start, _size);
		}
		INLINE void SetTextureRect(const IRect& _rect)
		{
			M_TEXTURE.SetTextureRect(sprite->GetShapeObject()->GetShape(), _rect);
		}

		SpriteActor(Level* _level, const FVector2& _size, const std::string& _path = "Default",
			const TextureExtensionType& _textureType = TextureExtensionType::PNG, const IRect& _rect = IRect(), 
			const bool& _isRepeated = false);
		SpriteActor(Level* _level, const float& _radius, const std::string& _path = "Default",
			const TextureExtensionType& _textureType = TextureExtensionType::PNG, const IRect& _rect = IRect(),
			const size_t& _pointCount = 30);

	public:
		void Move(const IVector2& _position);
	};


}
