#include "NetworkManager.h"
#include "Core/Engine.h"
#include <random>

using namespace Krampus;

namespace
{
	constexpr std::uint32_t Magic		= 0x4B52414D;	// "KRAM"
	constexpr std::uint32_t Protocol	= 2;

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
	constexpr int ToAll		= -1;
	constexpr int ToHost	= -2;
	constexpr int ToOthers	= -3;

	constexpr size_t MaxNameLength = 32;

	std::string CleanName(std::string _name, const int _id)
	{
		if (_name.size() > MaxNameLength) _name.resize(MaxNameLength);
		_name.erase(std::remove_if(_name.begin(), _name.end(), [](const unsigned char _c) { return _c < 32; }), _name.end());
		return _name.empty() ? "Player " + std::to_string(_id) : _name;
	}

	void WriteValue(sf::Packet& _packet, const PropertyValue& _value)
	{
		_packet << static_cast<std::uint8_t>(_value.index());
		switch (_value.index())
		{
		case 0: _packet << std::get<bool>(_value); break;
		case 1: _packet << static_cast<std::int32_t>(std::get<int>(_value)); break;
		case 2: _packet << std::get<float>(_value); break;
		case 3: _packet << std::get<std::string>(_value); break;
		case 4: _packet << std::get<FVector2>(_value).x << std::get<FVector2>(_value).y; break;
		default:
		{
			const Color& _color = std::get<Color>(_value);
			_packet << static_cast<std::uint8_t>(_color.r) << static_cast<std::uint8_t>(_color.g) << static_cast<std::uint8_t>(_color.b) << static_cast<std::uint8_t>(_color.a);
			break;
		}
		}
	}

	bool ReadValue(sf::Packet& _packet, PropertyValue& _value)
	{
		std::uint8_t _type = 0;
		if (!(_packet >> _type)) return false;

		switch (_type)
		{
		case 0: { bool _v = false; if (!(_packet >> _v)) return false; _value = _v; return true; }
		case 1: { std::int32_t _v = 0; if (!(_packet >> _v)) return false; _value = static_cast<int>(_v); return true; }
		case 2: { float _v = 0.f; if (!(_packet >> _v)) return false; _value = _v; return true; }
		case 3: { std::string _v; if (!(_packet >> _v)) return false; _value = _v; return true; }
		case 4: { float _x = 0.f, _y = 0.f; if (!(_packet >> _x >> _y)) return false; _value = FVector2(_x, _y); return true; }
		case 5:
		{
			std::uint8_t _r = 0, _g = 0, _b = 0, _a = 0;
			if (!(_packet >> _r >> _g >> _b >> _a)) return false;
			_value = Color(_r, _g, _b, _a);
			return true;
		}
		default: return false;
		}
	}

	// After the type byte: sender, target, name, values.
	void WriteMessage(sf::Packet& _packet, const NetMessage& _message, const int _target)
	{
		_packet << static_cast<std::uint8_t>(PMessage) << static_cast<std::int32_t>(_message.sender) << static_cast<std::int32_t>(_target) << _message.name
			<< static_cast<std::uint8_t>(std::min<size_t>(_message.args.size(), 255));
		for (size_t _i = 0; _i < _message.args.size() && _i < 255; ++_i)
			WriteValue(_packet, _message.args[_i]);
	}

	bool ReadMessage(sf::Packet& _packet, NetMessage& _message, int& _target)
	{
		std::int32_t _sender = 0, _to = 0;
		std::uint8_t _count = 0;
		if (!(_packet >> _sender >> _to >> _message.name >> _count)) return false;

		_message.sender = _sender;
		_target = _to;
		_message.args.clear();
		for (std::uint8_t _i = 0; _i < _count; ++_i)
		{
			PropertyValue _value;
			if (!ReadValue(_packet, _value)) return false;
			_message.args.push_back(std::move(_value));
		}
		return true;
	}
}

