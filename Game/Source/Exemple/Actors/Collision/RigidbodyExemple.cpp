#include "RigidbodyExemple.h"
#include "Components/SpriteComponent.h"

Krampus::Exemple::RigidbodyExemple::RigidbodyExemple(Level* _level)
	: Actor(_level)
{
	float _mass = IMath::RandomRange(1.0f, 10.0f);
	bool _isRectangle = IMath::RandomRange(0, 1);
	if (_isRectangle)
	{
		RectangleShapeData _data;
		_data.size = FVector2(100.0f, 50.0f) * _mass / 2.0f;
		_data.texturePath = "_internal/KrampusEngineBanner";
		CreateComponent<SpriteComponent>(_data);
	}
	else
	{
		CircleShapeData _data;
		_data.radius = 20.0f * _mass / 2.0f;
		_data.texturePath = "_internal/KrampusLogo";
		CreateComponent<SpriteComponent>(_data);
	}
	collision = CreateComponent<CollisionComponent>(CollisionChannel::Projectile, CollisionChannel::Wall | CollisionChannel::Projectile);
	rb = CreateComponent<RigidbodyComponent>(_mass);
}

void Krampus::Exemple::RigidbodyExemple::Construct()
{
	Super::Construct();

	rb->BindCollisionResponse();
	rb->restitution = 0.0f;
}
