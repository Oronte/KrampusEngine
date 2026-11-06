#include "NetworkManager.h"
#include "NetworkProtocol.h"
#include "Core/Engine.h"
#include <random>

using namespace Krampus;
using namespace Krampus::NetProtocol;


NetworkManager::NetworkManager(Engine* _engine)
	: KrampusObject(_engine)
	, steam(std::make_unique<SteamNet>())
{
	// Accepting an invitation in the Steam overlay (or picking a lobby) joins the game of its host.
	steam->onLobbyJoined = [this](const std::uint64_t _host) { JoinSteam(std::to_string(_host)); };
	steam->onLobbiesFound = [this]() { onSteamLobbiesFound.Broadcast(); };
	steam->onLobbyFailed = [this]()
		{
			disconnectReason = steam->lastError;
			LOG_ERROR("Network: cannot join the Steam lobby: " + disconnectReason);
			onConnectionFailed.Broadcast();
		};
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

	BeginHosting("on port " + std::to_string(_port));
	return true;
}

void NetworkManager::BeginHosting(const std::string& _where)
{
	std::random_device _random;
	session = _random();
	role = NetRole::Host;
	localId = 0;
	nextClientId = 1;
	playersTimer = 0.f;
	clock.restart();
	LOG_MSG("Network: hosting \"" + gameName + "\" " + _where + " as " + playerName);
	onHosted.Broadcast();
	onPlayersChanged.Broadcast();
}

// ---- Steam

bool NetworkManager::InitSteam()
{
	return steam->Init(steamAppId);
}

bool NetworkManager::HostSteam()
{
	Disconnect();
	disconnectReason.clear();

	if (!InitSteam() || !steam->Listen())
	{
		disconnectReason = steam->lastError;
		LOG_ERROR("Network: cannot host through Steam: " + disconnectReason);
		return false;
	}
	if (playerName == "Player") playerName = steam->GetPersonaName();	// the default name is replaced by the Steam one

	steamMode = true;
	steam->CreateLobby(maxClients + 1, steamFriendsOnly, gameName);
	BeginHosting("through Steam");
	return true;
}

void NetworkManager::JoinSteam(const std::string& _hostSteamId)
{
	std::uint64_t _host = 0;
	try { _host = std::stoull(_hostSteamId); }
	catch (const std::exception&) { _host = 0; }

	if (_host != 0 && _host == steam->GetLocalId()) return;	// our own invitation
	Disconnect();
	disconnectReason.clear();

	if (_host == 0 || !InitSteam() || !(steamPending = steam->Connect(_host)))
	{
		disconnectReason = _host == 0 ? "\"" + _hostSteamId + "\" is not a SteamID" : steam->lastError;
		LOG_ERROR("Network: cannot join through Steam: " + disconnectReason);
		onConnectionFailed.Broadcast();
		return;
	}
	if (playerName == "Player") playerName = steam->GetPersonaName();

	steamMode = true;
	udpReady = true;	// the Steam connection has its own unreliable channel
	role = NetRole::Connecting;
	connectAge = 0.f;
	clock.restart();
	LOG_MSG("Network: joining " + _hostSteamId + " through Steam as " + playerName);
}

void NetworkManager::InviteSteamFriends()
{
	if (role != NetRole::Host || !steamMode)
	{
		LOG_WARNING("Network: host a Steam game first (HostSteam) to invite friends");
		return;
	}
	steam->InviteFriends();
}

bool NetworkManager::InviteSteamFriend(const std::uint64_t _friendId)
{
	if (role != NetRole::Host || !steamMode)
	{
		LOG_WARNING("Network: host a Steam game first (HostSteam) to invite friends");
		return false;
	}
	if (!steam->InviteFriend(_friendId))
	{
		LOG_WARNING("Network: Steam invitation failed: " + steam->lastError);
		return false;
	}
	LOG(VerbosityType::Log, "Network: Steam invitation sent to " + std::to_string(_friendId));
	return true;
}

void NetworkManager::JoinSteamLobby(const std::uint64_t _lobbyId)
{
	if (_lobbyId == 0 || !InitSteam())
	{
		disconnectReason = _lobbyId == 0 ? std::string("no Steam lobby") : steam->lastError;
		LOG_ERROR("Network: cannot join the Steam lobby: " + disconnectReason);
		onConnectionFailed.Broadcast();
		return;
	}
	LOG_MSG("Network: entering the Steam lobby " + std::to_string(_lobbyId));
	steam->JoinLobby(_lobbyId);		// then onLobbyJoined -> JoinSteam(host)
}

void NetworkManager::SearchSteam()
{
	if (!InitSteam())
	{
		LOG_WARNING("Network: cannot search Steam games: " + steam->lastError);
		return;
	}
	steam->FindLobbies(gameName);
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
		auto _tcp = std::make_unique<TcpNetSocket>();
		_result.status = _tcp->tcp.connect(_target, _port, sf::seconds(3));
		_result.socket = std::move(_tcp);
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
	steamPending.reset();
	steam->StopListen();
	steam->LeaveLobby();
	steamMode = false;
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

void NetworkManager::SendPacket(NetSocket& _socket, sf::Packet& _packet)
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

bool NetworkManager::SendSteamUnreliable(NetSocket& _socket, const sf::Packet& _packet)
{
	if (!_socket.sendUnreliable(_packet)) return false;
	stats.bytesSent += _packet.getDataSize();
	windowSent += _packet.getDataSize();
	++stats.packetsSent;
	return true;
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
	else if (_channel == NetChannel::Unreliable && _peer.socket->IsSteam() && SendSteamUnreliable(*_peer.socket, _packet)) { }
	else
		SendPacket(*_peer.socket, _packet);
}

void NetworkManager::SendToServer(const NetMessage& _message, const int _target, const NetChannel _channel)
{
	if (role != NetRole::Client || !server) return;

	sf::Packet _packet;
	WriteMessage(_packet, _message, _target);
	// Steam first: a Steam client has udpReady set (its connection carries the unreliable messages) but no UDP address, so the UDP
	// path would drop them (the host then never saw the clients move).
	if (_channel == NetChannel::Unreliable && server->IsSteam() && SendSteamUnreliable(*server, _packet)) { }
	else if (_channel == NetChannel::Unreliable && udpReady && serverAddress && serverPort != 0)
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

