#include "Engine.h"
#include "Managers/TimerManager.h"
#include "Managers/InputManager.h"
#include "Managers/LevelManager.h"
#include "Graphics/Window/MainWindow.h"
#include "Graphics/Mouse.h"

using namespace Krampus;

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

Engine::Engine()
{
	MAIN_WINDOW.Create("EngineSFML", UVector2(1920, 1080));
}

void Engine::Start()
{
	onEngineStart.Broadcast();

#ifdef _MSC_VER
	InitConfig();
#endif

	M_INPUT.WindowClose.AddListener([this](){
		MAIN_WINDOW.Close();
		M_LEVEL.SetLevel(nullptr);
		});

	Logger::Init();
	Update();
	Stop();
}

void Engine::Update()
{
	while (Level* _currentLevel = M_LEVEL.GetCurrentLevel())
	{
		Mouse::GetInstance().Update();
		_currentLevel->Update(M_TIMER.Update());
		const std::optional<sf::Event>& _event = MAIN_WINDOW.PollEvent();
		M_INPUT.Update(_event);
		M_INPUT.UpdateSystemEvent(_event);
	}
}

void Engine::Stop()
{
	onEngineStop.Broadcast();
	M_LEVEL.Destroy();
	Logger::Shutdown();
}