NetworkManager::NetworkManager(Engine* _engine)
	: KrampusObject(_engine)
{
}

NetworkManager::~NetworkManager()
{
	// Quiet: the engine is going away, nobody is listening anymore (and the level, with its actors, is already gone).
	onDisconnected.Clear();
	onClientLeft.Clear();
	onPlayersChanged.Clear();
	destroySpawnedOnDisconnect = false;
	Reset();
	discoverySearch.unbind();
}

// ---- Connection

bool NetworkManager::Host()
{
	return Host(static_cast<unsigned short>(GetWorld()->GetSettings().network.port));
}

bool NetworkManager::Host(const unsigned short _port)
{
	Disconnect();
	disconnectReason.clear();

	listener.setBlocking(false);
	if (listener.listen(_port) != sf::Socket::Status::Done)
	{
		disconnectReason = "port " + std::to_string(_port) + " is already in use";
		LOG_ERROR("Network: cannot host on port " + std::to_string(_port) + " (already in use?)");
		return false;
	}
	udp.setBlocking(false);
	if (udp.bind(_port) != sf::Socket::Status::Done)
		LOG_WARNING("Network: UDP port " + std::to_string(_port) + " is not available: unreliable messages will use TCP");

	if (lanVisible)
	{
		discoveryHost.setBlocking(false);
		discoveryHostBound = discoveryHost.bind(discoveryPort) == sf::Socket::Status::Done;
		if (!discoveryHostBound)
			LOG_WARNING("Network: LAN discovery port " + std::to_string(discoveryPort) + " is busy (another host on this PC?): this game cannot be found by SearchLan, join it by address");
	}

	std::random_device _random;
	session = _random();
	role = NetRole::Host;
	localId = 0;
	nextClientId = 1;
	playersTimer = 0.f;
	clock.restart();
	LOG_MSG("Network: hosting \"" + gameName + "\" on port " + std::to_string(_port) + " as " + playerName);
	onHosted.Broadcast();
	onPlayersChanged.Broadcast();
	return true;
}

void NetworkManager::Join()
{
	const NetworkSettings& _settings = GetWorld()->GetSettings().network;
	Join(_settings.address, static_cast<unsigned short>(_settings.port));
}

void NetworkManager::Join(const std::string& _addressAndPort)
{
	std::string _address = _addressAndPort;
	unsigned short _port = static_cast<unsigned short>(GetWorld()->GetSettings().network.port);

	if (const size_t _colon = _address.rfind(':'); _colon != std::string::npos)
	{
		try { _port = static_cast<unsigned short>(std::stoi(_address.substr(_colon + 1))); }
		catch (const std::exception&) { LOG_WARNING("Network: invalid port in \"" + _address + "\", using the default"); }
		_address = _address.substr(0, _colon);
	}
	Join(_address, _port);
}

void NetworkManager::Join(const NetLanGame& _game)
{
	Join(_game.address, _game.port);
}

void NetworkManager::Join(const std::string& _address, const unsigned short _port)
{
	Disconnect();
	disconnectReason.clear();

	const std::optional<sf::IpAddress> _ip = sf::IpAddress::resolve(_address);
	if (!_ip)
	{
		disconnectReason = "\"" + _address + "\" is not a valid address";
		LOG_ERROR("Network: " + disconnectReason);
		onConnectionFailed.Broadcast();
		return;
	}

	udp.setBlocking(false);
	if (udp.bind(sf::Socket::AnyPort) != sf::Socket::Status::Done)
		LOG_WARNING("Network: no UDP socket available: unreliable messages will use TCP");

	role = NetRole::Connecting;
	serverAddress = _ip;
	serverPort = _port;
	connectAge = 0.f;
	clock.restart();
	LOG_MSG("Network: joining " + _address + ":" + std::to_string(_port) + " as " + playerName);

	// A connect that cannot reach anybody blocks for seconds: it runs on its own, Update picks the result up.
	const sf::IpAddress _target = *_ip;
	connecting = std::async(std::launch::async, [_target, _port]()
	{
		ConnectResult _result;
		_result.socket = std::make_unique<sf::TcpSocket>();
		_result.status = _result.socket->connect(_target, _port, sf::seconds(3));
		return _result;
	});
}

