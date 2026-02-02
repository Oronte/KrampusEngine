#include "Core/Engine.h"
#include "Managers/AudioManager.h"
#include "Managers/LevelManager.h"
#include "Actors/PhysicsActor.h"


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

	//Krampus::Music _sound = M_AUDIO.CreateMusic("Shoryuken", Krampus::AudioExtensionType::MP3);
	//_sound.SetRelativeToListener(false);
	//_sound.SetPosition(Krampus::FVector3(10.0f));
	//_sound.SetLoop(true);
	//_sound.SetVolume(100.0f);
	//_sound.Play();

	for (int _x = 0; _x < 10; _x++)
	{
		for (int _y = 0; _y < 3; _y++)
		{
			Krampus::Actor* _actor = nullptr;
			bool _rect = Krampus::IMath::RandomRange(0, 1);
			if (_rect) _actor = _level->SpawnActor<Krampus::PhysicsActor>(Krampus::RectangleShapeData());
			else _actor = _level->SpawnActor<Krampus::PhysicsActor>(Krampus::CircleShapeData());
			_actor->transform.position = Krampus::FVector2(200.f + 200 * _x, 200.f + 150 * _y);
			_actor->transform.rotation = Krampus::FMath::RandomRange(0, Krampus::FMath::pi * 2);
		}
	}

	Krampus::RectangleShapeData _data;
	_data.size = Krampus::FVector2(1920, 100);

	Krampus::Actor* _actor1 = _level->SpawnActor<Krampus::SpriteActor>(_data);
	_actor1->transform.rotation = Krampus::FMath::pi / 2.0f;
	_actor1->transform.position = Krampus::FVector2(50, 0);
	Krampus::Actor * _actor2 = _level->SpawnActor<Krampus::SpriteActor>(_data);
	_actor2->transform.rotation = Krampus::FMath::pi / 2.0f;
	_actor2->transform.position = Krampus::FVector2(1900, 0);
	Krampus::Actor * _actor3 = _level->SpawnActor<Krampus::SpriteActor>(_data);
	_actor3->transform.position = Krampus::FVector2(960, 1000);

	_engine.Start();
}