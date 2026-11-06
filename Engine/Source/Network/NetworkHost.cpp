#include "NetworkManager.h"
#include "NetworkProtocol.h"
#include "Core/Engine.h"
#include <random>

using namespace Krampus;
using namespace Krampus::NetProtocol;

// ---- Loop

void NetworkManager::Update()
{
	const float _dt = std::min(clock.restart().asSeconds(), 0.25f);
	uptime += _dt;

	steam->Update();		// Steam callbacks: invitations and lobby lists arrive even when offline
	UpdateDiscovery(_dt);	// a search works offline
	UpdateStats(_dt);
	if (role == NetRole::Offline) return;

	if (role == NetRole::Host) UpdateHost(_dt);
	else UpdateClient(_dt);
}

void NetworkManager::UpdateStats(const float _dt)
{
	statsTimer += _dt;
	if (statsTimer < 1.f) return;

	stats.sentPerSecond = static_cast<float>(windowSent) / statsTimer;
	stats.receivedPerSecond = static_cast<float>(windowReceived) / statsTimer;
	windowSent = 0;
	windowReceived = 0;
	statsTimer = 0.f;
}

void NetworkManager::UpdateHost(const float _dt)
{
	// New connections wait for their Hello.
	for (int _i = 0; _i < 8; ++_i)
	{
		auto _socket = std::make_unique<TcpNetSocket>();
		if (listener.accept(_socket->tcp) != sf::Socket::Status::Done) break;
		_socket->setBlocking(false);
		joining.push_back(Joining{ std::move(_socket), 0.f });
	}
	if (steamMode)
		for (int _i = 0; _i < 8; ++_i)
		{
			std::unique_ptr<NetSocket> _socket = steam->Accept();
			if (!_socket) break;
			joining.push_back(Joining{ std::move(_socket), 0.f });
		}

	for (size_t _i = 0; _i < joining.size();)
	{
		Joining& _join = joining[_i];
		_join.age += _dt;

		bool _remove = _join.age > 5.f;
		bool _accepted = false;
		sf::Packet _packet;
		const sf::Socket::Status _status = _join.socket->receive(_packet);
		if (_status == sf::Socket::Status::Done)
		{
			CountReceived(_packet);
			HandleJoiningPacket(_join, _packet, _accepted);
			if (role != NetRole::Host) return;	// a listener of onClientJoined disconnected: the lists are gone
			_remove = true;
		}
		else if (_status == sf::Socket::Status::Disconnected || _status == sf::Socket::Status::Error)
			_remove = true;

		if (_remove) joining.erase(joining.begin() + _i);	// an accepted socket was moved into peers
		else ++_i;
	}

	// Clients: read, and drop the ones that went silent. Handling a packet calls the game, which may drop players or disconnect:
	// the peers are found again by id each time instead of walking the list (a dropped Peer is deleted).
	std::vector<int> _ids;
	for (const std::unique_ptr<Peer>& _peer : peers)
		_ids.push_back(_peer->id);

	std::vector<int> _lost;
	for (const int _id : _ids)
	{
		Peer* _peer = FindPeer(_id);
		if (!_peer) continue;

		_peer->silence += _dt;
		ReadHostSocket(*_peer);
		if (role != NetRole::Host) return;

		if (Peer* _still = FindPeer(_id); _still && _still->silence > timeout) _lost.push_back(_id);
	}
	for (const int _id : _lost)
	{
		if (Peer* _peer = FindPeer(_id)) DropPeer(*_peer, "timeout");
		if (role != NetRole::Host) return;
	}

	ReadUdp();
	if (role != NetRole::Host) return;

	// The list of players goes to everybody once a second: the pings change.
	playersTimer += _dt;
	if (playersTimer >= 1.f)
	{
		playersTimer = 0.f;
		BroadcastPlayers();
		onPlayersChanged.Broadcast();
	}
}

void NetworkManager::HandleJoiningPacket(Joining& _join, sf::Packet& _packet, bool& _accepted)
{
	std::uint8_t _type = 0;
	std::uint32_t _magic = 0, _protocol = 0;
	std::string _name;
	if (!(_packet >> _type >> _magic >> _protocol >> _name) || _type != PHello || _magic != Magic) return;

	const auto _reject = [&](const std::string& _reason)
	{
		sf::Packet _answer;
		_answer << static_cast<std::uint8_t>(PReject) << _reason;
		SendPacket(*_join.socket, _answer);
		LOG_WARNING("Network: a client was refused (" + _reason + ")");
	};
	if (_protocol != Protocol) { _reject("wrong network version (the host and the client are not the same build)"); return; }
	if (static_cast<int>(peers.size()) >= maxClients) { _reject("the game is full"); return; }

	std::unique_ptr<Peer> _peer = std::make_unique<Peer>();
	_peer->id = nextClientId++;
	_peer->socket = std::move(_join.socket);
	_peer->name = CleanName(_name, _peer->id);

	// Tell the newcomer who is here, and the others that somebody came.
	sf::Packet _welcome;
	_welcome << static_cast<std::uint8_t>(PWelcome) << static_cast<std::int32_t>(_peer->id) << playerName << static_cast<std::int32_t>(peers.size());
	for (const std::unique_ptr<Peer>& _other : peers)
		_welcome << static_cast<std::int32_t>(_other->id) << _other->name;
	SendPacket(*_peer->socket, _welcome);

	sf::Packet _joined;
	_joined << static_cast<std::uint8_t>(PJoined) << static_cast<std::int32_t>(_peer->id) << _peer->name;
	for (std::unique_ptr<Peer>& _other : peers)
	{
		sf::Packet _copy = _joined;
		SendPacket(*_other->socket, _copy);
	}

	const int _id = _peer->id;
	LOG_MSG("Network: player " + std::to_string(_id) + " (" + _peer->name + ") joined");
	peers.push_back(std::move(_peer));
	_accepted = true;
	BroadcastPlayers();
	onClientJoined.Broadcast(_id);
	if (role != NetRole::Host) return;
	onPlayersChanged.Broadcast();
	ReplicationPeerJoined(_id);
}

