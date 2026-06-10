#include "Actor.h"
#include "GameFramework/Level.h"
#include "Managers/TimerManager.h"

Krampus::Engine* Krampus::Actor::GetWorld() const
{
	return level->GetWorld();
}

Float Krampus::Actor::GetDeltaTime() const
{
	return level->GetTimerManagerRef().GetDeltaTime();
}

Krampus::Actor::Actor(Level* _level)
{
	name = NAME_OF(Actor);
	level = _level;
}

void Krampus::Actor::SetActive(const Bool& _status)
{
	IGameObject::SetActive(_status);

	for (const std::unique_ptr<Component>& _component : components)
	{
		_component->SetActive(_status);
	}
}

Krampus::Timer* Krampus::Actor::CreateTimer(const std::function<void()>& _callback, const Float& _duration, const Bool& _isLoop, const Bool& _startRunning)
{
	return level->GetTimerManagerRef().CreateTimer(_callback, _duration, _isLoop, _startRunning);
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

void Krampus::Actor::Tick(const Float& _deltaTime)
{
	for (const std::unique_ptr<Component>& _component : components)
	{
		if (_component->IsActive()) 
			_component->Tick(_deltaTime);
	}
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

std::string Krampus::Actor::ToString() const
{
	return name + " -> Transform = " + transform.ToString();
}

void Krampus::Actor::SetActorPosition(const FVector2& _newPosition)
{
	for (Actor* _actor : children)
		_actor->Move(_newPosition - GetActorPosition());
	transform.position = _newPosition;
	onMove.Broadcast(_newPosition);
}

Krampus::FVector2 Krampus::Actor::GetActorPosition() const
{
	return transform.position;
}

void Krampus::Actor::SetActorRotation(const Angle& _newRotation)
{
	for (Actor* _actor : children)
	{
		const Angle& _deltaRot = _newRotation - GetActorRotation();
		_actor->SetActorPosition(_actor->GetActorPosition().FVector2::RotateAround(GetActorPosition(), _deltaRot));
		_actor->Rotate(_deltaRot);
	}
	transform.rotation = _newRotation;
	onRotate.Broadcast(_newRotation);
}

Krampus::Angle Krampus::Actor::GetActorRotation() const
{
	return transform.rotation;
}

void Krampus::Actor::SetActorScale(const FVector2& _newScale)
{
	for (Actor* _actor : children)
		_actor->Scale(_newScale - GetActorScale());
	transform.scale = _newScale;
	onScale.Broadcast(_newScale);
}

Krampus::FVector2 Krampus::Actor::GetActorScale() const
{
	return transform.scale;
}
