#include "BlackoutLevel.h"
#include "Components/PhysicsComponent.h"
#include "Components/LightComponents.h"
#include "Components/StainComponent.h"

namespace Krampus
{

BlackoutWall::BlackoutWall(Level* _level)
	: Actor(_level)
{
	name = "Wall";

	SpriteComponent* _sprite = CreateComponent<SpriteComponent>(RectangleShapeData(FVector2(200.f, 40.f), "Blackout/Solid"));
	_sprite->GetShapeObject()->GetShape()->SetFillColor(Color(96, 100, 114));
	_sprite->SetZOrder(ZOrder::World);

	CreateComponent<StainComponent>();		// blood and burns stay on it

	PhysicsComponent* _physics = CreateComponent<PhysicsComponent>();	// the shape of the sprite
	_physics->SetStatic();

	CreateComponent<ShadowCasterComponent>();
}

BlackoutFloor::BlackoutFloor(Level* _level)
	: Actor(_level)
{
	name = "Floor";

	const FVector2 _size(3200.f, 2200.f);
	SpriteComponent* _sprite = CreateComponent<SpriteComponent>(RectangleShapeData(_size, "Blackout/Floor", TextureExtensionType::PNG,
		IRect(IVector2(0, 0), IVector2(static_cast<int>(_size.x), static_cast<int>(_size.y))), true));
	_sprite->GetShapeObject()->GetShape()->SetFillColor(Color(70, 74, 82));
	_sprite->SetZOrder(ZOrder::Background);

	// Blood and burns stay on it. Half resolution: the floor is big (marks picture of 1600 x 1100).
	StainComponent* _stains = CreateComponent<StainComponent>();
	_stains->resolution = 0.5f;
}

}
