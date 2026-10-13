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
}

void Krampus::LabelComponent::Draw(Window& _window)
{
	if (!isActive) return;
	_window.Draw(text);
	DrawDebug();
}

void Krampus::LabelComponent::DrawDebug()
{
#ifdef DEBUG
	if (!IsDebugging()) return;
	Debug::DrawDebugRect(GetLevel(), GetPosition(), GetLocalBounds().GetSize(), GetRotation(), Color::Green());
#endif

}

void Krampus::LabelComponent::Construct()
{
	Component::Construct();

	GetLevel()->GetCameraManagerRef().AddToScreen(this);
}

void Krampus::LabelComponent::Deconstruct()
{
	Component::Deconstruct();

	GetLevel()->GetCameraManagerRef().RemoveToScreen(this);
}

void Krampus::LabelComponent::SetZOrder(const uint8_t& _zOrder)
{
	const uint8_t& _oldZOrder = GetZOrder();
	IDrawable::SetZOrder(_zOrder);
	GetLevel()->GetCameraManagerRef().ChangeScreenZOrder(this, _oldZOrder);
}

void Krampus::LabelComponent::SetZOrder(const ZOrder& _zOrder)
{
	const uint8_t& _oldZOrder = GetZOrder();
	IDrawable::SetZOrder(_zOrder);
	GetLevel()->GetCameraManagerRef().ChangeScreenZOrder(this, _oldZOrder);
}

std::string Krampus::LabelComponent::ToString() const
{
	return name + " -> Label = \"" + text.GetString() + "\"";
}
