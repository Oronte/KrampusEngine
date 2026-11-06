#include "BasicLabel.h"
#include "Actors/Actor.h"
#include "UI/Widgets/Widget.h"
#include "Graphics/Window.h"
#include "GameFramework/Level.h"

template <typename Base>
Krampus::BasicLabel<Base>::BasicLabel(typename Base::OwnerType* _owner, const TextData& _data)
	: DrawnComponent<Base>(_owner), text(_data)
{
	onOwnerMoveHandle = _owner->onMove.AddListener([this](FVector2 _position) { text.SetPosition(_position); });
	onOwnerRotateHandle = _owner->onRotate.AddListener([this](Angle _rotation) { text.SetRotation(_rotation); });
	onOwnerScaleHandle = _owner->onScale.AddListener([this](FVector2 _scale) { text.SetScale(_scale); });
	// Start where the owner is: its events only fire when it changes afterwards.
	text.SetTransform(Transform(_owner->GetPosition(), _owner->GetRotation(), _owner->GetScale()));
}

template <typename Base>
void Krampus::BasicLabel<Base>::Draw(Window& _window)
{
	if (this->isActive) _window.Draw(text);
}

template <typename Base>
void Krampus::BasicLabel<Base>::DrawDebug()
{
#ifdef DEBUG
	if (!IsDebugging()) return;
	const FRect _bounds = GetGlobalBounds();
	Debug::DrawDebugRect(this->GetLevel(), _bounds.GetPosition() + _bounds.GetSize() / 2.0f, _bounds.GetSize(), Angle(), Color::Green());
#endif
}

template <typename Base>
std::string Krampus::BasicLabel<Base>::ToString() const
{
	return this->name + " -> Label = \"" + text.GetString() + "\"";
}

template class Krampus::BasicLabel<Krampus::Component>;
template class Krampus::BasicLabel<Krampus::WidgetComponent>;
