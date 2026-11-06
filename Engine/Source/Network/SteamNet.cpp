#include "SteamNet.h"
#include "Utilities/Macro.h"
#include <cstdlib>
#include <deque>

#if KRAMPUS_STEAM
#include <steam/steam_api.h>
#include <steam/isteamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#endif

using namespace Krampus;

#if KRAMPUS_STEAM

namespace
{
	// sf::Packet fills itself from received bytes through a protected function.
	struct RawPacket : sf::Packet
	{
		void Fill(const void* _data, const std::size_t _size) { onReceive(_data, _size); }
	};

	// An unreliable message has to fit in one datagram.
	constexpr std::uint32_t MaxUnreliableSize = 1100;

	/// <summary>One Steam connection, as a NetSocket: a Steam message is a whole packet, so no framing is needed.</summary>
	class SteamNetSocket final : public NetSocket
	{
		HSteamNetConnection connection;

	public:
		explicit SteamNetSocket(const HSteamNetConnection _connection) : connection(_connection) { }
		~SteamNetSocket() override { disconnect(); }

		sf::Socket::Status send(sf::Packet& _packet) override
		{
			if (connection == k_HSteamNetConnection_Invalid) return sf::Socket::Status::Disconnected;
			const EResult _result = SteamNetworkingSockets()->SendMessageToConnection(connection, _packet.getData(),
				static_cast<std::uint32_t>(_packet.getDataSize()), k_nSteamNetworkingSend_Reliable, nullptr);
			if (_result == k_EResultOK) return sf::Socket::Status::Done;
			if (_result == k_EResultLimitExceeded) return sf::Socket::Status::Partial;	// the send buffer is full: the caller tries again
			if (_result == k_EResultNoConnection) return sf::Socket::Status::Disconnected;
			return sf::Socket::Status::Error;
		}

		sf::Socket::Status receive(sf::Packet& _packet) override
		{
			if (connection == k_HSteamNetConnection_Invalid) return sf::Socket::Status::Disconnected;

			SteamNetworkingMessage_t* _message = nullptr;
			if (SteamNetworkingSockets()->ReceiveMessagesOnConnection(connection, &_message, 1) > 0 && _message)
			{
				RawPacket _raw;
				_raw.Fill(_message->m_pData, static_cast<std::size_t>(_message->m_cbSize));
				_message->Release();
				_packet = _raw;
				return sf::Socket::Status::Done;
			}
			return GetState() == State::Failed ? sf::Socket::Status::Disconnected : sf::Socket::Status::NotReady;
		}

		void disconnect() override
		{
			if (connection == k_HSteamNetConnection_Invalid) return;
			SteamNetworkingSockets()->CloseConnection(connection, k_ESteamNetConnectionEnd_App_Generic, "left the game", true);
			connection = k_HSteamNetConnection_Invalid;
		}

		bool sendUnreliable(const sf::Packet& _packet) override
		{
			if (connection == k_HSteamNetConnection_Invalid) return false;
			const std::uint32_t _size = static_cast<std::uint32_t>(_packet.getDataSize());
			const int _flags = _size <= MaxUnreliableSize ? k_nSteamNetworkingSend_Unreliable : k_nSteamNetworkingSend_Reliable;
			return SteamNetworkingSockets()->SendMessageToConnection(connection, _packet.getData(), _size, _flags, nullptr) == k_EResultOK;
		}

		bool IsSteam() const override { return true; }

		State GetState() override
		{
			SteamNetConnectionInfo_t _info;
			if (connection == k_HSteamNetConnection_Invalid || !SteamNetworkingSockets()->GetConnectionInfo(connection, &_info)) return State::Failed;
			switch (_info.m_eState)
			{
			case k_ESteamNetworkingConnectionState_Connected:		return State::Connected;
			case k_ESteamNetworkingConnectionState_Connecting:
			case k_ESteamNetworkingConnectionState_FindingRoute:	return State::Connecting;
			default:												return State::Failed;
			}
		}
	};
}

struct SteamNet::Impl
{
	SteamNet*							owner;
	HSteamListenSocket					listen		= k_HSteamListenSocket_Invalid;
	std::deque<HSteamNetConnection>		incoming;
	CSteamID							lobby;
	bool								inLobby		= false;
	bool								lobbyWanted	= false;	// CreateLobby / JoinLobby asked, LeaveLobby not since: an answer that comes late is left at once
	std::string							gameName;
	std::vector<SteamLobby>				lobbies;

	CCallResult<Impl, LobbyCreated_t>	createdCall;
	CCallResult<Impl, LobbyMatchList_t>	listCall;
	CCallResult<Impl, LobbyEnter_t>		enterCall;

