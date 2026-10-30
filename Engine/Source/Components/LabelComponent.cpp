#include "LabelComponent.h"
#include "Actors/Actor.h"
#include "Graphics/Window.h"
#include "GameFramework/Level.h"

Krampus::LabelComponent::LabelComponent(Actor* _owner, const TextData& _data)
	: Component(_owner), text(_data)
{ 
	name = NAME_OF(LabelComponent);

	onOwnerMoveHandle = _owner->onMove.AddListener([this](FVector2 _newPos)
		{
			text.SetPosition(_newPos);
		});
	onOwnerRotateHandle = _owner->onRotate.AddListener([this](Angle _newRot)
		{
			text.SetRotation(_newRot);
		});
	onOwnerScaleHandle = _owner->onScale.AddListener([this](FVector2 _newScale)
		{
			text.SetScale(_newScale);
		});
	// Start where the owner is: its onMove / onRotate / onScale only fire when it changes afterwards.
	text.SetPosition(GetPosition());
	text.SetRotation(GetRotation());
	text.SetScale(GetScale());
}

void Krampus::LabelComponent::Draw(Window& _window)
{
	if (!isActive) return;
	_window.Draw(text);
}

void Krampus::LabelComponent::DrawDebug()
{
#ifdef DEBUG
	if (!IsDebugging()) return;
	{ const FRect _bounds = GetGlobalBounds(); Debug::DrawDebugRect(GetLevel(), _bounds.GetPosition() + _bounds.GetSize() / 2.0f, _bounds.GetSize(), Angle(), Color::Green()); }
#endif

}

void Krampus::LabelComponent::Construct()
{
	Component::Construct();

	text.SetPosition(GetPosition());
	text.SetRotation(GetRotation());
	text.SetScale(GetScale());
	GetLevel()->GetCameraManager().AddToWorld(this);
}

void Krampus::LabelComponent::Deconstruct()
{
	Component::Deconstruct();

	GetLevel()->GetCameraManager().RemoveToWorld(this);
}

void Krampus::LabelComponent::SetZOrder(const uint8_t& _zOrder)
{
	const uint8_t& _oldZOrder = GetZOrder();
	IDrawable::SetZOrder(_zOrder);
	GetLevel()->GetCameraManager().ChangeWorldZOrder(this, _oldZOrder);
}

void Krampus::LabelComponent::SetZOrder(const ZOrder& _zOrder)
{
	const uint8_t& _oldZOrder = GetZOrder();
	IDrawable::SetZOrder(_zOrder);
	GetLevel()->GetCameraManager().ChangeWorldZOrder(this, _oldZOrder);
}

std::string Krampus::LabelComponent::ToString() const
{
	return name + " -> Label = \"" + text.GetString() + "\"";
}
