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
	M_INPUT.MouseMoved.AddListener(this, &SpriteActor::Move);
	M_INPUT.MouseWheelScroll.AddListener([](float _delta) { LOG_MSG(std::to_string(_delta)); });
}

Krampus::SpriteActor::SpriteActor(Level* _level, const float& _radius, const std::string& _path, const TextureExtensionType& _textureType, const IRect& _rect, const size_t& _pointCount)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(_radius, _path, _textureType, _rect, _pointCount);
	animation = CreateComponent<AnimationComponent>();
}

void Krampus::SpriteActor::Move(const IVector2& _position)
{
	transform.position = _position;
}