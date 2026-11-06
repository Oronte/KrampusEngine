#include "NetworkManager.h"
#include "NetworkProtocol.h"
#include "Core/Engine.h"
#include <random>

using namespace Krampus;
using namespace Krampus::NetProtocol;

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
