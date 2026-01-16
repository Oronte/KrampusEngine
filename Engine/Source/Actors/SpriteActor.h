#pragma once
#include "Actor.h"
#include "Components/SpriteComponent.h"
#include "Managers/TextureManager.h"
#include "Components/CollisionComponent.h"
#include "Components/PhysicsComponent.h"

namespace Krampus
{

	class SpriteActor : public Actor
	{
		SpriteComponent* sprite = nullptr;
		CollisionComponent* collision = nullptr;
		PhysicsComponent* physics = nullptr;

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

		SpriteActor(Level* _level, const CircleShapeData& _data, bool test);
		SpriteActor(Level* _level, const RectangleShapeData& _data, bool test);

		virtual void Tick(const float& _deltaTime) override;
	};


}