void NetworkManager::ReadHostSocket(Peer& _peer)
{
	// Up to a few packets per frame and per client, so a flood cannot freeze the game.
	const int _id = _peer.id;	// _peer is deleted if it is dropped while a packet is handled: never read it after that
	for (int _i = 0; _i < 64; ++_i)
	{
		sf::Packet _packet;
		const sf::Socket::Status _status = _peer.socket->receive(_packet);
		if (_status == sf::Socket::Status::Done)
		{
			CountReceived(_packet);
			_peer.silence = 0.f;
			HandleHostPacket(_peer, _packet);
			if (role != NetRole::Host || !FindPeer(_id)) return;	// dropped (or disconnected) while handling
		}
		else if (_status == sf::Socket::Status::Disconnected || _status == sf::Socket::Status::Error)
		{
			DropPeer(_peer, "disconnected");
			return;
		}
		else
			return;
	}
}

void NetworkManager::HandleHostPacket(Peer& _peer, sf::Packet& _packet)
{
	std::uint8_t _type = 0;
	if (!(_packet >> _type)) return;

	if (_type == PMessage)
	{
		NetMessage _message;
		int _target = ToAll;
		if (!ReadMessage(_packet, _message, _target)) return;
		_message.sender = _peer.id;		// whatever the client says
		_message.reliable = true;
		Route(_message, _target, &_peer, NetChannel::Reliable);
	}
	else if (_type == PPing)
	{
		float _time = 0.f, _ping = 0.f;
		if (!(_packet >> _time)) return;
		if (_packet >> _ping) _peer.pingMs = std::max(0.f, _ping);
		sf::Packet _pong;
		_pong << static_cast<std::uint8_t>(PPong) << _time;
		SendPacket(*_peer.socket, _pong);
	}
	else if (_type == PRename)
	{
		std::string _name;
		if (!(_packet >> _name)) return;
		const std::string _old = _peer.name;
		_peer.name = CleanName(_name, _peer.id);
		LOG_MSG("Network: player " + std::to_string(_peer.id) + " (" + _old + ") is now " + _peer.name);
		BroadcastPlayers();
		onPlayersChanged.Broadcast();
	}
}

void NetworkManager::DropPeer(Peer& _peer, const std::string& _reason)
{
	const int _id = _peer.id;
	LOG_MSG("Network: player " + std::to_string(_id) + " (" + _peer.name + ") left (" + _reason + ")");

	_peer.socket->disconnect();
	peers.erase(std::remove_if(peers.begin(), peers.end(), [&](const std::unique_ptr<Peer>& _p) { return _p->id == _id; }), peers.end());

	sf::Packet _left;
	_left << static_cast<std::uint8_t>(PLeft) << static_cast<std::int32_t>(_id);
	for (std::unique_ptr<Peer>& _other : peers)
	{
		sf::Packet _copy = _left;
		SendPacket(*_other->socket, _copy);
	}
	BroadcastPlayers();

	onClientLeft.Broadcast(_id);
	if (role != NetRole::Host) return;
	onPlayersChanged.Broadcast();
	ReplicationPeerLeft(_id);
}

void NetworkManager::ReadUdp()
{
	for (int _i = 0; _i < 256; ++_i)
	{
		sf::Packet _packet;
		std::optional<sf::IpAddress> _address;
		unsigned short _port = 0;
		if (udp.receive(_packet, _address, _port) != sf::Socket::Status::Done) return;
		CountReceived(_packet);

		std::uint8_t _type = 0;
		if (!(_packet >> _type)) continue;

		if (role == NetRole::Host)
		{
			if (_type == PUdpHello)
			{
				std::uint32_t _magic = 0;
				std::int32_t _id = 0;
				if (!(_packet >> _magic >> _id) || _magic != Magic) continue;
				Peer* _peer = FindPeer(_id);
				if (!_peer) continue;

				_peer->udpAddress = _address;
				_peer->udpPort = _port;
				sf::Packet _ack;
				_ack << static_cast<std::uint8_t>(PUdpAck);
				SendUdp(_ack, _address, _port);
			}
			else if (_type == PMessage)
			{
				NetMessage _message;
				int _target = ToAll;
				if (!ReadMessage(_packet, _message, _target)) continue;

				// Only a known client, from the address it said hello from.
				Peer* _peer = FindPeer(_message.sender);
				if (!_peer || !_peer->udpAddress || *_peer->udpAddress != *_address || _peer->udpPort != _port) continue;
				_peer->silence = 0.f;
				_message.reliable = false;
				Route(_message, _target, _peer, NetChannel::Unreliable);
			}
		}
		else if (role == NetRole::Client)
		{
			if (!_address || !serverAddress || *_address != *serverAddress || _port != serverPort) continue;
			if (_type == PUdpAck) udpReady = true;
			else if (_type == PMessage)
			{
				NetMessage _message;
				int _target = ToAll;
				if (!ReadMessage(_packet, _message, _target)) continue;
				_message.reliable = false;
				Deliver(_message);
			}
		}
		else
			return;		// a callback disconnected
	}
}
