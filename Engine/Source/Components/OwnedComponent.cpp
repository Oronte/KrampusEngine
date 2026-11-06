#include "OwnedComponent.h"
#include "Actors/Actor.h"
#include "UI/Widgets/Widget.h"
#include "GameFramework/Level.h"

template <typename Owner>
Krampus::Timer* Krampus::OwnedComponent<Owner>::CreateTimer(const std::function<void()>& _callback, const Float& _duration, const Bool& _isLoop, const Bool& _startRunning)
{
	return owner->CreateTimer(_callback, _duration, _isLoop, _startRunning);
}

template <typename Owner> Krampus::Level*  Krampus::OwnedComponent<Owner>::GetLevel() const		{ return owner->GetLevel(); }
template <typename Owner> Krampus::Engine* Krampus::OwnedComponent<Owner>::GetWorld() const		{ return GetLevel()->GetWorld(); }
template <typename Owner> Float			   Krampus::OwnedComponent<Owner>::GetDeltaTime() const	{ return owner->GetDeltaTime(); }

template <typename Owner>
std::string Krampus::OwnedComponent<Owner>::ToString() const
{
	return owner ? name + " : owner = " + owner->ToString() : name + " with no owner";
}

template <typename Owner> void			   Krampus::OwnedComponent<Owner>::SetPosition(const FVector2& _position)	{ owner->SetPosition(_position); }
template <typename Owner> Krampus::FVector2 Krampus::OwnedComponent<Owner>::GetPosition() const					{ return owner->GetPosition(); }
template <typename Owner> void			   Krampus::OwnedComponent<Owner>::SetRotation(const Angle& _rotation)		{ owner->SetRotation(_rotation); }
template <typename Owner> Krampus::Angle	   Krampus::OwnedComponent<Owner>::GetRotation() const					{ return owner->GetRotation(); }
template <typename Owner> void			   Krampus::OwnedComponent<Owner>::SetScale(const FVector2& _scale)		{ owner->SetScale(_scale); }
template <typename Owner> Krampus::FVector2 Krampus::OwnedComponent<Owner>::GetScale() const					{ return owner->GetScale(); }

template class Krampus::OwnedComponent<Krampus::Actor>;
template class Krampus::OwnedComponent<Krampus::Widget>;