	explicit Impl(SteamNet* _owner) : owner(_owner) { }

	STEAM_CALLBACK(Impl, OnConnectionStatus, SteamNetConnectionStatusChangedCallback_t);
	STEAM_CALLBACK(Impl, OnJoinRequested, GameLobbyJoinRequested_t);

	void OnCreated(LobbyCreated_t* _result, bool _failed)
	{
		if (_failed || _result->m_eResult != k_EResultOK)
		{
			owner->lastError = "Steam could not create the lobby (friends cannot find this game)";
			return;
		}
		if (!lobbyWanted)	// the game was closed before Steam made the lobby: nobody must find it
		{
			SteamMatchmaking()->LeaveLobby(CSteamID(_result->m_ulSteamIDLobby));
			return;
		}
		lobby = CSteamID(_result->m_ulSteamIDLobby);
		inLobby = true;
		SteamMatchmaking()->SetLobbyData(lobby, "host", std::to_string(SteamUser()->GetSteamID().ConvertToUint64()).c_str());
		SteamMatchmaking()->SetLobbyData(lobby, "game", gameName.c_str());
		SteamMatchmaking()->SetLobbyData(lobby, "name", SteamFriends()->GetPersonaName());
	}

	void OnList(LobbyMatchList_t* _result, bool _failed)
	{
		lobbies.clear();
		for (std::uint32_t _i = 0; !_failed && _i < _result->m_nLobbiesMatching; ++_i)
		{
			const CSteamID _id = SteamMatchmaking()->GetLobbyByIndex(static_cast<int>(_i));
			SteamLobby _entry;
			_entry.id = _id.ConvertToUint64();
			_entry.host = std::strtoull(SteamMatchmaking()->GetLobbyData(_id, "host"), nullptr, 10);
			_entry.name = SteamMatchmaking()->GetLobbyData(_id, "name");
			_entry.players = SteamMatchmaking()->GetNumLobbyMembers(_id);
			_entry.maxPlayers = SteamMatchmaking()->GetLobbyMemberLimit(_id);
			if (_entry.host != 0) lobbies.push_back(std::move(_entry));
		}
		if (owner->onLobbiesFound) owner->onLobbiesFound();
	}

	void OnEnter(LobbyEnter_t* _result, bool _failed)
	{
		if (_failed || _result->m_EChatRoomEnterResponse != k_EChatRoomEnterResponseSuccess)
		{
			owner->lastError = "could not enter the Steam lobby (full, closed, or gone)";
			if (owner->onLobbyFailed) owner->onLobbyFailed();
			return;
		}
		if (!lobbyWanted)	// left (back to the menu) while Steam was answering
		{
			SteamMatchmaking()->LeaveLobby(CSteamID(_result->m_ulSteamIDLobby));
			return;
		}
		lobby = CSteamID(_result->m_ulSteamIDLobby);
		inLobby = true;
		const std::uint64_t _host = std::strtoull(SteamMatchmaking()->GetLobbyData(lobby, "host"), nullptr, 10);
		if (_host != 0 && owner->onLobbyJoined) owner->onLobbyJoined(_host);
	}
};

void SteamNet::Impl::OnConnectionStatus(SteamNetConnectionStatusChangedCallback_t* _info)
{
	if (_info->m_info.m_hListenSocket == k_HSteamListenSocket_Invalid || _info->m_info.m_hListenSocket != listen) return;	// not a player of ours

	switch (_info->m_info.m_eState)
	{
	case k_ESteamNetworkingConnectionState_Connecting:
		if (_info->m_eOldState == k_ESteamNetworkingConnectionState_None) SteamNetworkingSockets()->AcceptConnection(_info->m_hConn);
		break;
	case k_ESteamNetworkingConnectionState_Connected:
		// Through the Steam relays the connection goes Connecting -> FindingRoute -> Connected: any way in counts.
		if (_info->m_eOldState != k_ESteamNetworkingConnectionState_Connected
			&& std::find(incoming.begin(), incoming.end(), _info->m_hConn) == incoming.end())
			incoming.push_back(_info->m_hConn);
		break;
	case k_ESteamNetworkingConnectionState_ClosedByPeer:
	case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
	{
		// Gone before the game took it: free it (one already taken is closed by its SteamNetSocket).
		const auto _waiting = std::find(incoming.begin(), incoming.end(), _info->m_hConn);
		if (_waiting != incoming.end())
		{
			incoming.erase(_waiting);
			SteamNetworkingSockets()->CloseConnection(_info->m_hConn, 0, nullptr, false);
		}
		else if (_info->m_eOldState != k_ESteamNetworkingConnectionState_Connected)
			SteamNetworkingSockets()->CloseConnection(_info->m_hConn, 0, nullptr, false);	// never got in
		break;
	}
	default:
		break;
	}
}

