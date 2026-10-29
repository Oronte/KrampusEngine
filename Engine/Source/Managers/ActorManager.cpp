#include "ActorManager.h"

void Krampus::ActorManager::BeginPlay()
{
	isPlaying = true;

	// Actors spawned from a BeginPlay get theirs in CreateActor: they are not in the snapshot.
	for (Actor* _actor : Snapshot())
		if (!_actor->IsPendingDestroy()) _actor->BeginPlay();
}

void Krampus::ActorManager::Update(const Float& _deltaTime)
{
	for (Actor* _actor : Snapshot())
		if (_actor->IsActive() && !_actor->IsPendingDestroy()) _actor->Tick(_deltaTime);
}

void Krampus::ActorManager::Deconstruct()
{
	for (Actor* _actor : Snapshot())
		if (!_actor->IsPendingDestroy()) _actor->Deconstruct();
}

void Krampus::ActorManager::BeginDestroy()
{
	isPlaying = false;

	for (Actor* _actor : Snapshot())
		_actor->BeginDestroy();
}
