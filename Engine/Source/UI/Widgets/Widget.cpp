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
}

std::string Krampus::Widget::ToString() const
{
	return name + " -> Transform = " + transform.ToString();
}

void Krampus::Widget::SetPosition(const FVector2& _newPosition)
{
	for (Widget* _widget : children)
		_widget->Move(_newPosition - GetPosition());
	transform.position = _newPosition;
	onMove.Broadcast(_newPosition);
}

Krampus::FVector2 Krampus::Widget::GetPosition() const
{
	return transform.position;
}

void Krampus::Widget::SetRotation(const Angle& _newRotation)
{
	for (Widget* _widget : children)
	{
		const Angle& _deltaRot = _newRotation - GetRotation();
		_widget->SetPosition(_widget->GetPosition().FVector2::RotateAround(GetPosition(), _deltaRot));
		_widget->Rotate(_deltaRot);
	}
	transform.rotation = _newRotation;
	onRotate.Broadcast(_newRotation);
}

Krampus::Angle Krampus::Widget::GetRotation() const
{
	return transform.rotation;
}

void Krampus::Widget::SetScale(const FVector2& _newScale)
{
	for (Widget* _widget : children)
		_widget->Scale(_newScale - GetScale());
	transform.scale = _newScale;
	onScale.Broadcast(_newScale);
}

Krampus::FVector2 Krampus::Widget::GetScale() const
{
	return transform.scale;
}
