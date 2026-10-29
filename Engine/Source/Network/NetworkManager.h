#pragma once
#include "Core/KrampusObject.h"
#include "Reflection/EventList.h"
#include <SFML/Network.hpp>
#include <future>

// Networking for the game, on top of SFML (TCP + UDP). One host (the server, which is also a player: id 0) and any number of clients.
//
//   NetworkManager* net = GetWorld()->GetNetwork();
//
//   Connect      net->Host();   net->Join("192.168.1.20");   net->SearchLan(); ... net->Join(net->GetLanGames()[0]);
//   Players      for (const NetPlayer& _player : net->GetPlayers()) { _player.name, _player.pingMs, _player.isHost... }
//   Messages     net->Send("Score", NetworkManager::Args(10, "Alice"));                        // to everybody
//                auto _handle = net->Subscribe("Score", [](const NetMessage& _message) { int _points = _message.Int(0); });
//   Shared data  net->SetShared("round", 3);  net->AddShared("score", 1);  int _round = net->GetShared<int>("round", 0);
//                (the host keeps the values, every player sees the same ones, a player who joins later gets them all)
//   Actors       net->Spawn("Player", position);            // on every machine, owned by the one who asked
//                sync->CallRPC("Explode");                  // calls the KFUNCTION Explode of that actor on every machine
//
// Messages carry any number of values (bool, int, float, string, FVector2, Color). They go through a reliable channel (TCP: always
// arrive, in order) or an unreliable one (UDP: fast, may be lost: for positions). Everything is polled from the engine loop: no threads
// to manage, and every callback runs on the game thread.
// For actors that must be seen by everybody, add a NetworkSyncComponent (no code): see Components/NetworkSyncComponent.h.

namespace Krampus
{

	class NetworkSyncComponent;
	class Actor;

	/// <summary>What this game is in the network.</summary>
	enum class NetRole
	{
		Offline,
		Host,			// listens for clients, also plays (id 0)
		Connecting,		// a client that is joining
		Client			// a client that is connected
	};

	/// <summary>Who receives a message.</summary>
	enum class NetTarget
	{
		All,			// everybody, the sender included
		Others,			// everybody but the sender
		Host			// the host only
	};

	enum class NetChannel
	{
		Reliable,		// TCP
		Unreliable		// UDP (falls back to Reliable while the UDP link is not ready)
	};

	/// <summary>
	/// A message received from the network: a name and a list of values.
	/// </summary>
	struct NetMessage
	{
		std::string					name;
		std::vector<PropertyValue>	args;
		int							sender		= 0;		// id of the player that sent it (0 = host)
		bool						reliable	= true;

		template <typename T>
		T Get(const size_t _index, const T& _fallback = T()) const
		{
			if (_index >= args.size()) return _fallback;
			if (const T* _value = std::get_if<T>(&args[_index])) return *_value;
			return _fallback;
		}

		bool			Bool(const size_t _index, const bool _fallback = false) const { return Get<bool>(_index, _fallback); }
		int				Int(const size_t _index, const int _fallback = 0) const { return Get<int>(_index, _fallback); }
		float			Float(const size_t _index, const float _fallback = 0.f) const { return Get<float>(_index, _fallback); }
		std::string		String(const size_t _index, const std::string& _fallback = std::string()) const { return Get<std::string>(_index, _fallback); }
		FVector2		Vector(const size_t _index) const { return Get<FVector2>(_index, FVector2()); }
		Krampus::Color	Color(const size_t _index) const { return Get<Krampus::Color>(_index, Krampus::Color::White()); }
	};

	/// <summary>A player of the game, this one included.</summary>
	struct NetPlayer
	{
		int				id			= 0;		// 0 for the host
		std::string		name;
		float			pingMs		= 0.f;		// round trip to the host in milliseconds (0 for the host)
		bool			isHost		= false;
		bool			isLocal		= false;	// this machine
	};

