#include "Engine.h"
#include "Managers/TimerManager.h"
#include "Managers/InputManager.h"
#include "Managers/LevelManager.h"
#include "Graphics/Window/MainWindow.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui-SFML.h"

using namespace Krampus;

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

	onWindowClosed.AddListener([this](){
		//SaveWindowInfo();
		MAIN_WINDOW.Close();
		M_LEVEL.SetLevel(nullptr);
		});

	M_INPUT.A.AddListener([]() {LOG_MSG("A"); });
}

void Engine::Start()
{
	ImGui::CreateContext();
	onEngineStart.Broadcast();
	if (!ImGui::SFML::Init(MAIN_WINDOW.GetRenderWindow())) LOG_ERROR("ImGui has not being correctly initialize");
	ImGui::GetIO().IniFilename = nullptr; // Suppr imgui.ini file
	Logger::Init();
	Update();
	Stop();
}

void Engine::Update()
{
	while (Level* _currentLevel = M_LEVEL.GetCurrentLevel())
	{
		UpdateEvent();
		M_INPUT.Update(MAIN_WINDOW);
		_currentLevel->Update(M_TIMER.Update());
	}
}

void Engine::Stop()
{
	onEngineStop.Broadcast();
	M_LEVEL.Destroy();
	Logger::Shutdown();
	ImGui::SFML::Shutdown();
}


void Engine::UpdateEvent()
{
	const std::optional<sf::Event> _event = MAIN_WINDOW.PollEvent();
	if (!_event.has_value()) return;


	ImGui::SFML::ProcessEvent(MAIN_WINDOW.GetRenderWindow(), _event.value());

	if (_event->is<sf::Event::FocusGained>()) onFocusGained.Broadcast();
	if (_event->is<sf::Event::FocusLost>()) onFocusLost.Broadcast();
	if (_event->is<sf::Event::Resized>()) onWindowResized.Broadcast(_event->getIf<sf::Event::Resized>()->size);
	if (_event->is<sf::Event::MouseMoved>()) onMouseMoved.Broadcast(_event->getIf<sf::Event::MouseMoved>()->position);
	if (_event->is<sf::Event::MouseEntered>()) onMouseEntered.Broadcast();
	if (_event->is<sf::Event::MouseLeft>()) onMouseExit.Broadcast();
	if (_event->is<sf::Event::Closed>()) onWindowClosed.Broadcast();
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