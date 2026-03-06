#include "WeaponSocket.h"

WeaponSocket::WeaponSocket(Krampus::Level* _level, Krampus::Actor* _grabber)
	: Krampus::Actor(_level)
{
	grabable = CreateComponent<GrabableComponent>(_grabber);
}

void WeaponSocket::SetGraber(Krampus::Actor* _grabber)
{
	grabable->SetGraber(_grabber);
}
