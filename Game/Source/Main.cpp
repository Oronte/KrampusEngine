#include "Core/Engine.h"
#include "Actors/SpriteActor.h"
#include "Actors/SoftBodyActor.h"
#include "GameFramework/Level.h"
#include "Managers/TimerManager.h"
#include "Managers/LevelManager.h"
#include "Managers/AudioManager.h"
#include "Managers/InputManager.h"


void TestFps(Krampus::Level* _level)
{
	for (int i = 1; i < 150; i++)
	{
		for (int j = 1; j < 100; j++)
		{
			Krampus::SpriteActor* _actor = _level->GetActorManager().CreateActor<Krampus::SpriteActor>(_level, false,Krampus::FVector2(21.6f, 38.4f)/*30.f*/, "RyuGifSheet", Krampus::TextureExtensionType::PNG, Krampus::IRect());
			_actor->transform.position = Krampus::FVector2(5.4f * (j - 1), 9.6f * (i - 1));
		}
	}
}

void StartGame()
{
	std::unique_ptr<Krampus::Level> _level = std::make_unique<Krampus::Level>("first level");
	M_LEVEL.SetLevel(_level.get());
	Krampus::Engine& _engine = ENGINE;

	//TestFps(_level.get());
	
	Krampus::SpriteActor* _actor = _level->GetActorManager().CreateActor<Krampus::SpriteActor>(_level.get(), true, Krampus::FVector2(125.f, 193.75f) /*150.f*/, "RyuGifSheet", Krampus::TextureExtensionType::PNG, Krampus::IRect());
	_actor->transform.position += Krampus::FVector2(1600.f, 540.f);
	_actor->transform.origin += Krampus::FVector2();
	_actor->GetComponent<Krampus::SpriteComponent>()->useDebug = true;

	Krampus::SpriteActor* _actor2 = _level->GetActorManager().CreateActor<Krampus::SpriteActor>(_level.get(), false, 100.f, "RyuGifSheet", Krampus::TextureExtensionType::PNG, Krampus::IRect());
	_actor2->transform.position = Krampus::FVector2(200.f, 220.f);
	_actor2->GetComponent<Krampus::SpriteComponent>()->useDebug = true;

	Krampus::SpriteActor* _actor3 = _level->GetActorManager().CreateActor<Krampus::SpriteActor>(_level.get(), false, Krampus::FVector2(125.f, 193.75f), "RyuGifSheet", Krampus::TextureExtensionType::PNG, Krampus::IRect());
	_actor3->transform.position = Krampus::FVector2(200.f, 920.f);
	_actor3->transform.rotation = Krampus::Angle(50.f, true);
	_actor3->GetComponent<Krampus::SpriteComponent>()->useDebug = true;

	Krampus::SpriteActor* _actor4 = _level->GetActorManager().CreateActor<Krampus::SpriteActor>(_level.get(), false, Krampus::FVector2(125.f, 193.75f), "RyuGifSheet", Krampus::TextureExtensionType::PNG, Krampus::IRect());
	_actor4->transform.position = Krampus::FVector2(800.f, 720.f);
	_actor4->GetComponent<Krampus::SpriteComponent>()->useDebug = true;

	Krampus::Sound _sound = M_AUDIO.CreateSound("Shoryuken", Krampus::AudioExtensionType::MP3);
	_sound.SetVolume(50.f);
	_sound.SetLoop(true);
	_sound.SetPitch(1.5f);
	_sound.Play();
	Krampus::Sound _sound2 = M_AUDIO.CreateSound("Shoryuken", Krampus::AudioExtensionType::MP3);
	_sound2.SetVolume(50.f);
	_sound2.SetLoop(true);
	_sound2.Play();

	_engine.Start();
}

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

#ifdef RELEASE

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	StartGame();

	return EXIT_SUCCESS;
}
#endif