void SteamNet::Impl::OnJoinRequested(GameLobbyJoinRequested_t* _request)
{
	// A friend invited us (or we clicked "Join game" in the Steam overlay).
	lobbyWanted = true;
	enterCall.Set(SteamMatchmaking()->JoinLobby(_request->m_steamIDLobby), this, &Impl::OnEnter);
}

SteamNet::SteamNet() = default;

SteamNet::~SteamNet()
{
	if (impl)
	{
		StopListen();
		LeaveLobby();
		impl.reset();	// unregisters the callbacks while Steam is still there
		SteamAPI_Shutdown();
	}
}

bool SteamNet::IsCompiled() { return true; }
bool SteamNet::IsReady() const { return impl != nullptr; }

bool SteamNet::Init(const int _appId)
{
	if (impl) return true;

	// The AppID comes from the environment: it works from the editor (another folder than the exe) as from a game started by hand.
	const std::string _id = std::to_string(_appId);
	_putenv_s("SteamAppId", _id.c_str());
	_putenv_s("SteamGameId", _id.c_str());

	SteamErrMsg _error = { };
	if (SteamAPI_InitEx(&_error) != k_ESteamAPIInitResult_OK)
	{
		lastError = std::string("Steam is not ready: ") + _error + " (is the Steam client running?)";
		return false;
	}

	SteamNetworkingUtils()->InitRelayNetworkAccess();
	impl = std::make_unique<Impl>(this);
	lastError.clear();
	return true;
}

void SteamNet::Update()
{
	if (impl) SteamAPI_RunCallbacks();
}

std::string SteamNet::GetPersonaName() const { return impl ? SteamFriends()->GetPersonaName() : std::string(); }
std::uint64_t SteamNet::GetLocalId() const { return impl ? SteamUser()->GetSteamID().ConvertToUint64() : 0; }

bool SteamNet::Listen()
{
	if (!impl) return false;
	StopListen();
	impl->listen = SteamNetworkingSockets()->CreateListenSocketP2P(0, 0, nullptr);
	if (impl->listen == k_HSteamListenSocket_Invalid)
	{
		lastError = "Steam could not open the game for players";
		return false;
	}
	return true;
}

void SteamNet::StopListen()
{
	if (!impl) return;
	if (impl->listen != k_HSteamListenSocket_Invalid) SteamNetworkingSockets()->CloseListenSocket(impl->listen);
	impl->listen = k_HSteamListenSocket_Invalid;
	for (const HSteamNetConnection _connection : impl->incoming) SteamNetworkingSockets()->CloseConnection(_connection, 0, nullptr, false);
	impl->incoming.clear();
}

std::unique_ptr<NetSocket> SteamNet::Accept()
{
	if (!impl || impl->incoming.empty()) return nullptr;
	const HSteamNetConnection _connection = impl->incoming.front();
	impl->incoming.pop_front();
	return std::make_unique<SteamNetSocket>(_connection);
}

std::unique_ptr<NetSocket> SteamNet::Connect(const std::uint64_t _hostId)
{
	if (!impl) return nullptr;
	SteamNetworkingIdentity _identity;
	_identity.SetSteamID64(_hostId);
	const HSteamNetConnection _connection = SteamNetworkingSockets()->ConnectP2P(_identity, 0, 0, nullptr);
	if (_connection == k_HSteamNetConnection_Invalid)
	{
		lastError = "Steam could not connect to this player";
		return nullptr;
	}
	return std::make_unique<SteamNetSocket>(_connection);
}

void SteamNet::CreateLobby(const int _maxPlayers, const bool _friendsOnly, const std::string& _gameName)
{
	if (!impl) return;
	LeaveLobby();
	impl->lobbyWanted = true;
	impl->gameName = _gameName;
	impl->createdCall.Set(SteamMatchmaking()->CreateLobby(_friendsOnly ? k_ELobbyTypeFriendsOnly : k_ELobbyTypePublic, _maxPlayers), impl.get(), &Impl::OnCreated);
}

void SteamNet::LeaveLobby()
{
	if (!impl) return;
	impl->lobbyWanted = false;
	if (!impl->inLobby) return;
	SteamMatchmaking()->LeaveLobby(impl->lobby);
	impl->inLobby = false;
}

