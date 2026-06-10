#include "CollisionActorExemple.h"
#include "Components/SpriteComponent.h"
#include "Components/CollisionComponent.h"

Krampus::Exemple::CollisionActorExemple::CollisionActorExemple(Level* _level)
	: Actor(_level)
{
	RectangleShapeData _data;
	_data.size = FVector2(2920.0f, 200.0f);
	CreateComponent<SpriteComponent>(_data);
	CreateComponent<CollisionComponent>(CollisionChannel::Wall, CollisionChannel::All);
}
