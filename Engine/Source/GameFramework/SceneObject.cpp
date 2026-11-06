#include "SceneObject.h"
#include "Actors/Actor.h"
#include "UI/Widgets/Widget.h"
#include "GameFramework/Level.h"
#include "Managers/TimerManager.h"

template <typename Self, typename ComponentType>
Krampus::Engine* Krampus::SceneObject<Self, ComponentType>::GetWorld() const
{
	return level->GetWorld();
}

template <typename Self, typename ComponentType>
Float Krampus::SceneObject<Self, ComponentType>::GetDeltaTime() const
{
	return level->GetTimerManager().GetDeltaTime();
}

template <typename Self, typename ComponentType>
Krampus::Timer* Krampus::SceneObject<Self, ComponentType>::CreateTimer(const std::function<void()>& _callback, const Float& _duration, const Bool& _isLoop, const Bool& _startRunning)
{
	return level->GetTimerManager().CreateTimer(_callback, _duration, _isLoop, _startRunning);
}

// ---- Hierarchy

template <typename Self, typename ComponentType>
void Krampus::SceneObject<Self, ComponentType>::AddChild(Self* _child)
{
	if (!_child || _child == this) return;
	for (const SceneObject* _above = parent; _above; _above = _above->parent)
		if (_above == _child)
		{
			LOG_WARNING(_child->GetName() + " is above " + GetName() + " in the hierarchy, it cannot be its child");
			return;
		}
	SceneObject* _node = _child;
	if (_node->parent && _node->parent != this) _node->parent->RemoveChild(_child);	// one parent at a time
	_node->parent = static_cast<Self*>(this);
	_node->localTransform = RelativeTransform(transform, _node->transform);			// it stays where it is
	if (std::find(children.begin(), children.end(), _child) == children.end())
		children.push_back(_child);
}

template <typename Self, typename ComponentType>
void Krampus::SceneObject<Self, ComponentType>::RemoveChild(Self* _child)
{
	std::erase(children, _child);
	SceneObject* _node = _child;
	if (_node && _node->parent == this)
	{
		_node->parent = nullptr;
		_node->localTransform = _node->transform;
	}
}

// ---- Components

template <typename Self, typename ComponentType>
void Krampus::SceneObject<Self, ComponentType>::RemoveComponentInstance(ComponentType* _component)
{
	if (!_component || IsRemoved(_component)) return;
	LOG(VerbosityType::Verbose, "Component: " + std::string(typeid(*_component).name()) + " removed from " + GetName());
	_component->Deconstruct();
	_component->BeginDestroy();
	OnComponentRemoved(_component);
	// A component may remove itself (or another one) from its Tick: deleting it now would break the walk of the components.
	if (componentLoops > 0) removedComponents.push_back(_component);
	else std::erase_if(components, [&](const std::unique_ptr<ComponentType>& _item) { return _item.get() == _component; });
}

// ---- Lifecycle

template <typename Self, typename ComponentType>
void Krampus::SceneObject<Self, ComponentType>::Construct()
{
	// The components created in the constructor were waiting for this (CreateComponent): each one is constructed once.
	if (constructed) return;
	constructed = true;
	ForEachComponent([](ComponentType* _component) { _component->Construct(); });
}

template <typename Self, typename ComponentType>
void Krampus::SceneObject<Self, ComponentType>::Deconstruct()
{
	ForEachComponent([](ComponentType* _component) { _component->Deconstruct(); });
}

template <typename Self, typename ComponentType>
void Krampus::SceneObject<Self, ComponentType>::BeginPlay()
{
	ForEachComponent([](ComponentType* _component) { _component->BeginPlay(); });
}

template <typename Self, typename ComponentType>
void Krampus::SceneObject<Self, ComponentType>::Tick(const Float& _deltaTime)
{
	ForEachComponent([&](ComponentType* _component) { if (_component->IsActive()) _component->Tick(_deltaTime); });
}

template <typename Self, typename ComponentType>
void Krampus::SceneObject<Self, ComponentType>::BeginDestroy()
{
	ForEachComponent([](ComponentType* _component) { _component->BeginDestroy(); });
}

template <typename Self, typename ComponentType>
void Krampus::SceneObject<Self, ComponentType>::Destroy()
{
	if (pendingDestroy) return;	// a second Destroy would deconstruct the components twice
	pendingDestroy = true;
	Deconstruct();
	OnDestroy();

	// Out of the hierarchy: the parent would move a deleted object. The children stay, without a parent.
	if (parent) parent->RemoveChild(static_cast<Self*>(this));
	for (SceneObject* _child : children)
	{
		_child->parent = nullptr;
		_child->localTransform = _child->transform;
	}
	children.clear();
}

template <typename Self, typename ComponentType>
void Krampus::SceneObject<Self, ComponentType>::SetActive(const Bool& _status)
{
	IGameObject::SetActive(_status);
	for (const std::unique_ptr<ComponentType>& _component : components)
		_component->SetActive(_status);
}

template <typename Self, typename ComponentType>
std::string Krampus::SceneObject<Self, ComponentType>::ToString() const
{
	return name + " -> Transform = " + transform.ToString();
}

// ---- Transform

template <typename Self, typename ComponentType>
void Krampus::SceneObject<Self, ComponentType>::SetPosition(const FVector2& _position)
{
	transform.position = _position;
	WorldChanged();
	onMove.Broadcast(_position);	// even when it did not change: a setter always tells
}

template <typename Self, typename ComponentType>
void Krampus::SceneObject<Self, ComponentType>::SetRotation(const Angle& _rotation)
{
	transform.rotation = _rotation;
	WorldChanged();
	onRotate.Broadcast(_rotation);
}

template <typename Self, typename ComponentType>
void Krampus::SceneObject<Self, ComponentType>::SetScale(const FVector2& _scale)
{
	transform.scale = _scale;
	WorldChanged();
	onScale.Broadcast(_scale);
}

template <typename Self, typename ComponentType>
void Krampus::SceneObject<Self, ComponentType>::SetLocalTransform(const Transform& _local)
{
	localTransform = _local;
	const Transform _old = transform;
	FollowParent();
	Propagate(_old);
}

template <typename Self, typename ComponentType>
void Krampus::SceneObject<Self, ComponentType>::WorldChanged()
{
	localTransform = parent ? RelativeTransform(static_cast<const SceneObject*>(parent)->transform, transform) : transform;
	Propagate(transform);	// only the children: nothing differs for this one
}

template <typename Self, typename ComponentType>
void Krampus::SceneObject<Self, ComponentType>::FollowParent()
{
	transform = parent ? ComposeTransform(static_cast<const SceneObject*>(parent)->transform, localTransform) : localTransform;
}

template <typename Self, typename ComponentType>
void Krampus::SceneObject<Self, ComponentType>::Propagate(const Transform& _old)
{
	const Transform _before = _old;	// _old may be this->transform: keep a copy before the children run
	for (SceneObject* _child : children)
	{
		const Transform _childOld = _child->transform;
		_child->FollowParent();
		_child->Propagate(_childOld);
	}
	if (transform.position.x != _before.position.x || transform.position.y != _before.position.y) onMove.Broadcast(transform.position);
	if (transform.rotation != _before.rotation) onRotate.Broadcast(transform.rotation);
	if (transform.scale.x != _before.scale.x || transform.scale.y != _before.scale.y) onScale.Broadcast(transform.scale);
}

template class Krampus::SceneObject<Krampus::Actor, Krampus::Component>;
template class Krampus::SceneObject<Krampus::Widget, Krampus::WidgetComponent>;