	/// <summary>A game found on the local network by SearchLan().</summary>
	struct NetLanGame
	{
		std::string		gameName;				// Settings > Ship > gameName of the host
		std::string		hostName;				// player name of the host
		std::string		address;				// where to join
		unsigned short	port		= 0;
		int				players		= 0;
		int				maxPlayers	= 0;
		float			age			= 0.f;		// seconds since its last answer
		std::uint32_t	session		= 0;		// tells two answers of the same host apart from two hosts
	};

	/// <summary>What went through the network (both channels).</summary>
	struct NetStats
	{
		std::uint64_t	bytesSent			= 0;
		std::uint64_t	bytesReceived		= 0;
		std::uint64_t	packetsSent			= 0;
		std::uint64_t	packetsReceived		= 0;
		float			sentPerSecond		= 0.f;	// bytes during the last second
		float			receivedPerSecond	= 0.f;
	};

	/// <summary>
	/// Hosts or joins a game, sends and receives messages, keeps the list of players and the shared values, finds games on the LAN.
	/// Owned by the Engine (Engine::GetNetwork()).
	/// </summary>
	class NetworkManager : public KrampusObject
	{
		struct Peer
		{
			int									id			= 0;
			std::unique_ptr<sf::TcpSocket>		socket;
			std::optional<sf::IpAddress>		udpAddress;
			unsigned short						udpPort		= 0;
			float								silence		= 0.f;		// seconds since the last thing received
			float								pingMs		= 0.f;		// as the client measured it
			std::string							name;
		};

		struct Joining
		{
			std::unique_ptr<sf::TcpSocket>		socket;
			float								age			= 0.f;
		};

		struct ConnectResult
		{
			sf::Socket::Status					status		= sf::Socket::Status::Error;
			std::unique_ptr<sf::TcpSocket>		socket;
		};

		struct SpawnRecord
		{
			int				id;
			std::string		className;
			int				owner;
			FVector2		position;
		};

		NetRole									role			= NetRole::Offline;
		int										localId			= 0;
		int										nextClientId	= 1;
		sf::Clock								clock;
		float									uptime			= 0.f;
		std::string								disconnectReason;

		// Host
		sf::TcpListener							listener;
		std::vector<std::unique_ptr<Peer>>		peers;
		std::vector<Joining>					joining;
		float									playersTimer	= 0.f;		// the list of players (with the pings) goes to the clients every second
		std::uint32_t							session			= 0;		// random, for the LAN answers

		// Client
		std::unique_ptr<sf::TcpSocket>			server;
		std::future<ConnectResult>				connecting;
		std::optional<sf::IpAddress>			serverAddress;
		unsigned short							serverPort		= 0;
		float									silence			= 0.f;
		float									pingTimer		= 0.f;
		float									udpTimer		= 0.f;
		bool									udpReady		= false;
		float									pingMs			= 0.f;
		std::vector<NetPlayer>					players;			// every player as the host announced them (this one included)
		float									connectAge		= 0.f;

		sf::UdpSocket							udp;
		std::unordered_map<std::string, Event<const NetMessage&>>		subscribers;

		// LAN discovery: the host answers on discoveryPort, SearchLan asks with a broadcast.
		sf::UdpSocket							discoveryHost;
		bool									discoveryHostBound	= false;
		sf::UdpSocket							discoverySearch;
		bool									discoverySearchBound = false;
		float									searchTime		= 0.f;		// seconds of search left
		float									searchResend	= 0.f;
		std::vector<NetLanGame>					lanGames;

		// Statistics
		NetStats								stats;
		std::uint64_t							windowSent		= 0;
		std::uint64_t							windowReceived	= 0;
		float									statsTimer		= 0.f;

		// Shared values (host authority)
		std::map<std::string, PropertyValue>	shared;

		// Replication (NetworkReplication.cpp)
		std::unordered_map<int, NetworkSyncComponent*>	synced;
		std::vector<SpawnRecord>				spawned;
		int										nextSpawnId		= 0x40000000;

	public:
		/// <summary>Every message, with its name inside (the internal ones, "__...", never come here).</summary>
		Event<const NetMessage&>				onMessage;