void NetworkManager::Disconnect()
{
	const NetRole _was = role;
	const bool _wasOnline = _was == NetRole::Host || _was == NetRole::Client;
	if (_wasOnline)
	{
		disconnectReason = _was == NetRole::Host ? "you closed the game" : "you left the game";
		DestroySpawnedActors();
	}
	Reset();
	if (_wasOnline)
	{
		LOG_MSG("Network: disconnected");
		onDisconnected.Broadcast();
		onPlayersChanged.Broadcast();
	}
}

bool NetworkManager::Kick(const int _playerId, const std::string& _reason)
{
	if (role != NetRole::Host)
	{
		LOG_WARNING("Network: only the host can kick a player");
		return false;
	}
	Peer* _peer = FindPeer(_playerId);
	if (!_peer) return false;

	sf::Packet _kick;
	_kick << static_cast<std::uint8_t>(PKick) << _reason;
	SendPacket(*_peer->socket, _kick);
	DropPeer(*_peer, "kicked: " + _reason);
	return true;
}

void NetworkManager::EndGame(const std::string& _reason, const bool _connectionFailed)
{
	disconnectReason = _reason;
	if (IsOnline()) DestroySpawnedActors();
	Reset();
	if (_connectionFailed) onConnectionFailed.Broadcast();
	else onDisconnected.Broadcast();
	onPlayersChanged.Broadcast();
}

void NetworkManager::Reset()
{
	if (connecting.valid()) connecting.wait();
	connecting = { };

	listener.close();
	for (std::unique_ptr<Peer>& _peer : peers)
		_peer->socket->disconnect();
	peers.clear();
	joining.clear();
	if (server) server->disconnect();
	server.reset();
	udp.unbind();
	discoveryHost.unbind();
	discoveryHostBound = false;

	role = NetRole::Offline;
	localId = 0;
	players.clear();
	silence = 0.f;
	pingMs = 0.f;
	udpReady = false;
	ReplicationReset();
}

// ---- State

std::vector<NetPlayer> NetworkManager::GetPlayers() const
{
	std::vector<NetPlayer> _players;
	if (role == NetRole::Host)
	{
		_players.push_back(NetPlayer{ 0, playerName, 0.f, true, true });
		for (const std::unique_ptr<Peer>& _peer : peers)
			_players.push_back(NetPlayer{ _peer->id, _peer->name, _peer->pingMs, false, false });
	}
	else if (role == NetRole::Client)
	{
		_players = players;
		bool _hasLocal = false;
		for (NetPlayer& _player : _players)
		{
			_player.isLocal = _player.id == localId;
			_player.isHost = _player.id == 0;
			if (_player.isLocal)
			{
				_player.name = playerName;
				_player.pingMs = pingMs;
				_hasLocal = true;
			}
		}
		if (!_hasLocal) _players.push_back(NetPlayer{ localId, playerName, pingMs, false, true });
	}
	else
		_players.push_back(NetPlayer{ 0, playerName, 0.f, false, true });

	std::sort(_players.begin(), _players.end(), [](const NetPlayer& _a, const NetPlayer& _b) { return _a.id < _b.id; });
	return _players;
}

std::string NetworkManager::GetPlayerName(const int _playerId) const
{
	for (const NetPlayer& _player : GetPlayers())
		if (_player.id == _playerId) return _player.name;
	return "Player " + std::to_string(_playerId);
}

void NetworkManager::SetPlayerName(const std::string& _name)
{
	playerName = CleanName(_name, localId);
	if (role == NetRole::Host)
		BroadcastPlayers();
	else if (role == NetRole::Client && server)
	{
		sf::Packet _rename;
		_rename << static_cast<std::uint8_t>(PRename) << playerName;
		SendPacket(*server, _rename);
	}
	onPlayersChanged.Broadcast();
}

