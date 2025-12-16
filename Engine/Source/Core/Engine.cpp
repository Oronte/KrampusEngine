#include "Engine.h"
#include "Managers/TimerManager.h"
#include "Managers/InputManager.h"
#include "Managers/LevelManager.h"
#include "Graphics/Window/MainWindow.h"
//#include "ImGui/imgui.h"
//#include "ImGui/imgui-SFML.h"
#include "Graphics/Mouse.h"
#include "Managers/CollisionManager.h"

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
	//std::ifstream _ifs(windowSaveDir + windowSaveFileName);
	//if (!_ifs) MAIN_WINDOW.Create("EngineSFML", UVector2(1920, 1080));
	//else
	//{
	//	std::stringstream _buffer;
	//	_buffer << _ifs.rdbuf();
	//	std::string _jsonStr = _buffer.str();

	//	rapidjson::Document _document;
	//	_document.Parse(_jsonStr.c_str());

	//	UVector2 _windowSize;
	//	_windowSize.FromJson(_document["Size"]);
	//	UVector2 _windowPos;
	//	_windowPos.FromJson(_document["Pos"]);

	//	MAIN_WINDOW.Create("EngineSFML", _windowSize);
	//	MAIN_WINDOW.SetPosition(_windowPos);
	//}

}

void Engine::Start()
{
	//ImGui::CreateContext();
	onEngineStart.Broadcast();

#ifdef _MSC_VER
	InitConfig();
#endif


	M_INPUT.WindowClose.AddListener([this](){
		//SaveWindowInfo();
		MAIN_WINDOW.Close();
		M_LEVEL.SetLevel(nullptr);
		});

	//if (!ImGui::SFML::Init(MAIN_WINDOW.GetRenderWindow())) LOG_ERROR("ImGui has not being correctly initialize");
	//ImGui::GetIO().IniFilename = nullptr; // Suppr imgui.ini file
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
		UpdateEvent();
		M_COLLISION.Update();
	}
}

void Engine::Stop()
{
	onEngineStop.Broadcast();
	M_LEVEL.Destroy();
	Logger::Shutdown();
	//ImGui::SFML::Shutdown();
}


void Engine::UpdateEvent()
{
	const std::optional<sf::Event>& _event = MAIN_WINDOW.PollEvent();
	M_INPUT.Update(_event);

	//ImGui::SFML::ProcessEvent(MAIN_WINDOW.GetRenderWindow(), _event.value());
}

//void Engine::SaveWindowInfo()
//{
//	rapidjson::Document _doc;
//	_doc.SetObject();
//
//	rapidjson::Document::AllocatorType& _allocator = _doc.GetAllocator();
//
//	_doc.AddMember("Size", MAIN_WINDOW.GetSize().ToJson(_allocator), _allocator);
//	_doc.AddMember("Pos", MAIN_WINDOW.GetPosition().ToJson(_allocator), _allocator);
//
//	rapidjson::StringBuffer _buffer;
//	rapidjson::Writer<rapidjson::StringBuffer> _writer(_buffer);
//	_doc.Accept(_writer);
//
//	std::filesystem::create_directories(windowSaveDir);
//	std::ofstream _ofs(windowSaveDir + windowSaveFileName);
//
//	_ofs << _buffer.GetString();
//	_ofs.close();
//}