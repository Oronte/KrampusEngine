#pragma once
#include "Core/KrampusObject.h"
#include "Reflection/EventList.h"
#include <SFML/Network.hpp>
#include <future>

// Networking for the game, on top of SFML (TCP + UDP). One host (the server, which is also a player: id 0) and any number of clients.
//
//   Engine::GetNetwork()->Host();                       // or Join("192.168.1.20")
//   net->Send("Score", { 10, std::string("Alice") });   // to everybody
//   auto _handle = net->Subscribe("Score", [](const NetMessage& _message) { int _points = _message.Int(0); });
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
	};

	/// <summary>
	/// Hosts or joins a game, sends and receives messages, keeps the list of players. Owned by the Engine (Engine::GetNetwork()).
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

		// Host
		sf::TcpListener							listener;
		std::vector<std::unique_ptr<Peer>>		peers;
		std::vector<Joining>					joining;

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
		std::vector<int>						otherClients;		// ids of the other players, as the host announced them
		float									connectAge		= 0.f;

		sf::UdpSocket							udp;
		std::unordered_map<std::string, Event<const NetMessage&>>		subscribers;

		// Replication (NetworkReplication.cpp)
		std::unordered_map<int, NetworkSyncComponent*>	synced;
		std::vector<SpawnRecord>				spawned;
		int										nextSpawnId		= 0x40000000;

	public:
		/// <summary>Every message, with its name inside.</summary>
		Event<const NetMessage&>				onMessage;

		Event<>									onHosted;				// Host() succeeded
		Event<>									onConnected;			// Join() succeeded
		Event<>									onConnectionFailed;		// Join() failed (nobody there, refused, timeout)
		Event<>									onDisconnected;			// the connection ended (or Disconnect() was called)
		Event<int>								onClientJoined;			// a player arrived (host and clients): its id
		Event<int>								onClientLeft;			// a player left: its id

		/// <summary>Seconds without anything from the other side before the connection is dropped.</summary>
		float									timeout			= 10.f;
		/// <summary>Maximum number of clients the host accepts.</summary>
		int										maxClients		= 8;
		/// <summary>Name given to the host / sent when joining.</summary>
		std::string								playerName		= "Player";

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

		/// <summary>Join with the address and port of the Network settings.</summary>
		void Join();

		/// <summary>Leaves the game (a host closes it: clients are disconnected).</summary>
		void Disconnect();

		// ---- State

		NetRole GetRole() const { return role; }
		bool IsHost() const { return role == NetRole::Host; }
		bool IsClient() const { return role == NetRole::Client || role == NetRole::Connecting; }
		/// <returns>True when messages can go somewhere: hosting, or connected to a host.</returns>
		bool IsOnline() const { return role == NetRole::Host || role == NetRole::Client; }
		/// <returns>Id of this player: 0 for the host, 1, 2... for clients.</returns>
		int GetLocalId() const { return localId; }
		/// <returns>The ids of the other players (the clients for a host; the host and the other clients for a client).</returns>
		std::vector<int> GetOtherPlayers() const;
		/// <returns>Number of players in the game, this one included (1 when offline).</returns>
		int GetPlayerCount() const;
		/// <summary>Round trip time to the host in milliseconds (a client only).</summary>
		float GetPingMs() const { return pingMs; }
		bool IsUdpReady() const { return role == NetRole::Host || udpReady; }
		std::string GetStatusText() const;

		// ---- Messages

		/// <summary>Sends a message to _target. Offline, it only reaches this game (target All).</summary>
		void Send(const std::string& _name, const std::vector<PropertyValue>& _args = { }, NetTarget _target = NetTarget::All, NetChannel _channel = NetChannel::Reliable);

		/// <summary>Sends a message to one player (0 = the host).</summary>
		void SendTo(int _playerId, const std::string& _name, const std::vector<PropertyValue>& _args = { }, NetChannel _channel = NetChannel::Reliable);

		/// <summary>
		/// Calls _callback for each message with this name. Keep the handle: the callback stops when it is destroyed.
		/// </summary>
		Event<const NetMessage&>::ListenerHandle Subscribe(const std::string& _name, std::function<void(const NetMessage&)> _callback);

		// ---- Replicated actors (NetworkReplication.cpp)

		/// <summary>
		/// Creates an actor of this class (by name) on every machine. A client asks the host to do it; the new actor belongs to the
		/// player who asked (NetworkSyncComponent::IsLocalOwner). Needs a NetworkSyncComponent on the class, or one is added.
		/// </summary>
		void Spawn(const std::string& _className, const FVector2& _position = FVector2());

		/// <summary>Destroys a spawned actor on every machine (host only).</summary>
		void Despawn(int _networkId);

		/// <summary>Host: tells the clients that a spawned actor is gone (called when it is destroyed on the host).</summary>
		void ForgetSpawn(int _networkId);

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
		Peer* FindPeer(int _id);
		void DropPeer(Peer& _peer, const char* _reason);
		void Reset();

		void UpdateHost(float _dt);
		void UpdateClient(float _dt);
		void ReadHostSocket(Peer& _peer);
		void ReadServerSocket();
		void ReadUdp();
		void HandleHostPacket(Peer& _peer, sf::Packet& _packet);
		void HandleServerPacket(sf::Packet& _packet);
		void HandleJoiningPacket(Joining& _join, sf::Packet& _packet, bool& _accepted);
		void ReplicationPeerJoined(int _id);
		void ReplicationPeerLeft(int _id);
		void ReplicationReset();
		bool HandleInternal(const NetMessage& _message);
		Actor* CreateNetworkedActor(int _id, const std::string& _className, int _owner, const FVector2& _position);
		void SpawnFor(int _owner, const std::string& _className, const FVector2& _position);
	};

}
