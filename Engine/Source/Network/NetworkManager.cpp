#include "NetworkManager.h"
#include "Core/Engine.h"

using namespace Krampus;

namespace
{
	constexpr std::uint32_t Magic		= 0x4B52414D;	// "KRAM"
	constexpr std::uint32_t Protocol	= 1;

	// First byte of every packet.
	enum PacketType : std::uint8_t
	{
		PHello		= 1,	// client -> host (TCP): magic, protocol, name
		PWelcome	= 2,	// host -> client (TCP): your id, host name, number of other players, their ids
		PReject		= 3,	// host -> client (TCP): reason
		PJoined		= 4,	// host -> clients (TCP): id, name
		PLeft		= 5,	// host -> clients (TCP): id
		PMessage	= 6,	// both ways, TCP or UDP: sender, target, name, values
		PPing		= 7,	// client -> host (TCP): time
		PPong		= 8,	// host -> client (TCP): the same time
		PUdpHello	= 9,	// client -> host (UDP): magic, id
		PUdpAck		= 10	// host -> client (UDP)
	};

	// Targets inside a packet: >= 0 a player, or one of these.
	constexpr int ToAll		= -1;
	constexpr int ToHost	= -2;
	constexpr int ToOthers	= -3;

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
	// Quiet: the engine is going away, nobody is listening anymore.
	onDisconnected.Clear();
	onClientLeft.Clear();
	Reset();
}

// ---- Connection

bool NetworkManager::Host()
{
	return Host(static_cast<unsigned short>(GetWorld()->GetSettings().network.port));
}

bool NetworkManager::Host(const unsigned short _port)
{
	Disconnect();

	listener.setBlocking(false);
	if (listener.listen(_port) != sf::Socket::Status::Done)
	{
		LOG_ERROR("Network: cannot host on port " + std::to_string(_port) + " (already in use?)");
		return false;
	}
	udp.setBlocking(false);
	if (udp.bind(_port) != sf::Socket::Status::Done)
		LOG_WARNING("Network: UDP port " + std::to_string(_port) + " is not available: unreliable messages will use TCP");

	role = NetRole::Host;
	localId = 0;
	nextClientId = 1;
	clock.restart();
	LOG_MSG("Network: hosting on port " + std::to_string(_port));
	onHosted.Broadcast();
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

void NetworkManager::Join(const std::string& _address, const unsigned short _port)
{
	Disconnect();

	const std::optional<sf::IpAddress> _ip = sf::IpAddress::resolve(_address);
	if (!_ip)
	{
		LOG_ERROR("Network: \"" + _address + "\" is not a valid address");
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
	LOG_MSG("Network: joining " + _address + ":" + std::to_string(_port));

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
	Reset();
	if (_was == NetRole::Host || _was == NetRole::Client)
	{
		LOG_MSG("Network: disconnected");
		onDisconnected.Broadcast();
	}
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

	role = NetRole::Offline;
	localId = 0;
	otherClients.clear();
	silence = 0.f;
	pingMs = 0.f;
	udpReady = false;
	ReplicationReset();
}

// ---- State

std::vector<int> NetworkManager::GetOtherPlayers() const
{
	std::vector<int> _ids;
	if (role == NetRole::Host)
		for (const std::unique_ptr<Peer>& _peer : peers)
			_ids.push_back(_peer->id);
	else if (role == NetRole::Client)
	{
		_ids.push_back(0);
		_ids.insert(_ids.end(), otherClients.begin(), otherClients.end());
	}
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
	case NetRole::Host:			return "Hosting (" + std::to_string(peers.size()) + " client" + (peers.size() == 1 ? "" : "s") + ")";
	case NetRole::Connecting:	return "Connecting...";
	case NetRole::Client:		return "Connected as player " + std::to_string(localId) + " (" + std::to_string(static_cast<int>(pingMs)) + " ms)";
	default:					return "Offline";
	}
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
	// Sockets are not blocking: a full buffer gives Partial, the rest is sent by the next call (SFML keeps it).
	sf::Socket::Status _status = _socket.send(_packet);
	for (int _attempt = 0; _status == sf::Socket::Status::Partial && _attempt < 200; ++_attempt)
		_status = _socket.send(_packet);
}

void NetworkManager::SendUdp(const sf::Packet& _packet, const std::optional<sf::IpAddress>& _address, const unsigned short _port)
{
	if (!_address || _port == 0) return;
	sf::Packet _copy = _packet;
	(void)udp.send(_copy, *_address, _port);
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

// ---- Loop

void NetworkManager::Update()
{
	if (role == NetRole::Offline) return;

	const float _dt = std::min(clock.restart().asSeconds(), 0.25f);
	uptime += _dt;

	if (role == NetRole::Host) UpdateHost(_dt);
	else UpdateClient(_dt);
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
			HandleJoiningPacket(_join, _packet, _accepted);
			_remove = true;
		}
		else if (_status == sf::Socket::Status::Disconnected || _status == sf::Socket::Status::Error)
			_remove = true;

		if (_remove) joining.erase(joining.begin() + _i);	// an accepted socket was moved into peers
		else ++_i;
	}

	// Clients: read, and drop the ones that went silent.
	std::vector<Peer*> _lost;
	for (std::unique_ptr<Peer>& _peer : peers)
	{
		_peer->silence += _dt;
		ReadHostSocket(*_peer);
		if (_peer->silence > timeout) _lost.push_back(_peer.get());
	}
	for (Peer* _peer : _lost)
		if (FindPeer(_peer->id)) DropPeer(*_peer, "timeout");

	ReadUdp();
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
	if (_protocol != Protocol) { _reject("wrong network version"); return; }
	if (static_cast<int>(peers.size()) >= maxClients) { _reject("the game is full"); return; }

	std::unique_ptr<Peer> _peer = std::make_unique<Peer>();
	_peer->id = nextClientId++;
	_peer->socket = std::move(_join.socket);
	_peer->name = _name;

	// Tell the newcomer who is here, and the others that somebody came.
	sf::Packet _welcome;
	_welcome << static_cast<std::uint8_t>(PWelcome) << static_cast<std::int32_t>(_peer->id) << playerName << static_cast<std::int32_t>(peers.size());
	for (const std::unique_ptr<Peer>& _other : peers)
		_welcome << static_cast<std::int32_t>(_other->id);
	SendPacket(*_peer->socket, _welcome);

	sf::Packet _joined;
	_joined << static_cast<std::uint8_t>(PJoined) << static_cast<std::int32_t>(_peer->id) << _peer->name;
	for (std::unique_ptr<Peer>& _other : peers)
		SendPacket(*_other->socket, _joined);

	const int _id = _peer->id;
	LOG_MSG("Network: player " + std::to_string(_id) + " (" + _name + ") joined");
	peers.push_back(std::move(_peer));
	_accepted = true;
	onClientJoined.Broadcast(_id);
	ReplicationPeerJoined(_id);
}

