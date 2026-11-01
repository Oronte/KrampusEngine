#include "Widget.h"
#include "GameFramework/Level.h"
#include "Managers/TimerManager.h"

Krampus::Engine* Krampus::Widget::GetWorld() const
{
	return level->GetWorld();
}

Float Krampus::Widget::GetDeltaTime() const
{
	return level->GetTimerManager().GetDeltaTime();
}

Krampus::Widget::Widget(Level* _level)
{
	name = NAME_OF(Widget);
	level = _level;
}

void Krampus::Widget::SetActive(const Bool& _status)
{
	IGameObject::SetActive(_status);

	for (const std::unique_ptr<WidgetComponent>& _component : components)
	{
		_component->SetActive(_status);
	}
}

Krampus::Timer* Krampus::Widget::CreateTimer(const std::function<void()>& _callback, const Float& _duration, const Bool& _isLoop, const Bool& _startRunning)
{
	return level->GetTimerManager().CreateTimer(_callback, _duration, _isLoop, _startRunning);
}

void Krampus::Widget::Construct()
{
	// The components created in the constructor were waiting for this (CreateComponent): each one is constructed once.
	if (constructed) return;
	constructed = true;
	for (const std::unique_ptr<WidgetComponent>& _component : components)
	{
		_component->Construct();
	}
}

void Krampus::Widget::Deconstruct()
{
	for (const std::unique_ptr<WidgetComponent>& _component : components)
	{
		_component->Deconstruct();
	}
}

void Krampus::Widget::BeginPlay()
{
	for (const std::unique_ptr<WidgetComponent>& _component : components)
	{
		_component->BeginPlay();
	}
}

void Krampus::Widget::Tick(const Float& _deltaTime)
{
	for (const std::unique_ptr<WidgetComponent>& _component : components)
	{
		if (_component->IsActive()) 
			_component->Tick(_deltaTime);
	}
}

void Krampus::Widget::BeginDestroy()
{
	for (const std::unique_ptr<WidgetComponent>& _component : components)
	{
		_component->BeginDestroy();
	}
}

void Krampus::Widget::Destroy()
{
	if (pendingDestroy) return;	// a second Destroy would deconstruct the components twice
	pendingDestroy = true;
	Deconstruct();
	level->GetHUD().MarkForDeleteWidget(this);

	// Out of the hierarchy: the parent would move a deleted widget. The children stay in the HUD, without a parent.
	if (parent) parent->RemoveChild(this);
	for (Widget* _child : children)
	{
		_child->parent = nullptr;
		_child->localTransform = _child->transform;
	}
	children.clear();
}

std::string Krampus::Widget::ToString() const
{
	return name + " -> Transform = " + transform.ToString();
}

void Krampus::Widget::SetPosition(const FVector2& _newPosition)
{
	transform.position = _newPosition;
	WorldChanged();
	onMove.Broadcast(_newPosition);		// even when it did not change: a setter always tells
}

Krampus::FVector2 Krampus::Widget::GetPosition() const
{
	return transform.position;
}

void Krampus::Widget::SetRotation(const Angle& _newRotation)
{
	transform.rotation = _newRotation;
	WorldChanged();
	onRotate.Broadcast(_newRotation);
}

Krampus::Angle Krampus::Widget::GetRotation() const
{
	return transform.rotation;
}

void Krampus::Widget::SetScale(const FVector2& _newScale)
{
	transform.scale = _newScale;
	WorldChanged();
	onScale.Broadcast(_newScale);
}

void Krampus::Widget::SetLocalTransform(const Transform& _local)
{
	localTransform = _local;
	const Transform _old = transform;
	FollowParent();
	Propagate(_old);
}

void Krampus::Widget::WorldChanged()
{
	localTransform = parent ? RelativeTransform(parent->transform, transform) : transform;
	const Transform _same = transform;
	Propagate(_same);	// only the children: nothing differs for this one
}

void Krampus::Widget::FollowParent()
{
	transform = parent ? ComposeTransform(parent->transform, localTransform) : localTransform;
}

void Krampus::Widget::Propagate(const Transform& _old)
{
	for (Widget* _child : children)
	{
		const Transform _childOld = _child->transform;
		_child->FollowParent();
		_child->Propagate(_childOld);
	}
	if (transform.position.x != _old.position.x || transform.position.y != _old.position.y) onMove.Broadcast(transform.position);
	if (transform.rotation != _old.rotation) onRotate.Broadcast(transform.rotation);
	if (transform.scale.x != _old.scale.x || transform.scale.y != _old.scale.y) onScale.Broadcast(transform.scale);
}

Krampus::FVector2 Krampus::Widget::GetScale() const
{
	return transform.scale;
}
