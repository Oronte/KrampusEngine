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

	//engine::SpriteActor* _actor = _level->GetActorManager().CreateActor<engine::SpriteActor>(_level.get(), engine::FVector2(200.f), "Player", engine::TextureExtensionType::PNG, engine::IRect());
	//_actor->transform.position += engine::FVector2(200.f, 540.f);
	//_actor->transform.rotation = engine::Angle(50.f, true);
	//_actor->GetComponent<engine::SpriteComponent>()->useDebug = true;
	//
	//engine::SpriteActor* _defaultActor = _level->GetActorManager().CreateActor<engine::SpriteActor>(_level.get(), 50.f);
	//_defaultActor->GetComponent<engine::SpriteComponent>()->useDebug = true;
	//_defaultActor->transform.position = engine::FVector2(1600.f, 300.f);

	TestFps(_level.get());
	
	Krampus::SpriteActor* _actor = _level->GetActorManager().CreateActor<Krampus::SpriteActor>(_level.get(), Krampus::FVector2(125.f, 193.75f), "RyuGifSheet", Krampus::TextureExtensionType::PNG, Krampus::IRect());
	_actor->transform.position += Krampus::FVector2(200.f, 540.f);
	_actor->transform.rotation = Krampus::Angle(50.f, true);
	_actor->GetComponent<Krampus::SpriteComponent>()->useDebug = true;

	//engine::SoftBodyActor* softActor = _level->GetActorManager().CreateActor<engine::SoftBodyActor>(
	//	_level.get(), 80.f, 20 /*pointCount*/
	//);
	//softActor->transform.position = engine::FVector2(600.f, 600.f);
	//softActor->GetSoftBody()->PinParticle(0, true);

	//// exemple : actor soft body grille
	//engine::SoftBodyActor* softGrid = _level->GetActorManager().CreateActor<engine::SoftBodyActor>(
	//	_level.get(), engine::FVector2(200.f, 120.f), 10 /*w*/, 6 /*h*/
	//);
	//softGrid->transform.position = engine::FVector2(600.f, 300.f);
	//softGrid->GetSoftBody()->PinParticle(0, true);

	std::unique_ptr<Krampus::Level> _level2 = std::make_unique<Krampus::Level>("second level");
	//_level2->SetBackgroundColor(engine::Color::Green());
	Krampus::SpriteActor* _actor2 = _level2->GetActorManager().CreateActor<Krampus::SpriteActor>(_level2.get(), Krampus::FVector2(125.f, 193.75f), "RyuGifSheet", Krampus::TextureExtensionType::PNG, Krampus::IRect());
	_actor2->transform.rotation += Krampus::FMath::pi;

	_engine.Start();
}

int main(/*int argc, char** argv*/)
{
	//std::cout << PROJECT_ROOT;

	M_TIMER.SetMaxFrameRate(0);

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