std::vector<int> NetworkManager::GetOtherPlayers() const
{
	std::vector<int> _ids;
	if (!IsOnline()) return _ids;
	for (const NetPlayer& _player : GetPlayers())
		if (!_player.isLocal) _ids.push_back(_player.id);
	return _ids;
}

int NetworkManager::GetPlayerCount() const
{
	return 1 + static_cast<int>(GetOtherPlayers().size());
}

std::string NetworkManager::GetStatusText() const
{
	switch (role)
	{
	case NetRole::Host:			return "Hosting \"" + gameName + "\" (" + std::to_string(peers.size()) + " client" + (peers.size() == 1 ? "" : "s") + ")";
	case NetRole::Connecting:	return "Connecting...";
	case NetRole::Client:		return "Connected as player " + std::to_string(localId) + " (" + std::to_string(static_cast<int>(pingMs)) + " ms)";
	default:					return disconnectReason.empty() ? "Offline" : "Offline (" + disconnectReason + ")";
	}
}

void NetworkManager::BroadcastPlayers()
{
	if (role != NetRole::Host) return;

	sf::Packet _packet;
	_packet << static_cast<std::uint8_t>(PPlayers) << static_cast<std::int32_t>(peers.size() + 1);
	_packet << static_cast<std::int32_t>(0) << playerName << 0.f;
	for (const std::unique_ptr<Peer>& _peer : peers)
		_packet << static_cast<std::int32_t>(_peer->id) << _peer->name << _peer->pingMs;
	for (std::unique_ptr<Peer>& _peer : peers)
	{
		sf::Packet _copy = _packet;
		SendPacket(*_peer->socket, _copy);
	}
}

void NetworkManager::SetPlayersFromHost(std::vector<NetPlayer> _players)
{
	for (NetPlayer& _player : _players)
	{
		_player.isHost = _player.id == 0;
		_player.isLocal = _player.id == localId;
	}
	players = std::move(_players);
	onPlayersChanged.Broadcast();
}

// ---- Messages

void NetworkManager::Deliver(const NetMessage& _message)
{
	if (HandleInternal(_message)) return;

	onMessage.Broadcast(_message);
	if (const auto _found = subscribers.find(_message.name); _found != subscribers.end())
		_found->second.Broadcast(_message);
}

Event<const NetMessage&>::ListenerHandle NetworkManager::Subscribe(const std::string& _name, std::function<void(const NetMessage&)> _callback)
{
	return subscribers[_name].AddListener(std::move(_callback));
}

void NetworkManager::Send(const std::string& _name, const std::vector<PropertyValue>& _args, const NetTarget _target, const NetChannel _channel)
{
	NetMessage _message;
	_message.name = _name;
	_message.args = _args;
	_message.sender = localId;
	_message.reliable = _channel == NetChannel::Reliable;

	if (!IsOnline())
	{
		// Alone: the message only goes to this game.
		if (role == NetRole::Offline && _target == NetTarget::All) Deliver(_message);
		return;
	}

	const int _code = _target == NetTarget::All ? ToAll : (_target == NetTarget::Others ? ToOthers : ToHost);
	if (role == NetRole::Host)
		Route(_message, _code, nullptr, _channel);
	else
	{
		SendToServer(_message, _code, _channel);
		if (_target == NetTarget::All) Deliver(_message);
	}
}

void NetworkManager::SendTo(const int _playerId, const std::string& _name, const std::vector<PropertyValue>& _args, const NetChannel _channel)
{
	NetMessage _message;
	_message.name = _name;
	_message.args = _args;
	_message.sender = localId;
	_message.reliable = _channel == NetChannel::Reliable;

	if (!IsOnline())
	{
		if (_playerId == 0 || _playerId == localId) Deliver(_message);
		return;
	}
	if (_playerId == localId)
	{
		Deliver(_message);
		return;
	}

	if (role == NetRole::Host) Route(_message, _playerId, nullptr, _channel);
	else SendToServer(_message, _playerId, _channel);
}

