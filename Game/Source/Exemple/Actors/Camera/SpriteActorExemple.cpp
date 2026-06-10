#include "SpriteActorExemple.h"

Krampus::Exemple::SpriteActorExemple::SpriteActorExemple(Level* _level, const RectangleShapeData& _data)
	: Actor(_level)
{
	CreateComponent<SpriteComponent>(_data);
}

Krampus::Exemple::SpriteActorExemple::SpriteActorExemple(Level* _level, const CircleShapeData& _data)
	: Actor(_level)
{
	CreateComponent<SpriteComponent>(_data);
}
