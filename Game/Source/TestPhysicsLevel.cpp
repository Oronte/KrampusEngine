#include "TestPhysicsLevel.h"
#include "Actors/SpriteActor.h"

void Krampus::TestPhysicsLevel::InitLevel()
{
	for (int i = 0; i < 15; i++)
	{
		Krampus::Actor* _actor;
		if (Krampus::IMath::RandomRange(0, 1))
			_actor = SpawnActor<Krampus::SpriteActor>(Krampus::RectangleShapeData(Krampus::FVector2(50, 150)), true);
		else
			_actor = SpawnActor<Krampus::SpriteActor>(Krampus::CircleShapeData(50), true);

		_actor->transform.position = Krampus::FVector2((float)i * 150 + 150, 0);
		_actor->GetComponent<Krampus::SpriteComponent>()->useDebug = true;
	}

	Krampus::Actor* _actor2 = SpawnActor<Krampus::SpriteActor>(Krampus::RectangleShapeData(Krampus::FVector2(1920, 100)), false);
	_actor2->transform.position = Krampus::FVector2(960, 1080);
	Krampus::Actor* _actor3 = SpawnActor<Krampus::SpriteActor>(Krampus::RectangleShapeData(Krampus::FVector2(100, 1080)), false);
	_actor3->transform.position = Krampus::FVector2(0, 540);
	Krampus::Actor* _actor4 = SpawnActor<Krampus::SpriteActor>(Krampus::RectangleShapeData(Krampus::FVector2(100, 1080)), false);
	_actor4->transform.position = Krampus::FVector2(1920, 540);

	_actor2->GetComponent<Krampus::SpriteComponent>()->useDebug = true;
	_actor3->GetComponent<Krampus::SpriteComponent>()->useDebug = true;
	_actor4->GetComponent<Krampus::SpriteComponent>()->useDebug = true;
}
