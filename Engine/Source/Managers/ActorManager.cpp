#include "ActorManager.h"

void Krampus::ActorManager::BeginPlay()
{
	isPlaying = true;

	for (const std::unique_ptr<Actor>& _actor : actors) 
		_actor->BeginPlay();
}

void Krampus::ActorManager::Update(const float& _deltaTime)
{
	for (const std::unique_ptr<Actor>& _actor : actors)
		if (_actor->IsActive()) _actor->Tick(_deltaTime);
}

void Krampus::ActorManager::Deconstruct()
{
	for (const std::unique_ptr<Actor>& _actor : actors)
		_actor->Deconstruct();
}

void Krampus::ActorManager::BeginDestroy()
{
	isPlaying = false;

	for (const std::unique_ptr<Actor>& _actor : actors)
		_actor->BeginDestroy();
}
