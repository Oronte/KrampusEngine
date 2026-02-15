#include "Poulpe.h"

Krampus::Poulpe::Poulpe(Level* _level)
	: SpriteActor(_level, RectangleShapeData(FVector2(400.0f, 300.0f), "Pulpe", TextureExtensionType::JPG))
{
	animation = CreateComponent<AnimationComponent>();
	AnimationData _data = AnimationData(5, 2.0f, SpriteData(IRect(FVector2(), FVector2(192, 186))));
	animation->AddAnimation("Poulpe", _data);
	rigidbody = CreateComponent<RigidbodyComponent>();
	rigidbody->gravity = 200.0f;
}
