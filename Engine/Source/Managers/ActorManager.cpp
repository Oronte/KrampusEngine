#include "ActorManager.h"

void Krampus::ActorManager::BeginPlay()
{
	for (const std::unique_ptr<Actor>& _actor : actors) 
		_actor->BeginPlay();
}

void Krampus::ActorManager::Update(const float& _deltaTime)
{
	for (const std::unique_ptr<Actor>& _actor : actors)
		if (_actor->IsActive()) _actor->Tick(_deltaTime);
}

void Krampus::ActorManager::BeginDestroy()
{
	for (const std::unique_ptr<Actor>& _actor : actors)
		_actor->BeginDestroy();
}
