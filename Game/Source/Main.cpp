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
#include "Actors/Collider2D.h"
#include "TestPhysicsLevel.h"


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
	Krampus::Level* _level = M_LEVEL.SetLevel<Krampus::Level>();
	Krampus::Engine& _engine = ENGINE;

	_level->SpawnWidget<Krampus::ButtonWidget>(Krampus::CircleShapeData());

	M_INPUT.A.onPress.AddListener([&]() {M_LEVEL.SetLevel<Krampus::TestPhysicsLevel>(); });

	M_TIMER.SetMaxFrameRate(0);

	_engine.Start();
}