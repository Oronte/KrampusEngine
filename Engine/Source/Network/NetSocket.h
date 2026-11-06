#pragma once
#include <SFML/Network.hpp>

namespace Krampus
{
	/// <summary>
	/// One reliable connection to a player, whatever carries it: an SFML TCP socket or a Steam connection (SteamNet.cpp).
	/// The calls look like the ones of sf::TcpSocket so the host and the client code do not care which one they talk to.
	/// </summary>
	class NetSocket
	{
	public:
		enum class State { Connecting, Connected, Failed };

		virtual ~NetSocket() = default;

		/// <summary>Sends a whole packet (reliable, in order).</summary>
		virtual sf::Socket::Status send(sf::Packet& _packet) = 0;

		/// <summary>Takes the next packet if one arrived. NotReady when there is none, Disconnected when the other side left.</summary>
		virtual sf::Socket::Status receive(sf::Packet& _packet) = 0;

		virtual void disconnect() = 0;
		virtual void setBlocking(bool) { }

		/// <summary>A packet that may get lost (positions). False when this connection has no such channel (TCP uses the UDP socket instead).</summary>
		virtual bool sendUnreliable(const sf::Packet&) { return false; }

		virtual bool IsSteam() const { return false; }
		virtual State GetState() { return State::Connected; }
	};

	/// <summary>A TCP connection. tcp is public: the listener accepts into it and the client connects with it.</summary>
	class TcpNetSocket final : public NetSocket
	{
	public:
		sf::TcpSocket tcp;

		sf::Socket::Status send(sf::Packet& _packet) override { return tcp.send(_packet); }
		sf::Socket::Status receive(sf::Packet& _packet) override { return tcp.receive(_packet); }
		void disconnect() override { tcp.disconnect(); }
		void setBlocking(const bool _blocking) override { tcp.setBlocking(_blocking); }
	};
}
