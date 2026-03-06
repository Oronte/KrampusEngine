#pragma once
#include "GameFramework/Level.h"

using namespace Krampus;

class DemoLevel : public Level
{
public:
	DemoLevel(Engine* _world);

protected:
	virtual void InitLevel() override;
};

