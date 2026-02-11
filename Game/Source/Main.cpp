#include "Core/Engine.h"
#include "Managers/AudioManager.h"
#include "Managers/LevelManager.h"
#include "Actors/Character.h"

using namespace Krampus;

void StartGame();

#ifdef RELEASE

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	StartGame();

	return EXIT_SUCCESS;
}
#endif

#ifdef DEBUG

int main(/*int argc, char** argv*/)
{
	try
	{
		StartGame();
	}
	catch (const Krampus::Exception& _exception)
	{
		std::cerr << TXT_DARK_RED << _exception.What() << COLOR_RESET << std::endl;
	}

	return EXIT_SUCCESS;
}

#endif

void StartGame()
{
	Krampus::Engine _engine;
	Krampus::Level* _level = _engine.GetLevelManager()->SetLevel<Krampus::Level>();
	

	//for (int _x = 0; _x < 10; _x++)
	//{
	//	for (int _y = 0; _y < 2; _y++)
	//	{
	//		Krampus::Actor* _actor = nullptr;
	//		bool _rect = Krampus::IMath::RandomRange(0, 1);
	//		if (_rect)
	//		{
	//			Krampus::RectangleShapeData _data;
	//			_data.texturePath = "KrampusEngineBanner";
	//			_actor = _level->SpawnActor<Krampus::PhysicsActor>(_data);
	//		}
	//		else
	//		{
	//			Krampus::CircleShapeData _data;
	//			_data.texturePath = "Player";
	//			_actor = _level->SpawnActor<Krampus::PhysicsActor>(_data);
	//		}
	//		_actor->transform.position = Krampus::FVector2(200.f + 200 * _x, 200.f + 150 * _y);
	//		_actor->transform.rotation = Krampus::FMath::RandomRange(0, Krampus::FMath::pi * 2);
	//	}
	//}
	//
	//Krampus::RectangleShapeData _data;
	//_data.size = Krampus::FVector2(1920, 100);
	//
	//Krampus::Actor* _actor1 = _level->SpawnActor<Krampus::SpriteActor>(_data);
	//_actor1->transform.rotation = Krampus::FMath::pi / 2.0f;
	//_actor1->transform.position = Krampus::FVector2(50, 0);
	//Krampus::Actor * _actor2 = _level->SpawnActor<Krampus::SpriteActor>(_data);
	//_actor2->transform.rotation = Krampus::FMath::pi / 2.0f;
	//_actor2->transform.position = Krampus::FVector2(1900, 0);
	//Krampus::Actor * _actor3 = _level->SpawnActor<Krampus::SpriteActor>(_data);
	//_actor3->transform.position = Krampus::FVector2(960, 1000);

	Krampus::CircleShapeData _data;
	_data.texturePath = "Player";
	_data.pointCount = 60;
	_data.radius = 75.0f;
	_level->SpawnActor<Krampus::SpriteActor>(_data);

	Krampus::RectangleShapeData _rectData;
	_rectData.texturePath = "ZQSD";
	_rectData.size = FVector2(300.0f, 200.0f);
	_level->SpawnActor<Krampus::SpriteActor>(_rectData)->transform.position = FVector2(500.0f, 300.0f);
	
	_engine.Start();
}