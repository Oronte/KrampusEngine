#include "Core/Engine.h"
#include "Managers/AudioManager.h"
#include "Managers/LevelManager.h"


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
	Krampus::Engine _engine;
	Krampus::Level* _level = _engine.GetLevelManager()->SetLevel<Krampus::Level>();

	//Krampus::Music _sound = M_AUDIO.CreateMusic("Shoryuken", Krampus::AudioExtensionType::MP3);
	//_sound.SetRelativeToListener(false);
	//_sound.SetPosition(Krampus::FVector3(10.0f));
	//_sound.SetLoop(true);
	//_sound.SetVolume(100.0f);
	//_sound.Play();

	_engine.Start();
}