// Host side: delivers to the host and forwards to the clients according to the target.
void NetworkManager::Route(const NetMessage& _message, const int _target, const Peer* _from, const NetChannel _channel)
{
	const bool _toHost = _target == ToAll || _target == ToHost || _target == 0 || (_target == ToOthers && _from);
	for (std::unique_ptr<Peer>& _peer : peers)
	{
		if (_peer.get() == _from || _target == ToHost) continue;
		if (_target == ToAll || _target == ToOthers || _target == _peer->id)
			SendMessageTo(*_peer, _message, _channel);
	}
	if (_toHost) Deliver(_message);
}

void NetworkManager::SendPacket(sf::TcpSocket& _socket, sf::Packet& _packet)
{
	stats.bytesSent += _packet.getDataSize() + 4;	// + the size SFML puts before a TCP packet
	windowSent += _packet.getDataSize() + 4;
	++stats.packetsSent;

	// Sockets are not blocking: a full buffer gives Partial, the rest is sent by the next call (SFML keeps it).
	sf::Socket::Status _status = _socket.send(_packet);
	for (int _attempt = 0; _status == sf::Socket::Status::Partial && _attempt < 200; ++_attempt)
		_status = _socket.send(_packet);

	// Still not sent: the packet dies with this function, so leaving it half written would cut the TCP stream in the middle of a
	// packet and garble everything after it. Finish it in blocking mode (rare: only when the other side reads too slowly).
	if (_status == sf::Socket::Status::Partial)
	{
		_socket.setBlocking(true);
		_status = _socket.send(_packet);
		_socket.setBlocking(false);
	}
}

void NetworkManager::SendUdp(const sf::Packet& _packet, const std::optional<sf::IpAddress>& _address, const unsigned short _port)
{
	if (!_address || _port == 0) return;
	stats.bytesSent += _packet.getDataSize();
	windowSent += _packet.getDataSize();
	++stats.packetsSent;
	sf::Packet _copy = _packet;
	(void)udp.send(_copy, *_address, _port);
}

void NetworkManager::CountReceived(const sf::Packet& _packet)
{
	stats.bytesReceived += _packet.getDataSize();
	windowReceived += _packet.getDataSize();
	++stats.packetsReceived;
}

void NetworkManager::SendMessageTo(Peer& _peer, const NetMessage& _message, const NetChannel _channel)
{
	sf::Packet _packet;
	WriteMessage(_packet, _message, _peer.id);
	if (_channel == NetChannel::Unreliable && _peer.udpPort != 0)
		SendUdp(_packet, _peer.udpAddress, _peer.udpPort);
	else
		SendPacket(*_peer.socket, _packet);
}

void NetworkManager::SendToServer(const NetMessage& _message, const int _target, const NetChannel _channel)
{
	if (role != NetRole::Client || !server) return;

	sf::Packet _packet;
	WriteMessage(_packet, _message, _target);
	if (_channel == NetChannel::Unreliable && udpReady)
		SendUdp(_packet, serverAddress, serverPort);
	else
		SendPacket(*server, _packet);
}

NetworkManager::Peer* NetworkManager::FindPeer(const int _id)
{
	for (std::unique_ptr<Peer>& _peer : peers)
		if (_peer->id == _id) return _peer.get();
	return nullptr;
}

// ---- Shared values

void NetworkManager::SetShared(const std::string& _key, const PropertyValue& _value)
{
	if (role == NetRole::Client)
	{
		Send("__setreq", { _key, _value }, NetTarget::Host);
		return;
	}
	if (role == NetRole::Connecting)
	{
		LOG_WARNING("Network: SetShared(\"" + _key + "\") while joining: wait for onConnected");
		return;
	}

	ApplyShared(_key, _value);
	if (role == NetRole::Host) Send("__shared", { _key, _value }, NetTarget::Others);
}

