#include "Actor.h"
#include "GameFramework/Level.h"
#include "Managers/TimerManager.h"

Krampus::Engine* Krampus::Actor::GetWorld() const
{
	return level->GetWorld();
}

float Krampus::Actor::GetDeltaTime() const
{
	return GetWorld()->GetTimerManager()->GetDeltaTime();
}

Krampus::Actor::Actor(Level* _level)
{
	level = _level;
}

void Krampus::Actor::SetActive(const bool& _status)
{
	IGameObject::SetActive(_status);

	for (const std::unique_ptr<Component>& _component : components)
	{
		_component->SetActive(_status);
	}
}

Krampus::Timer* Krampus::Actor::CreateTimer(const std::function<void()>& _callback, const float& _duration, const bool& _isLoop, const bool& _startRunning)
{
	return GetWorld()->GetTimerManager()->CreateTimer(_callback, _duration, _isLoop, _startRunning);
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
		if (_component->IsActive()) 
			_component->Tick(_deltaTime);
	}

	for (Actor* _actor : children)
	{
		_actor->transform.position += transform.position - oldTransform.position;

		const Angle& _deltaRot = transform.rotation - oldTransform.rotation;

		_actor->transform.position = 
			_actor->transform.position.FVector2::RotateAround(transform.position, _deltaRot);
		
		_actor->transform.rotation += _deltaRot;
		_actor->transform.scale += transform.scale - oldTransform.scale;
	}

	oldTransform = transform;
}

void Krampus::Actor::BeginDestroy()
{
	for (const std::unique_ptr<Component>& _component : components)
	{
		_component->BeginDestroy();
	}
}

void Krampus::Actor::Destroy()
{
	Deconstruct();
	level->GetActorManagerRef().MarkForDeleteActor(this);
}
