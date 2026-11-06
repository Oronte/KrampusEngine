#include "NetworkManager.h"
#include "NetworkProtocol.h"
#include "Core/Engine.h"
#include <random>

using namespace Krampus;
using namespace Krampus::NetProtocol;


void NetworkManager::UpdateClient(const float _dt)
{
	if (role == NetRole::Connecting)
	{
		connectAge += _dt;
		if (steamPending)
		{
			const NetSocket::State _state = steamPending->GetState();
			if (_state == NetSocket::State::Connected)
			{
				server = std::move(steamPending);
				OnServerConnected();
			}
			else if (_state == NetSocket::State::Failed)
			{
				LOG_WARNING("Network: could not reach the host through Steam");
				EndGame("could not reach the host through Steam (is it still hosting?)", true);
				return;
			}
		}
		else if (connecting.valid() && connecting.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
		{
			ConnectResult _result = connecting.get();
			connecting = { };
			if (_result.status == sf::Socket::Status::Done)
			{
				server = std::move(_result.socket);
				OnServerConnected();
			}
			else
			{
				LOG_WARNING("Network: could not reach the host");
				EndGame("could not reach the host (wrong address, not hosting, or a firewall)", true);
				return;
			}
		}

		if (server) ReadServerSocket();
		// Steam: finding a route through the relays can take several seconds by itself.
		if (role == NetRole::Connecting && connectAge > (steamMode ? 20.f : 8.f))
		{
			LOG_WARNING("Network: the host did not answer");
			EndGame("the host did not answer", true);
		}
		return;
	}

	ReadServerSocket();
	if (role != NetRole::Client) return;
	ReadUdp();
	if (role != NetRole::Client) return;

	silence += _dt;
	if (silence > timeout)
	{
		LOG_WARNING("Network: connection to the host lost (timeout)");
		EndGame("connection to the host lost (timeout)", false);
		return;
	}

	pingTimer += _dt;
	if (pingTimer >= 1.f)
	{
		pingTimer = 0.f;
		sf::Packet _ping;
		_ping << static_cast<std::uint8_t>(PPing) << uptime << pingMs;
		SendPacket(*server, _ping);
	}

	// Open the UDP link: tell the host from which port we talk, until it answers.
	udpTimer += _dt;
	if (!udpReady && !steamMode && udpTimer >= 0.5f)
	{
		udpTimer = 0.f;
		sf::Packet _hello;
		_hello << static_cast<std::uint8_t>(PUdpHello) << Magic << static_cast<std::int32_t>(localId);
		SendUdp(_hello, serverAddress, serverPort);
	}
}

void NetworkManager::OnServerConnected()
{
	server->setBlocking(false);
	sf::Packet _hello;
	_hello << static_cast<std::uint8_t>(PHello) << Magic << Protocol << playerName;
	SendPacket(*server, _hello);
	connectAge = 0.f;	// now waiting for the Welcome
}

void NetworkManager::ReadServerSocket()
{
	for (int _i = 0; _i < 64 && server; ++_i)
	{
		sf::Packet _packet;
		const sf::Socket::Status _status = server->receive(_packet);
		if (_status == sf::Socket::Status::Done)
		{
			CountReceived(_packet);
			silence = 0.f;
			HandleServerPacket(_packet);
		}
		else if (_status == sf::Socket::Status::Disconnected || _status == sf::Socket::Status::Error)
		{
			if (role == NetRole::Connecting)
				EndGame("the host closed the connection while joining", true);
			else
			{
				LOG_WARNING("Network: the host closed the connection");
				EndGame("the host closed the game", false);
			}
			return;
		}
		else
			return;
	}
}

void NetworkManager::HandleServerPacket(sf::Packet& _packet)
{
	std::uint8_t _type = 0;
	if (!(_packet >> _type)) return;

	switch (_type)
	{
	case PWelcome:
	{
		std::int32_t _id = 0, _count = 0;
		std::string _hostName;
		if (!(_packet >> _id >> _hostName >> _count)) return;

		localId = _id;
		players.clear();
		players.push_back(NetPlayer{ 0, _hostName, 0.f, true, false });
		for (std::int32_t _i = 0; _i < _count; ++_i)	// the count comes from the network: stop at the end of the packet
		{
			std::int32_t _other = 0;
			std::string _name;
			if (!(_packet >> _other >> _name)) break;
			players.push_back(NetPlayer{ _other, _name, 0.f, false, false });
		}
		players.push_back(NetPlayer{ localId, playerName, 0.f, false, true });
		role = NetRole::Client;
		silence = 0.f;
		shared.clear();		// the host sends its values right after
		LOG_MSG("Network: connected to " + _hostName + " as player " + std::to_string(localId));

		std::vector<int> _others;
		for (const NetPlayer& _player : players)
			if (_player.id != localId) _others.push_back(_player.id);
		onConnected.Broadcast();
		for (const int _other : _others)
		{
			if (role != NetRole::Client) break;
			onClientJoined.Broadcast(_other);
		}
		onPlayersChanged.Broadcast();
		break;
	}
	case PReject:
	{
		std::string _reason;
		_packet >> _reason;
		LOG_WARNING("Network: the host refused the connection (" + _reason + ")");
		EndGame("refused by the host: " + _reason, true);
		break;
	}
	case PKick:
	{
		std::string _reason;
		_packet >> _reason;
		LOG_WARNING("Network: kicked by the host (" + _reason + ")");
		EndGame("kicked: " + _reason, false);
		break;
	}
	case PJoined:
	{
		std::int32_t _id = 0;
		std::string _name;
		if (!(_packet >> _id >> _name)) return;
		if (std::none_of(players.begin(), players.end(), [&](const NetPlayer& _player) { return _player.id == _id; }))
			players.push_back(NetPlayer{ _id, _name, 0.f, false, false });
		LOG_MSG("Network: player " + std::to_string(_id) + " (" + _name + ") joined");
		onClientJoined.Broadcast(_id);
		onPlayersChanged.Broadcast();
		break;
	}
	case PLeft:
	{
		std::int32_t _id = 0;
		if (!(_packet >> _id)) return;
		const std::string _name = GetPlayerName(_id);
		players.erase(std::remove_if(players.begin(), players.end(), [&](const NetPlayer& _player) { return _player.id == _id; }), players.end());
		LOG_MSG("Network: player " + std::to_string(_id) + " (" + _name + ") left");
		onClientLeft.Broadcast(_id);
		onPlayersChanged.Broadcast();
		break;
	}
	case PPlayers:
	{
		std::int32_t _count = 0;
		if (!(_packet >> _count)) return;
		std::vector<NetPlayer> _list;
		for (std::int32_t _i = 0; _i < _count; ++_i)
		{
			NetPlayer _player;
			std::int32_t _id = 0;
			if (!(_packet >> _id >> _player.name >> _player.pingMs)) break;
			_player.id = _id;
			_list.push_back(std::move(_player));
		}
		SetPlayersFromHost(std::move(_list));
		break;
	}
	case PMessage:
	{
		NetMessage _message;
		int _target = ToAll;
		if (!ReadMessage(_packet, _message, _target)) return;
		_message.reliable = true;
		Deliver(_message);
		break;
	}
	case PPong:
	{
		float _time = 0.f;
		if (_packet >> _time) pingMs = (uptime - _time) * 1000.f;
		break;
	}
	default:
		break;
	}
}

