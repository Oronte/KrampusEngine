#pragma once
#include "GameFramework/Level.h"

class Engine;

class CustomLevel : public Krampus::Level
{
public:
	CustomLevel(Krampus::Engine* _engine) :
		Krampus::Level(_engine) { }

protected:
	virtual void InitLevel() override;

public:
	virtual void Load() override;
	virtual void Unload() override;
};

