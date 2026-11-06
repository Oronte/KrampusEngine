#pragma once
#include "NetSocket.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Krampus
{
	/// <summary>A game other players can join through Steam (a Steam lobby made by HostSteam).</summary>
	struct SteamLobby
	{
		std::uint64_t	id			= 0;
		std::uint64_t	host		= 0;	// SteamID64 of the host: what JoinSteam needs
		std::string		name;				// Steam name of the host
		int				players		= 0;
		int				maxPlayers	= 0;
	};

	/// <summary>A Steam friend (GetFriends): online ones only.</summary>
	struct SteamFriend
	{
		std::uint64_t	id			= 0;	// SteamID64
		std::string		name;
		bool			inThisGame	= false;	// plays this game (same AppID)
		std::uint64_t	lobby		= 0;		// the lobby he is in (a game he hosts or joined), 0 none: JoinLobby joins it
	};

	/// <summary>
	/// Everything Steam: the connection to the Steam client, connections between players (Steam Networking Sockets, through the
	/// Steam relays: no port to open, no address to type) and lobbies (find a game, invite a friend, join from the Steam overlay).
	/// Without the Steamworks SDK in ThirdParty/Steamworks (KRAMPUS_STEAM), Init always fails and the rest does nothing.
	/// Owned by NetworkManager, which calls Update every frame.
	/// </summary>
	class SteamNet
	{
		struct Impl;
		std::unique_ptr<Impl> impl;

	public:
		/// <summary>A lobby was entered (we joined one, or accepted an invitation): the SteamID64 of its host. NetworkManager joins it.</summary>
		std::function<void(std::uint64_t)>	onLobbyJoined;
		std::function<void()>				onLobbiesFound;
		/// <summary>JoinLobby failed (lastError says why).</summary>
		std::function<void()>				onLobbyFailed;

		/// <summary>Why the last call failed (shown to the player).</summary>
		std::string							lastError;

		SteamNet();
		~SteamNet();

		/// <returns>True when this build was made with the Steamworks SDK.</returns>
		static bool IsCompiled();

		/// <summary>Connects to the Steam client (which must be running). AppID 480 is the test app of Valve, "Spacewar".</summary>
		bool Init(int _appId);
		bool IsReady() const;
		void Update();

		std::string GetPersonaName() const;
		std::uint64_t GetLocalId() const;

		// ---- Connections
		bool Listen();
		void StopListen();
		/// <returns>A player who connected to this host since the last call (already accepted), or null.</returns>
		std::unique_ptr<NetSocket> Accept();
		/// <returns>The connection to a host (GetState tells when it is ready), or null.</returns>
		std::unique_ptr<NetSocket> Connect(std::uint64_t _hostId);

		// ---- Lobbies
		void CreateLobby(int _maxPlayers, bool _friendsOnly, const std::string& _gameName);
		void LeaveLobby();
		void InviteFriends();
		/// <summary>Sends an invitation to the lobby we host (a Steam chat message; no overlay needed). False without a lobby.</summary>
		bool InviteFriend(std::uint64_t _friendId);
		/// <returns>The friends who are online: those playing this game first, then by name.</returns>
		std::vector<SteamFriend> GetFriends() const;
		void FindLobbies(const std::string& _gameName);
		void JoinLobby(std::uint64_t _lobbyId);
		const std::vector<SteamLobby>& GetLobbies() const;
	};
}