void NetworkManager::ReadHostSocket(Peer& _peer)
{
	// Up to a few packets per frame and per client, so a flood cannot freeze the game.
	for (int _i = 0; _i < 64; ++_i)
	{
		sf::Packet _packet;
		const sf::Socket::Status _status = _peer.socket->receive(_packet);
		if (_status == sf::Socket::Status::Done)
		{
			_peer.silence = 0.f;
			HandleHostPacket(_peer, _packet);
			if (!FindPeer(_peer.id)) return;	// dropped while handling
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
		float _time = 0.f;
		if (!(_packet >> _time)) return;
		sf::Packet _pong;
		_pong << static_cast<std::uint8_t>(PPong) << _time;
		SendPacket(*_peer.socket, _pong);
	}
}

void NetworkManager::DropPeer(Peer& _peer, const char* _reason)
{
	const int _id = _peer.id;
	LOG_MSG("Network: player " + std::to_string(_id) + " left (" + _reason + ")");

	_peer.socket->disconnect();
	peers.erase(std::remove_if(peers.begin(), peers.end(), [&](const std::unique_ptr<Peer>& _p) { return _p->id == _id; }), peers.end());

	sf::Packet _left;
	_left << static_cast<std::uint8_t>(PLeft) << static_cast<std::int32_t>(_id);
	for (std::unique_ptr<Peer>& _other : peers)
		SendPacket(*_other->socket, _left);

	onClientLeft.Broadcast(_id);
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
				Reset();
				onConnectionFailed.Broadcast();
				return;
			}
		}

		if (server) ReadServerSocket();
		if (role == NetRole::Connecting && connectAge > 8.f)
		{
			LOG_WARNING("Network: the host did not answer");
			Reset();
			onConnectionFailed.Broadcast();
		}
		return;
	}

	ReadServerSocket();
	if (role != NetRole::Client) return;
	ReadUdp();

	silence += _dt;
	if (silence > timeout)
	{
		LOG_WARNING("Network: connection to the host lost (timeout)");
		Disconnect();
		return;
	}

	pingTimer += _dt;
	if (pingTimer >= 1.f)
	{
		pingTimer = 0.f;
		sf::Packet _ping;
		_ping << static_cast<std::uint8_t>(PPing) << uptime;
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
			silence = 0.f;
			HandleServerPacket(_packet);
		}
		else if (_status == sf::Socket::Status::Disconnected || _status == sf::Socket::Status::Error)
		{
			const bool _wasConnecting = role == NetRole::Connecting;
			Reset();
			if (_wasConnecting) onConnectionFailed.Broadcast();
			else
			{
				LOG_WARNING("Network: the host closed the connection");
				onDisconnected.Broadcast();
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
		otherClients.clear();
		for (std::int32_t _i = 0; _i < _count; ++_i)
		{
			std::int32_t _other = 0;
			if (_packet >> _other) otherClients.push_back(_other);
		}
		role = NetRole::Client;
		silence = 0.f;
		LOG_MSG("Network: connected to " + _hostName + " as player " + std::to_string(localId));
		onConnected.Broadcast();
		onClientJoined.Broadcast(0);
		for (const int _other : std::vector<int>(otherClients))
			onClientJoined.Broadcast(_other);
		break;
	}
	case PReject:
	{
		std::string _reason;
		_packet >> _reason;
		LOG_WARNING("Network: the host refused the connection (" + _reason + ")");
		Reset();
		onConnectionFailed.Broadcast();
		break;
	}
	case PJoined:
	{
		std::int32_t _id = 0;
		std::string _name;
		if (!(_packet >> _id >> _name)) return;
		otherClients.push_back(_id);
		LOG_MSG("Network: player " + std::to_string(_id) + " (" + _name + ") joined");
		onClientJoined.Broadcast(_id);
		break;
	}
	case PLeft:
	{
		std::int32_t _id = 0;
		if (!(_packet >> _id)) return;
		otherClients.erase(std::remove(otherClients.begin(), otherClients.end(), static_cast<int>(_id)), otherClients.end());
		LOG_MSG("Network: player " + std::to_string(_id) + " left");
		onClientLeft.Broadcast(_id);
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
