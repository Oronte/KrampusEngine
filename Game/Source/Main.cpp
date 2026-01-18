#include "Core/Engine.h"
#include "Actors/SpriteActor.h"
#include "Actors/SoftBodyActor.h"
#include "GameFramework/Level.h"
#include "Managers/TimerManager.h"
#include "Managers/LevelManager.h"
#include "Managers/AudioManager.h"
#include "Managers/InputManager.h"
#include "UI/ButtonWidget.h"
#include "UI/GIFWidget.h"
#include "UI/LabelWidget.h"
#include "Actors/Collider2D.h"


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
	std::unique_ptr<Krampus::Level> _level = std::make_unique<Krampus::Level>("first level");
	M_LEVEL.SetLevel(_level.get());
	Krampus::Engine& _engine = ENGINE;

	for (int i = 0; i < 15; i++)
	{
		Krampus::Actor* _actor;
		if (Krampus::IMath::RandomRange(0, 1, i+55))
			_actor = _level->SpawnActor<Krampus::SpriteActor>(Krampus::RectangleShapeData(Krampus::FVector2(50, 150)), true);
		else	
			_actor = _level->SpawnActor<Krampus::SpriteActor>(Krampus::CircleShapeData(50), true);
		
		_actor->transform.position = Krampus::FVector2((float)i * 150 + 150, 0);
		_actor->GetComponent<Krampus::SpriteComponent>()->useDebug = true;
	}

	//Krampus::Actor* _actor = _level->SpawnActor<Krampus::SpriteActor>(Krampus::RectangleShapeData(Krampus::FVector2(200))/*Krampus::CircleShapeData()*/, true);
	//_actor->transform.position = Krampus::FVector2(900, 0);
	////_actor->transform.rotation = Krampus::Angle(0.7f);

	Krampus::Actor* _actor2 = _level->SpawnActor<Krampus::SpriteActor>(Krampus::RectangleShapeData(Krampus::FVector2(1920, 100)), false);
	_actor2->transform.position = Krampus::FVector2(960, 1080);
	Krampus::Actor* _actor3 = _level->SpawnActor<Krampus::SpriteActor>(Krampus::RectangleShapeData(Krampus::FVector2(100, 1080)), false);
	_actor3->transform.position = Krampus::FVector2(0, 0);
	Krampus::Actor* _actor4 = _level->SpawnActor<Krampus::SpriteActor>(Krampus::RectangleShapeData(Krampus::FVector2(100, 1080)), false);
	_actor4->transform.position = Krampus::FVector2(1920, 0);

	//_actor->GetComponent<Krampus::SpriteComponent>()->useDebug = true;
	_actor2->GetComponent<Krampus::SpriteComponent>()->useDebug = true;
	_actor3->GetComponent<Krampus::SpriteComponent>()->useDebug = true;
	_actor4->GetComponent<Krampus::SpriteComponent>()->useDebug = true;

	M_TIMER.SetMaxFrameRate(0);

	_engine.Start();
}