		Event<>									onHosted;				// Host() succeeded
		Event<>									onConnected;			// Join() succeeded
		Event<>									onConnectionFailed;		// Join() failed (nobody there, refused, timeout): see GetDisconnectReason()
		Event<>									onDisconnected;			// the connection ended (or Disconnect() was called): see GetDisconnectReason()
		Event<int>								onClientJoined;			// a player arrived (host and clients): its id
		Event<int>								onClientLeft;			// a player left: its id
		Event<>									onPlayersChanged;		// somebody came, left, changed name, or the pings were updated
		Event<const std::string&, const PropertyValue&>	onSharedChanged;	// a shared value was set (on every machine)
		Event<Actor*>							onActorSpawned;			// Spawn created an actor on this machine (NetworkSyncComponent::ownerId says whose)
		Event<const NetLanGame&>				onLanGameFound;			// SearchLan found a game it did not know yet

		/// <summary>Seconds without anything from the other side before the connection is dropped.</summary>
		float									timeout			= 10.f;
		/// <summary>Maximum number of clients the host accepts.</summary>
		int										maxClients		= 8;
		/// <summary>Name of this player (see SetPlayerName to change it during a game).</summary>
		std::string								playerName		= "Player";
		/// <summary>Name of the game, shown to the players who search the LAN.</summary>
		std::string								gameName		= "KrampusGame";
		/// <summary>UDP port of the LAN discovery (the host answers on it, SearchLan asks on it).</summary>
		unsigned short							discoveryPort	= 7778;
		/// <summary>When false, a host does not answer SearchLan (players must type its address).</summary>
		bool									lanVisible		= true;
		/// <summary>When the game ends (Disconnect, host gone, kicked), the actors made by Spawn are destroyed on this machine.</summary>
		bool									destroySpawnedOnDisconnect = true;

		explicit NetworkManager(Engine* _engine);
		~NetworkManager();

		// ---- Connection

		/// <summary>Starts listening for clients on _port (TCP and UDP). Returns false if the port is not available.</summary>
		bool Host(unsigned short _port);

		/// <summary>Host with the port of the Network settings.</summary>
		bool Host();

		/// <summary>Joins a host. Returns at once: onConnected or onConnectionFailed tells the result.</summary>
		void Join(const std::string& _address, unsigned short _port);

		/// <summary>Joins "address" or "address:port" (the port of the Network settings when none is given).</summary>
		void Join(const std::string& _addressAndPort);

		/// <summary>Joins a game found by SearchLan.</summary>
		void Join(const NetLanGame& _game);

		/// <summary>Join with the address and port of the Network settings.</summary>
		void Join();

		/// <summary>Leaves the game (a host closes it: clients are disconnected).</summary>
		void Disconnect();

		/// <summary>Host: sends a player away with a reason (shown on its side). Returns false if there is no such player.</summary>
		bool Kick(int _playerId, const std::string& _reason = "kicked by the host");

		/// <summary>Why the last game ended or the last Join failed ("timeout", "the host closed the game", "kicked: ...").</summary>
		const std::string& GetDisconnectReason() const { return disconnectReason; }

		// ---- LAN discovery

		/// <summary>Looks for hosts on the local network for _seconds. Results: GetLanGames() and onLanGameFound. Works offline.</summary>
		void SearchLan(float _seconds = 3.f);

		bool IsSearchingLan() const { return searchTime > 0.f; }

		/// <summary>The games found by the last SearchLan.</summary>
		const std::vector<NetLanGame>& GetLanGames() const { return lanGames; }

		// ---- State

