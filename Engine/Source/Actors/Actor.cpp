#include "Actor.h"
#include "GameFramework/Level.h"


Krampus::Actor::Actor(Level* _level)
{
	level = _level;
}

void Krampus::Actor::SetActive(const bool& _status)
{
	Core::SetActive(_status);

	for (const std::unique_ptr<Component>& _component : components)
	{
		_component->SetActive(_status);
	}
}

void Krampus::Actor::Construct()
{
	for (const std::unique_ptr<Component>& _component : components)
	{
		_component->Construct();
	}
}

void Krampus::Actor::Deconstruct()
{
	for (const std::unique_ptr<Component>& _component : components)
	{
		_component->Deconstruct();
	}
}

void Krampus::Actor::BeginPlay()
{
	for (const std::unique_ptr<Component>& _component : components)
	{
		_component->BeginPlay();
	}
}

void Krampus::Actor::Tick(const float& _deltaTime)
{
	for (const std::unique_ptr<Component>& _component : components)
	{
		_component->Tick(_deltaTime);
	}
}

void Krampus::Actor::BeginDestroy()
{
	for (const std::unique_ptr<Component>& _component : components)
	{
		_component->BeginDestroy();
	}
	components.clear();
}

void Krampus::Actor::Destroy()
{
	toDelete = true;
}
