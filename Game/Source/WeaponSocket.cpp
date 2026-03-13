#include "WeaponSocket.h"

WeaponSocket::WeaponSocket(Krampus::Level* _level, Player* _grabber)
	: Krampus::Actor(_level)
{
	grabable = CreateComponent<GrabableComponent>(_grabber);
}

void WeaponSocket::SetGraber(Player* _grabber)
{
	grabable->SetGraber(_grabber);
}
