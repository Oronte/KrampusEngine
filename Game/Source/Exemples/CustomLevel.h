#pragma once
#include "GameFramework/Level.h"

class CustomLevel: public Krampus::Level
{
public:
	CustomLevel(Krampus::Engine* _engine);

protected:
	virtual void InitLevel() override;
};