void NetworkManager::AddShared(const std::string& _key, const float _amount)
{
	if (role == NetRole::Client)
	{
		Send("__addreq", { _key, _amount }, NetTarget::Host);
		return;
	}

	// An int stays an int while whole numbers are added to it.
	const bool _whole = std::floor(_amount) == _amount;
	const PropertyValue _current = GetShared(_key, PropertyValue(0));
	if (const int* _int = std::get_if<int>(&_current); _int && _whole)
		SetShared(_key, *_int + static_cast<int>(_amount));
	else if (_int)
		SetShared(_key, static_cast<float>(*_int) + _amount);
	else if (const float* _float = std::get_if<float>(&_current))
		SetShared(_key, *_float + _amount);
	else
		SetShared(_key, _whole ? PropertyValue(static_cast<int>(_amount)) : PropertyValue(_amount));
}

PropertyValue NetworkManager::GetShared(const std::string& _key, const PropertyValue& _fallback) const
{
	const auto _found = shared.find(_key);
	return _found == shared.end() ? _fallback : _found->second;
}

void NetworkManager::ApplyShared(const std::string& _key, const PropertyValue& _value)
{
	shared[_key] = _value;
	const std::string _keyCopy = _key;		// a listener may change the map: the event gets its own copies
	const PropertyValue _valueCopy = _value;
	onSharedChanged.Broadcast(_keyCopy, _valueCopy);
}

// ---- Loop

