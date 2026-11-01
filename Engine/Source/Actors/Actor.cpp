#include "Actor.h"
#include "GameFramework/Level.h"
#include "Managers/TimerManager.h"
#include <chrono>
#include <random>

Krampus::Engine* Krampus::Actor::GetWorld() const
{
	return level->GetWorld();
}

Float Krampus::Actor::GetDeltaTime() const
{
	return level->GetTimerManager().GetDeltaTime();
}

Krampus::Actor::Actor(Level* _level)
{
	static std::mt19937_64 _random(std::random_device{ }() ^ static_cast<std::uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count()));
	name = NAME_OF(Actor);
	level = _level;
	do id = _random(); while (id == 0);	// 0 is never an actor
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
	return level->GetTimerManager().CreateTimer(_callback, _duration, _isLoop, _startRunning);
}

void Krampus::Actor::Construct()
{
	// The components created in the constructor were waiting for this (CreateComponent): each one is constructed once, with the
	// actor at its place in the level.
	if (constructed) return;
	constructed = true;
	ForEachComponent([](Component* _component) { _component->Construct(); });
}

void Krampus::Actor::Deconstruct()
{
	ForEachComponent([](Component* _component) { _component->Deconstruct(); });
}

void Krampus::Actor::BeginPlay()
{
	ForEachComponent([](Component* _component) { _component->BeginPlay(); });
}

void Krampus::Actor::Tick(const Float& _deltaTime)
{
	ForEachComponent([&](Component* _component)
		{
			if (_component->IsActive())
				_component->Tick(_deltaTime);
		});
}

void Krampus::Actor::BeginDestroy()
{
	ForEachComponent([](Component* _component) { _component->BeginDestroy(); });
}

void Krampus::Actor::Destroy()
{
	if (pendingDestroy) return;	// a second Destroy would deconstruct the components twice
	pendingDestroy = true;
	Deconstruct();
	level->GetActorManager().MarkForDeleteActor(this);
	ForgetReferencesTo(level, this);	// a reference to it is nullptr from now on, not a pointer to a deleted actor

	// Out of the hierarchy: the parent would move a deleted actor. The children stay in the level, without a parent.
	if (parent) parent->RemoveChild(this);
	for (Actor* _child : children)
	{
		_child->parent = nullptr;
		_child->localTransform = _child->transform;
	}
	children.clear();
}

std::string Krampus::Actor::ToString() const
{
	return name + " -> Transform = " + transform.ToString();
}

void Krampus::Actor::SetPosition(const FVector2& _newPosition)
{
	transform.position = _newPosition;
	WorldChanged();
	onMove.Broadcast(_newPosition);		// even when it did not change: a setter always tells
}

Krampus::FVector2 Krampus::Actor::GetPosition() const
{
	return transform.position;
}

void Krampus::Actor::SetRotation(const Angle& _newRotation)
{
	transform.rotation = _newRotation;
	WorldChanged();
	onRotate.Broadcast(_newRotation);
}

Krampus::Angle Krampus::Actor::GetRotation() const
{
	return transform.rotation;
}

void Krampus::Actor::SetScale(const FVector2& _newScale)
{
	transform.scale = _newScale;
	WorldChanged();
	onScale.Broadcast(_newScale);
}

Krampus::FVector2 Krampus::Actor::GetScale() const
{
	return transform.scale;
}

void Krampus::Actor::SetLocalTransform(const Transform& _local)
{
	localTransform = _local;
	const Transform _old = transform;
	FollowParent();
	Propagate(_old);
}

void Krampus::Actor::WorldChanged()
{
	localTransform = parent ? RelativeTransform(parent->transform, transform) : transform;
	const Transform _same = transform;
	Propagate(_same);	// only the children: nothing differs for this one
}

void Krampus::Actor::FollowParent()
{
	transform = parent ? ComposeTransform(parent->transform, localTransform) : localTransform;
}

void Krampus::Actor::Propagate(const Transform& _old)
{
	for (Actor* _child : children)
	{
		const Transform _childOld = _child->transform;
		_child->FollowParent();
		_child->Propagate(_childOld);
	}
	if (transform.position.x != _old.position.x || transform.position.y != _old.position.y) onMove.Broadcast(transform.position);
	if (transform.rotation != _old.rotation) onRotate.Broadcast(transform.rotation);
	if (transform.scale.x != _old.scale.x || transform.scale.y != _old.scale.y) onScale.Broadcast(transform.scale);
}