		NetRole GetRole() const { return role; }
		bool IsHost() const { return role == NetRole::Host; }
		bool IsClient() const { return role == NetRole::Client || role == NetRole::Connecting; }
		/// <returns>True when messages can go somewhere: hosting, or connected to a host.</returns>
		bool IsOnline() const { return role == NetRole::Host || role == NetRole::Client; }
		/// <returns>True on the machine that decides (the host, or a game played alone).</returns>
		bool HasAuthority() const { return role == NetRole::Host || role == NetRole::Offline; }
		/// <returns>Id of this player: 0 for the host, 1, 2... for clients.</returns>
		int GetLocalId() const { return localId; }
		/// <returns>Every player, this one included, sorted by id (only this one when offline).</returns>
		std::vector<NetPlayer> GetPlayers() const;
		/// <returns>The name of a player ("Player 3" when unknown).</returns>
		std::string GetPlayerName(int _playerId) const;
		/// <summary>Changes the name of this player, also during a game (the others see it).</summary>
		void SetPlayerName(const std::string& _name);
		/// <returns>The ids of the other players (the clients for a host; the host and the other clients for a client).</returns>
		std::vector<int> GetOtherPlayers() const;
		/// <returns>Number of players in the game, this one included (1 when offline).</returns>
		int GetPlayerCount() const;
		/// <summary>Round trip time to the host in milliseconds (a client only).</summary>
		float GetPingMs() const { return pingMs; }
		bool IsUdpReady() const { return role == NetRole::Host || udpReady; }
		std::string GetStatusText() const;
		const NetStats& GetStats() const { return stats; }

		// ---- Messages

		/// <summary>Sends a message to _target. Offline, it only reaches this game (target All).</summary>
		void Send(const std::string& _name, const std::vector<PropertyValue>& _args = { }, NetTarget _target = NetTarget::All, NetChannel _channel = NetChannel::Reliable);

		/// <summary>Sends a message to one player (0 = the host).</summary>
		void SendTo(int _playerId, const std::string& _name, const std::vector<PropertyValue>& _args = { }, NetChannel _channel = NetChannel::Reliable);

		/// <summary>
		/// Calls _callback for each message with this name. Keep the handle: the callback stops when it is destroyed.
		/// </summary>
		Event<const NetMessage&>::ListenerHandle Subscribe(const std::string& _name, std::function<void(const NetMessage&)> _callback);

		/// <summary>One value of a message, from the usual C++ types: Args(10, "Alice", 2.5, FVector2(1, 2), true).</summary>
		template <typename T>
		static PropertyValue Arg(const T& _value)
		{
			using V = std::remove_cvref_t<T>;
			if constexpr (std::is_same_v<V, bool> || std::is_same_v<V, ::Bool>)						return PropertyValue(static_cast<bool>(_value));
			else if constexpr (std::is_same_v<V, ::Int>)												return PropertyValue(static_cast<int>(_value));
			else if constexpr (std::is_same_v<V, ::Float>)											return PropertyValue(static_cast<float>(_value));
			else if constexpr (std::is_same_v<V, ::String>)											return PropertyValue(_value.StdString());
			else if constexpr (std::is_integral_v<V> || std::is_enum_v<V>)									return PropertyValue(static_cast<int>(_value));
			else if constexpr (std::is_floating_point_v<V>)													return PropertyValue(static_cast<float>(_value));
			else if constexpr (std::is_convertible_v<const V&, std::string>)								return PropertyValue(std::string(_value));
			else																							return PropertyValue(_value);	// FVector2, Color, PropertyValue
		}

		/// <summary>The values of a message: net->Send("Hit", NetworkManager::Args(damage, "sword"));</summary>
		template <typename... Ts>
		static std::vector<PropertyValue> Args(const Ts&... _values)
		{
			return std::vector<PropertyValue>{ Arg(_values)... };
		}

		// ---- Shared values: the same on every machine, kept by the host

		/// <summary>Sets a value for everybody. A client asks the host, so the change shows a moment later (onSharedChanged).</summary>
		void SetShared(const std::string& _key, const PropertyValue& _value);

		/// <summary>Adds _amount to a number (int or float) for everybody. Done by the host: two players adding at once both count.</summary>
		void AddShared(const std::string& _key, float _amount);

		/// <returns>A shared value, or _fallback when it was never set.</returns>
		PropertyValue GetShared(const std::string& _key, const PropertyValue& _fallback = PropertyValue(0)) const;