void NetworkManager::Update()
{
	const float _dt = std::min(clock.restart().asSeconds(), 0.25f);
	uptime += _dt;

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
		auto _socket = std::make_unique<sf::TcpSocket>();
		if (listener.accept(*_socket) != sf::Socket::Status::Done) break;
		_socket->setBlocking(false);
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

void NetworkManager::UpdateClient(const float _dt)
{
	if (role == NetRole::Connecting)
	{
		connectAge += _dt;
		if (connecting.valid() && connecting.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
		{
			ConnectResult _result = connecting.get();
			connecting = { };
			if (_result.status == sf::Socket::Status::Done)
			{
				server = std::move(_result.socket);
				server->setBlocking(false);
				sf::Packet _hello;
				_hello << static_cast<std::uint8_t>(PHello) << Magic << Protocol << playerName;
				SendPacket(*server, _hello);
				connectAge = 0.f;	// now waiting for the Welcome
			}
			else
			{
				LOG_WARNING("Network: could not reach the host");
				EndGame("could not reach the host (wrong address, not hosting, or a firewall)", true);
				return;
			}
		}

		if (server) ReadServerSocket();
		if (role == NetRole::Connecting && connectAge > 8.f)
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
	if (!udpReady && udpTimer >= 0.5f)
	{
		udpTimer = 0.f;
		sf::Packet _hello;
		_hello << static_cast<std::uint8_t>(PUdpHello) << Magic << static_cast<std::int32_t>(localId);
		SendUdp(_hello, serverAddress, serverPort);
	}
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

// ---- LAN discovery

void NetworkManager::SearchLan(const float _seconds)
{
	if (!discoverySearchBound)
	{
		discoverySearch.setBlocking(false);
		discoverySearchBound = discoverySearch.bind(sf::Socket::AnyPort) == sf::Socket::Status::Done;
		if (!discoverySearchBound)
		{
			LOG_WARNING("Network: no UDP socket for the LAN search");
			return;
		}
	}
	lanGames.clear();
	searchTime = std::max(0.5f, _seconds);
	searchResend = 0.f;
	LOG_MSG("Network: searching the LAN for games (port " + std::to_string(discoveryPort) + ")...");
}

void NetworkManager::UpdateDiscovery(const float _dt)
{
	for (NetLanGame& _game : lanGames)
		_game.age += _dt;

	// Host: answer the players who search.
	if (role == NetRole::Host && discoveryHostBound)
	{
		for (int _i = 0; _i < 32; ++_i)
		{
			sf::Packet _packet;
			std::optional<sf::IpAddress> _address;
			unsigned short _port = 0;
			if (discoveryHost.receive(_packet, _address, _port) != sf::Socket::Status::Done || !_address) break;
			CountReceived(_packet);

			std::uint8_t _type = 0;
			std::uint32_t _magic = 0, _protocol = 0;
			if (!(_packet >> _type >> _magic >> _protocol) || _type != PLanAsk || _magic != Magic) continue;

			sf::Packet _answer;
			_answer << static_cast<std::uint8_t>(PLanAnswer) << Magic << Protocol << gameName << playerName
				<< static_cast<std::uint16_t>(listener.getLocalPort()) << static_cast<std::int32_t>(GetPlayerCount())
				<< static_cast<std::int32_t>(maxClients + 1) << session;
			stats.bytesSent += _answer.getDataSize();
			++stats.packetsSent;
			(void)discoveryHost.send(_answer, *_address, _port);
		}
	}

	if (searchTime <= 0.f) return;

	// Searcher: ask twice a second (a broadcast for the LAN, and this PC itself for a test with two windows), read the answers.
	searchTime -= _dt;
	searchResend -= _dt;
	if (searchResend <= 0.f)
	{
		searchResend = 0.5f;
		sf::Packet _ask;
		_ask << static_cast<std::uint8_t>(PLanAsk) << Magic << Protocol;
		sf::Packet _askLocal = _ask;
		(void)discoverySearch.send(_ask, sf::IpAddress::Broadcast, discoveryPort);
		(void)discoverySearch.send(_askLocal, sf::IpAddress::LocalHost, discoveryPort);
	}

	for (int _i = 0; _i < 32; ++_i)
	{
		sf::Packet _packet;
		std::optional<sf::IpAddress> _address;
		unsigned short _port = 0;
		if (discoverySearch.receive(_packet, _address, _port) != sf::Socket::Status::Done || !_address) break;
		CountReceived(_packet);

		std::uint8_t _type = 0;
		std::uint32_t _magic = 0, _protocol = 0;
		NetLanGame _game;
		std::uint16_t _gamePort = 0;
		std::int32_t _players = 0, _max = 0;
		if (!(_packet >> _type >> _magic >> _protocol >> _game.gameName >> _game.hostName >> _gamePort >> _players >> _max >> _game.session)
			|| _type != PLanAnswer || _magic != Magic)
			continue;
		if (_protocol != Protocol) continue;							// another build: it would refuse us anyway
		if (role == NetRole::Host && _game.session == session) continue;	// this game itself

		_game.address = _address->toString();
		_game.port = _gamePort;
		_game.players = _players;
		_game.maxPlayers = _max;

		const auto _known = std::find_if(lanGames.begin(), lanGames.end(), [&](const NetLanGame& _other) { return _other.session == _game.session; });
		if (_known != lanGames.end())
		{
			const std::string _firstAddress = _known->address;	// the first answer gave the address: the same host also answers on another one
			*_known = _game;
			_known->address = _firstAddress;
			continue;
		}
		lanGames.push_back(_game);
		LOG_MSG("Network: found \"" + _game.gameName + "\" hosted by " + _game.hostName + " at " + _game.address + ":" + std::to_string(_game.port)
			+ " (" + std::to_string(_game.players) + "/" + std::to_string(_game.maxPlayers) + ")");
		const NetLanGame _copy = _game;
		onLanGameFound.Broadcast(_copy);
	}

	if (searchTime <= 0.f)
	{
		searchTime = 0.f;
		LOG_MSG("Network: LAN search done (" + std::to_string(lanGames.size()) + " game" + (lanGames.size() == 1 ? "" : "s") + " found)");
	}
}
