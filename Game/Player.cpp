#include "Player.h"

Player::Player(Level* _level)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(RectangleShapeData(FVector2(300.0f, 200.0f), "Character/JumpPack_CharacterSpriteSheet"));
	collision = CreateComponent<CollisionComponent>();
	camera = CreateComponent<CameraComponent>();
}
