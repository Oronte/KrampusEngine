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

	_level->SpawnWidget<Krampus::ImageWidget>(Krampus::RectangleShapeData())->transform.position = Krampus::FVector2(200);

	_engine.Start();
}