void SteamNet::InviteFriends()
{
	if (!impl) return;
	if (!impl->inLobby)
	{
		lastError = "no Steam lobby yet: host a game first";
		return;
	}
	SteamFriends()->ActivateGameOverlayInviteDialog(impl->lobby);
}

bool SteamNet::InviteFriend(const std::uint64_t _friendId)
{
	if (!impl) return false;
	if (!impl->inLobby)
	{
		lastError = "no Steam lobby yet: host a game first";
		return false;
	}
	if (!SteamMatchmaking()->InviteUserToLobby(impl->lobby, CSteamID(_friendId)))
	{
		lastError = "Steam could not send the invitation";
		return false;
	}
	return true;
}

std::vector<SteamFriend> SteamNet::GetFriends() const
{
	std::vector<SteamFriend> _friends;
	if (!impl) return _friends;
	const AppId_t _app = SteamUtils()->GetAppID();
	const int _count = SteamFriends()->GetFriendCount(k_EFriendFlagImmediate);
	for (int _i = 0; _i < _count; ++_i)
	{
		const CSteamID _id = SteamFriends()->GetFriendByIndex(_i, k_EFriendFlagImmediate);
		if (SteamFriends()->GetFriendPersonaState(_id) == k_EPersonaStateOffline) continue;
		SteamFriend _friend;
		_friend.id = _id.ConvertToUint64();
		_friend.name = SteamFriends()->GetFriendPersonaName(_id);
		FriendGameInfo_t _game = { };
		if (SteamFriends()->GetFriendGamePlayed(_id, &_game) && _game.m_gameID.AppID() == _app)
		{
			_friend.inThisGame = true;
			if (_game.m_steamIDLobby.IsValid()) _friend.lobby = _game.m_steamIDLobby.ConvertToUint64();
		}
		_friends.push_back(std::move(_friend));
	}
	std::sort(_friends.begin(), _friends.end(), [](const SteamFriend& _a, const SteamFriend& _b)
		{
			if (_a.inThisGame != _b.inThisGame) return _a.inThisGame;
			return _a.name < _b.name;
		});
	return _friends;
}

void SteamNet::FindLobbies(const std::string& _gameName)
{
	if (!impl) return;
	SteamMatchmaking()->AddRequestLobbyListDistanceFilter(k_ELobbyDistanceFilterWorldwide);
	SteamMatchmaking()->AddRequestLobbyListStringFilter("game", _gameName.c_str(), k_ELobbyComparisonEqual);
	impl->listCall.Set(SteamMatchmaking()->RequestLobbyList(), impl.get(), &Impl::OnList);
}

void SteamNet::JoinLobby(const std::uint64_t _lobbyId)
{
	if (!impl) return;
	impl->lobbyWanted = true;
	impl->enterCall.Set(SteamMatchmaking()->JoinLobby(CSteamID(_lobbyId)), impl.get(), &Impl::OnEnter);
}

const std::vector<SteamLobby>& SteamNet::GetLobbies() const
{
	static const std::vector<SteamLobby> none;
	return impl ? impl->lobbies : none;
}

#else	// no Steamworks SDK in ThirdParty: the same functions, doing nothing

struct SteamNet::Impl { };

SteamNet::SteamNet() = default;
SteamNet::~SteamNet() = default;
bool SteamNet::IsCompiled() { return false; }
bool SteamNet::IsReady() const { return false; }

bool SteamNet::Init(const int)
{
	lastError = "this build has no Steam (put the Steamworks SDK in ThirdParty/Steamworks/sdk and rebuild)";
	return false;
}

void SteamNet::Update() { }
std::string SteamNet::GetPersonaName() const { return { }; }
std::uint64_t SteamNet::GetLocalId() const { return 0; }
bool SteamNet::Listen() { return false; }
void SteamNet::StopListen() { }
std::unique_ptr<NetSocket> SteamNet::Accept() { return nullptr; }
std::unique_ptr<NetSocket> SteamNet::Connect(std::uint64_t) { return nullptr; }
void SteamNet::CreateLobby(int, bool, const std::string&) { }
void SteamNet::LeaveLobby() { }
void SteamNet::InviteFriends() { }
bool SteamNet::InviteFriend(std::uint64_t) { return false; }
std::vector<SteamFriend> SteamNet::GetFriends() const { return { }; }
void SteamNet::FindLobbies(const std::string&) { }
void SteamNet::JoinLobby(std::uint64_t) { }

const std::vector<SteamLobby>& SteamNet::GetLobbies() const
{
	static const std::vector<SteamLobby> none;
	return none;
}

#endif
