#pragma once
#include "Actor.h"
#include "Components/SpriteComponent.h"
#include "Components/CollisionComponent.h"
#include "Managers/TextureManager.h"

namespace Krampus
{

	class SpriteActor : public Actor
	{
		std::vector<Event<>::ListenerHandle> handle;
		Event<IVector2>::ListenerHandle h;

	protected:
		SpriteComponent* sprite = nullptr;
		CollisionComponent* collision = nullptr;

	public:
		inline SpriteComponent* GetSprite() const
		{
			return sprite;
		}
		inline void SetTextureRect(const IVector2& _start, const IVector2& _size)
		{
			M_TEXTURE.SetTextureRect(sprite->GetShapeObject()->GetShape(), _start, _size);
		}
		inline void SetTextureRect(const IRect& _rect)
		{
			M_TEXTURE.SetTextureRect(sprite->GetShapeObject()->GetShape(), _rect);
		}

		SpriteActor(Level* _level, const CircleShapeData& _data);
		SpriteActor(Level* _level, const RectangleShapeData& _data);

		void MoveUp();
		void MoveRight();
		void MoveDown();
		void MoveLeft();
		void Rotate(IVector2 _pos);
		void Shoot();
	};


}
