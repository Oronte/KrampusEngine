#pragma once
#include "NetworkManager.h"

// The wire format shared by the files NetworkManager*.cpp (packet types, targets, how values and messages are written).
namespace Krampus::NetProtocol
{
	inline constexpr std::uint32_t Magic		= 0x4B52414D;	// "KRAM"
	inline constexpr std::uint32_t Protocol	= 2;

	// First byte of every packet.
	enum PacketType : std::uint8_t
	{
		PHello		= 1,	// client -> host (TCP): magic, protocol, name
		PWelcome	= 2,	// host -> client (TCP): your id, host name, number of other players, then their id and name
		PReject		= 3,	// host -> client (TCP): reason
		PJoined		= 4,	// host -> clients (TCP): id, name
		PLeft		= 5,	// host -> clients (TCP): id
		PMessage	= 6,	// both ways, TCP or UDP: sender, target, name, values
		PPing		= 7,	// client -> host (TCP): time, the ping the client measured last
		PPong		= 8,	// host -> client (TCP): the same time
		PUdpHello	= 9,	// client -> host (UDP): magic, id
		PUdpAck		= 10,	// host -> client (UDP)
		PPlayers	= 11,	// host -> clients (TCP): count, then id, name, ping of every player
		PRename		= 12,	// client -> host (TCP): new name
		PKick		= 13,	// host -> client (TCP): reason
		PLanAsk		= 20,	// anybody -> broadcast (UDP, discovery port): magic, protocol
		PLanAnswer	= 21	// host -> asker (UDP): magic, protocol, game name, host name, port, players, max players, session
	};

	// Targets inside a packet: >= 0 a player, or one of these.
	inline constexpr int ToAll		= -1;
	inline constexpr int ToHost	= -2;
	inline constexpr int ToOthers	= -3;

	inline constexpr size_t MaxNameLength = 32;

	std::string CleanName(std::string _name, int _id);
	void WriteValue(sf::Packet& _packet, const PropertyValue& _value);
	bool ReadValue(sf::Packet& _packet, PropertyValue& _value);
	void WriteMessage(sf::Packet& _packet, const NetMessage& _message, int _target);
	bool ReadMessage(sf::Packet& _packet, NetMessage& _message, int& _target);
}
