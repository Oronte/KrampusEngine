#include "SpriteActor.h"
#include "GameFramework/Level.h"

#include"Managers/InputManager.h"
#include"Managers/TimerManager.h"

Krampus::SpriteActor::SpriteActor(Level* _level, bool _test, const FVector2& _size, const std::string& _path, const TextureExtensionType& _textureType, const IRect& _rect, const bool& _isRepeated)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(_size, _path, _textureType, _rect, _isRepeated);
	animation = CreateComponent<AnimationComponent>();
	animation->AddAnimation("Idle", AnimationData(6, 0.5f, SpriteData(IVector2(0, 0), IVector2(500, 775)), true, true, ReadDirection::RD_ROW));
	animation->SetCurrentAnimation("Idle");
	animation->StartAnimation();

	collision = CreateComponent<CollisionComponent>();

	if (_test)
	{
		//M_INPUT.MouseMoved.AddListener([this](Krampus::FVector2 _pos) {transform.position = _pos; });
		M_INPUT.Z.onPerform.AddListener(this, &SpriteActor::MoveUp);
		M_INPUT.S.onPerform.AddListener(this, &SpriteActor::MoveDown);
		M_INPUT.Q.onPerform.AddListener(this, &SpriteActor::MoveLeft);
		M_INPUT.D.onPerform.AddListener(this, &SpriteActor::MoveRight);

		collision->onCollision.AddListener([this](CollisionInfo _info) {
			transform.position += _info.normal * _info.penetration;
			LOG_MSG("Collision");
			});
	}
}

Krampus::SpriteActor::SpriteActor(Level* _level, bool _test, const float& _radius, const std::string& _path, const TextureExtensionType& _textureType, const IRect& _rect, const size_t& _pointCount)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(_radius, _path, _textureType, _rect, _pointCount);
	animation = CreateComponent<AnimationComponent>();
	animation->AddAnimation("Idle", AnimationData(6, 0.5f, SpriteData(IVector2(0, 0), IVector2(500, 775)), true, true, ReadDirection::RD_ROW));
	animation->SetCurrentAnimation("Idle");
	animation->StartAnimation();

	collision = CreateComponent<CollisionComponent>();

	if (_test)
	{
		//M_INPUT.MouseMoved.AddListener([this](Krampus::FVector2 _pos) {transform.position = _pos; });
		M_INPUT.Z.onPerform.AddListener(this, &SpriteActor::MoveUp);
		M_INPUT.S.onPerform.AddListener(this, &SpriteActor::MoveDown);
		M_INPUT.Q.onPerform.AddListener(this, &SpriteActor::MoveLeft);
		M_INPUT.D.onPerform.AddListener(this, &SpriteActor::MoveRight);

		collision->onCollision.AddListener([this](CollisionInfo _info) {
			transform.position += _info.normal * _info.penetration;
			LOG_MSG("Collision");
			});
	}
}

void Krampus::SpriteActor::MoveUp()
{
	transform.position += FVector2(0, -10);
}
void Krampus::SpriteActor::MoveDown()
{
	transform.position += FVector2(0, 10);

}
void Krampus::SpriteActor::MoveLeft()
{
	transform.position += FVector2(-10, 0);

}
void Krampus::SpriteActor::MoveRight()
{
	transform.position += FVector2(10, 0);

}