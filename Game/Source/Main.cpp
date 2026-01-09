#include "Core/Engine.h"
#include "Actors/SpriteActor.h"
#include "Actors/SoftBodyActor.h"
#include "GameFramework/Level.h"
#include "Managers/TimerManager.h"
#include "Managers/LevelManager.h"
#include "Managers/AudioManager.h"
#include "Managers/InputManager.h"
#include "UI/Button.h"


void StartGame();
void InitConfig();

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
	//InitConfig();
	//_CrtMemState _start, _end, _diff;
	//_CrtMemCheckpoint(&_start);

	try
	{
		StartGame();
	}
	catch (const Krampus::Exception& _exception)
	{
		std::cerr << TXT_DARK_RED << _exception.What() << COLOR_RESET << std::endl;
	}

	//_CrtMemCheckpoint(&_end);
	//if (_CrtMemDifference(&_diff, &_start, &_end))
	//	if (_diff.lCounts[1] > 0)
	//		_CrtMemDumpStatistics(&_diff);

	return EXIT_SUCCESS;
}


// Memory leak detection
#ifdef _MSC_VER

int __cdecl HookReport(int _reportType, char* _message, int* _returnValue)
{
	HANDLE _hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(_hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);

	DWORD _written;
	WriteConsoleA(_hOut, _message, (DWORD)strlen(_message), &_written, NULL);

	SetConsoleTextAttribute(_hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

	return false;
}
void InitConfig()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportHook(HookReport);
}
#endif

#endif

void TestFps(Krampus::Level* _level)
{
	for (int i = 1; i < 15; i++)
	{
		for (int j = 1; j < 100; j++)
		{
			Krampus::SpriteActor* _actor = _level->SpawnActor<Krampus::SpriteActor>(false, Krampus::FVector2(21.6f, 38.4f)/*30.f*/, "RyuGifSheet", Krampus::TextureExtensionType::PNG, Krampus::IRect());
			_actor->transform.position = Krampus::FVector2(5.4f * (j - 1), 9.6f * (i - 1));
			//_actor->GetComponent<Krampus::SpriteComponent>()->useDebug = true;
		}
	}
}

void StartGame()
{
	std::unique_ptr<Krampus::Level> _level = std::make_unique<Krampus::Level>("first level");
	M_LEVEL.SetLevel(_level.get());
	Krampus::Engine& _engine = ENGINE;

	//TestFps(_level.get());

	Krampus::SpriteActor* _actor = _level->SpawnActor<Krampus::SpriteActor>(true, Krampus::FVector2(125.f, 193.75f) /*150.f*/, "RyuGifSheet", Krampus::TextureExtensionType::PNG, Krampus::IRect());
	_actor->transform.position += Krampus::FVector2(1600.f, 540.f);
	//_actor->transform.rotation = Krampus::Angle(-70.f, true);
	_actor->GetComponent<Krampus::SpriteComponent>()->useDebug = true;
	_actor->GetComponent<Krampus::CollisionComponent>()->channel = Krampus::CollisionChannel::Player;
	_actor->GetComponent<Krampus::CollisionComponent>()->mask = Krampus::CollisionChannel::Enemy;

	Krampus::SpriteActor* _actor2 = _level->SpawnActor<Krampus::SpriteActor>(false, 100.f, "RyuGifSheet", Krampus::TextureExtensionType::PNG, Krampus::IRect());
	_actor2->transform.position = Krampus::FVector2(200.f, 220.f);
	_actor2->GetComponent<Krampus::SpriteComponent>()->useDebug = true;
	//_actor2->SetActive(false);
	_actor2->GetComponent<Krampus::CollisionComponent>()->channel = Krampus::CollisionChannel::Enemy;
	_actor2->GetComponent<Krampus::CollisionComponent>()->mask = Krampus::CollisionChannel::Player;

	Krampus::SpriteActor* _actor3 = _level->SpawnActor<Krampus::SpriteActor>(false, Krampus::FVector2(125.f, 193.75f), "RyuGifSheet", Krampus::TextureExtensionType::PNG, Krampus::IRect());
	_actor3->transform.position = Krampus::FVector2(200.f, 920.f);
	_actor3->transform.rotation = Krampus::Angle(50.f, true);
	_actor3->GetComponent<Krampus::SpriteComponent>()->useDebug = true;
	_actor3->GetComponent<Krampus::CollisionComponent>()->channel = Krampus::CollisionChannel::Enemy;
	_actor3->GetComponent<Krampus::CollisionComponent>()->mask = Krampus::CollisionChannel::Player;

	Krampus::SpriteActor* _actor4 = _level->SpawnActor<Krampus::SpriteActor>(false, Krampus::FVector2(125.f, 193.75f), "RyuGifSheet", Krampus::TextureExtensionType::PNG, Krampus::IRect());
	_actor4->transform.position = Krampus::FVector2(800.f, 720.f);
	_actor4->GetComponent<Krampus::SpriteComponent>()->useDebug = true;
	_actor4->GetComponent<Krampus::CollisionComponent>()->channel = Krampus::CollisionChannel::Enemy;
	_actor4->GetComponent<Krampus::CollisionComponent>()->mask = Krampus::CollisionChannel::Player;

	Krampus::Button* _button = _level->SpawnWidget<Krampus::Button>(Krampus::FVector2(200.0f, 300.0f), "KrampusEngineBanner");
	_button->onHover.AddListener([]() {LOG_MSG("onHover"); });
	_button->onUnhover.AddListener([]() {LOG_MSG("onUnhover"); });
	_button->onPress.AddListener([]() {LOG_MSG("onPress"); });
	_button->onPerform.AddListener([]() {LOG_MSG("onPerform"); });
	_button->onRelease.AddListener([]() {LOG_MSG("onRelease"); });
	_button->screenPosition = Krampus::FVector2(200.0f, 300.0f) / 2.0f;

	//_button->Destroy();
	//_actor3->Destroy();

	//Krampus::Sound _sound = M_AUDIO.CreateSound("Shoryuken", Krampus::AudioExtensionType::MP3);
	//_sound.SetVolume(50.f);
	//_sound.SetLoop(true);
	//_sound.SetPitch(1.5f);
	//_sound.Play();
	//Krampus::Sound _sound2 = M_AUDIO.CreateSound("Shoryuken", Krampus::AudioExtensionType::MP3);
	//_sound2.SetVolume(50.f);
	//_sound2.SetLoop(true);
	//_sound2.Play();

	_engine.Start();
}