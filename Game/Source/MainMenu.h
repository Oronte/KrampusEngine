#pragma once
#include "GameFramework/Level.h"
#include "Audio/Music.h"

using namespace Krampus;

class MainMenu : public Level
{
	Music						music;
	Event<>::ListenerHandle		onClickPlayHandle;
	Event<>::ListenerHandle		onClickQuitHandle;

public:
	MainMenu(Engine* _world);

protected:
	virtual void InitLevel() override;
	virtual void Unload() override;
};