		/// <returns>A shared value of this type (an int asked as a float is converted), or _fallback.</returns>
		template <typename T>
		T GetShared(const std::string& _key, const T& _fallback) const
		{
			const auto _found = shared.find(_key);
			if (_found == shared.end()) return _fallback;
			if (const T* _value = std::get_if<T>(&_found->second)) return *_value;
			if constexpr (std::is_same_v<T, float>)	if (const int* _int = std::get_if<int>(&_found->second)) return static_cast<float>(*_int);
			if constexpr (std::is_same_v<T, int>)	if (const float* _float = std::get_if<float>(&_found->second)) return static_cast<int>(*_float);
			return _fallback;
		}

		bool HasShared(const std::string& _key) const { return shared.contains(_key); }

		const std::map<std::string, PropertyValue>& GetSharedValues() const { return shared; }

		// ---- Replicated actors (NetworkReplication.cpp)

		/// <summary>
		/// Creates an actor of this class (by name) on every machine. A client asks the host to do it; the new actor belongs to the
		/// player who asked (NetworkSyncComponent::IsLocalOwner). Needs a NetworkSyncComponent on the class, or one is added.
		/// </summary>
		void Spawn(const std::string& _className, const FVector2& _position = FVector2());

		/// <summary>Destroys a spawned actor on every machine. The host can despawn any of them, a client only its own.</summary>
		void Despawn(int _networkId);

		/// <summary>Host: tells the clients that a spawned actor is gone (called when it is destroyed on the host).</summary>
		void ForgetSpawn(int _networkId);

		/// <summary>
		/// Calls a function of a synced actor (a KFUNCTION of the actor or of one of its components, found by name) on the machines of
		/// _target. _args: nothing, or the one argument of the function.
		/// </summary>
		void CallRPC(int _networkId, const std::string& _function, NetTarget _target = NetTarget::All, const std::vector<PropertyValue>& _args = { });

		/// <returns>The synced component with this network id on this machine, or nullptr.</returns>
		NetworkSyncComponent* FindSynced(int _networkId) const;

		void Register(int _networkId, NetworkSyncComponent* _component);
		void Unregister(int _networkId, NetworkSyncComponent* _component);

		// ---- Loop

		/// <summary>Called every frame by the Engine.</summary>
		void Update();

	private:
		void Deliver(const NetMessage& _message);
		void SendPacket(sf::TcpSocket& _socket, sf::Packet& _packet);
		void SendMessageTo(Peer& _peer, const NetMessage& _message, NetChannel _channel);
		void SendToServer(const NetMessage& _message, int _target, NetChannel _channel);
		void Route(const NetMessage& _message, int _target, const Peer* _from, NetChannel _channel);
		void SendUdp(const sf::Packet& _packet, const std::optional<sf::IpAddress>& _address, unsigned short _port);
		void CountReceived(const sf::Packet& _packet);
		Peer* FindPeer(int _id);
		void DropPeer(Peer& _peer, const std::string& _reason);
		void Reset();
		void EndGame(const std::string& _reason, bool _connectionFailed);
		void BroadcastPlayers();
		void SetPlayersFromHost(std::vector<NetPlayer> _players);

		void UpdateHost(float _dt);
		void UpdateClient(float _dt);
		void UpdateDiscovery(float _dt);
		void UpdateStats(float _dt);
		void ReadHostSocket(Peer& _peer);
		void ReadServerSocket();
		void ReadUdp();
		void HandleHostPacket(Peer& _peer, sf::Packet& _packet);
		void HandleServerPacket(sf::Packet& _packet);
		void HandleJoiningPacket(Joining& _join, sf::Packet& _packet, bool& _accepted);
		void ReplicationPeerJoined(int _id);
		void ReplicationPeerLeft(int _id);
		void ReplicationReset();
		void DestroySpawnedActors();
		bool HandleInternal(const NetMessage& _message);
		void ApplyShared(const std::string& _key, const PropertyValue& _value);
		void InvokeRPC(int _networkId, const std::string& _function, const std::vector<PropertyValue>& _args);
		Actor* CreateNetworkedActor(int _id, const std::string& _className, int _owner, const FVector2& _position);
		void SpawnFor(int _owner, const std::string& _className, const FVector2& _position);
	};

}
