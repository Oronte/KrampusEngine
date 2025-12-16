#include "SpriteActor.h"
#include "GameFramework/Level.h"

#include"Managers/InputManager.h"

Krampus::SpriteActor::SpriteActor(Level* _level, const FVector2& _size, const std::string& _path, const TextureExtensionType& _textureType, const IRect& _rect, const bool& _isRepeated)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(_size, _path, _textureType, _rect, _isRepeated);
	animation = CreateComponent<AnimationComponent>();
	animation->AddAnimation("Idle", AnimationData(6, 0.5f, SpriteData(IVector2(0, 0), IVector2(500, 775)), true, true, ReadDirection::RD_ROW));
	animation->SetCurrentAnimation("Idle");
	animation->StartAnimation();
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
		M_INPUT.MouseMoved.AddListener([this](Krampus::FVector2 _pos) {transform.position = _pos; });

		collision->onCollision.AddListener([this](CollisionInfo _info) {
			animation->StopAnimation();
			LOG_MSG("Contact Point : " + _info.contactPoint.ToString());
			LOG_MSG("Normal : " + _info.normal.ToString());
			LOG_MSG("Penetration : " + std::to_string(_info.penetration));
			transform.position += _info.normal * _info.penetration;
			});
	}
}