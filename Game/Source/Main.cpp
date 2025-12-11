#include "Core/Engine.h"
#include "Actors/SpriteActor.h"
#include "Actors/SoftBodyActor.h"
#include "GameFramework/Level.h"
#include "Managers/TimerManager.h"
#include "Managers/LevelManager.h"

void InitConfig()
{
#ifdef _MSC_VER
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void TestFps(Krampus::Level* _level)
{
	for (int i = 1; i < 100; i++)
	{
		for (int j = 1; j < 100; j++)
		{
			Krampus::SpriteActor* _actor = _level->GetActorManager().CreateActor<Krampus::SpriteActor>(_level, Krampus::FVector2(21.6f, 38.4f), "RyuGifSheet", Krampus::TextureExtensionType::PNG, Krampus::IRect());
			_actor->transform.position = Krampus::FVector2(5.4f * (j - 1), 9.6f * (i - 1));
		}
	}
}

void StartGame()
{
	InitConfig();

	std::unique_ptr<Krampus::Level> _level = std::make_unique<Krampus::Level>("first level");
	M_LEVEL.SetLevel(_level.get());
	Krampus::Engine& _engine = Krampus::Engine::GetInstance();

	//TestFps(_level.get());
	
	Krampus::SpriteActor* _actor = _level->GetActorManager().CreateActor<Krampus::SpriteActor>(_level.get(), Krampus::FVector2(125.f, 193.75f), "RyuGifSheet", Krampus::TextureExtensionType::PNG, Krampus::IRect());
	_actor->transform.position += Krampus::FVector2(200.f, 540.f);
	_actor->transform.rotation = Krampus::Angle(50.f, true);
	_actor->GetComponent<Krampus::SpriteComponent>()->useDebug = true;

	_engine.Start();
}

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