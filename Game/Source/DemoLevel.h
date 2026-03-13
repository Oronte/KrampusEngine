#pragma once
#include "GameFramework/Level.h"
#include "Audio/Music.h"

using namespace Krampus;

class DemoLevel : public Level
{
	Music	backgroundMusic;

public:
	DemoLevel(Engine* _world);

protected:
	virtual void InitLevel() override;
	virtual void Unload() override;
};

