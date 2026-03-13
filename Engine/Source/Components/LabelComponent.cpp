#include "LabelComponent.h"
#include "Actors/Actor.h"
#include "Graphics/Window.h"
#include "GameFramework/Level.h"

Krampus::LabelComponent::LabelComponent(Actor* _owner, const TextData& _data)
	: Component(_owner), text(_data)
{ 
	name = NAME_OF(LabelComponent);
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
	Debug::DrawDebugRect(level, transform.position, GetLocalBounds().GetSize(), transform.rotation, Color::Green());
#endif

}

void Krampus::LabelComponent::Construct()
{
	Component::Construct();

	level->GetCameraManagerRef().AddToWindow(this);
}

void Krampus::LabelComponent::Tick(const Float& _deltaTime)
{
	Component::Tick(_deltaTime);

	text.SetTransform(transform);
}

void Krampus::LabelComponent::Deconstruct()
{
	Component::Deconstruct();

	level->GetCameraManagerRef().RemoveToWindow(this);
}

void Krampus::LabelComponent::SetZOrder(const uint8_t& _zOrder)
{
	const uint8_t& _oldZOrder = GetZOrder();
	IDrawable::SetZOrder(_zOrder);
	level->GetCameraManagerRef().ChangeZOrder(this, _oldZOrder);
}

void Krampus::LabelComponent::SetZOrder(const ZOrder& _zOrder)
{
	const uint8_t& _oldZOrder = GetZOrder();
	IDrawable::SetZOrder(_zOrder);
	level->GetCameraManagerRef().ChangeZOrder(this, _oldZOrder);
}

std::string Krampus::LabelComponent::ToString() const
{
	return name + " -> Label = \"" + text.GetString() + "\"";
}
