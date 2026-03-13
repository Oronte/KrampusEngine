//Core engine includes
#include "Core/Engine.h"
#include "Utilities/MemoryLeaksDetection.h"
#include "Utilities/KrampusInclude.h"

//Custom game includes
#include "Managers/LevelManager.h"
#include "MainMenu.h"

#define _CRTDBG_MAP_ALLOC


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
	//Krampus::SetupMemoryLeakDetection();

	try
	{
		StartGame();
	}
	catch (const Krampus::Exception& _exception)
	{
		std::cerr << TXT_DARK_RED << _exception.What() << COLOR_RESET << std::endl;
	}

	//Krampus::TeardownMemoryLeakDetection();

	return EXIT_SUCCESS;
}

#endif

void StartGame()
{
	Krampus::Engine _engine;
	_engine.GetLevelManager()->SetLevel<MainMenu>();

	_engine.Start();
}