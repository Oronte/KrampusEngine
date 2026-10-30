#pragma once
#include "Reflection/Reflection.h"
#include "Network/NetworkManager.h"
#include <deque>

namespace Krampus
{

	class Widget;
	class LabelWidgetComponent;

	/// <summary>
	/// The network demo (scene DemoNetwork): put it in a scene, press Play (or run KrampusGame --scene DemoNetwork), and in a second
	/// window join the first one. Everything the NetworkManager does is on a key:
	///
	///   H host      J join (Settings > Network > address)      L search the LAN, then 1..9 join a game found      X leave
	///   ZQSD / WASD / arrows move your player (NetworkSyncComponent)      Space flash (RPC on every machine)      F / G size (synced property)
	///   E +1 to the shared score      R score back to 0      C chat message to everybody      N new random name      K kick (host)
	///
	/// Two windows on one PC: start KrampusGame.exe twice, "--scene DemoNetwork --host" and "--scene DemoNetwork --join 127.0.0.1",
	/// or press H in one and J (or L then 1) in the other.
	/// </summary>
	class DemoNetworkManager : public Actor
	{
		KACTOR(DemoNetworkManager, Actor)

	public:
		DemoNetworkManager(Level* _level);

		KPROPERTY(String, playerClass) = "DemoNetworkPlayer";		// what Spawn makes for each player
		KPROPERTY_RANGE(Int, chatLines, 3, 20) = 9;				// lines of the event log

	protected:
		void Tick(const Float& _deltaTime) override;
		void BeginDestroy() override;

	private:
		bool									started		= false;
		float									refresh		= 0.f;
		std::map<sf::Keyboard::Key, bool>		held;
		std::deque<std::string>					eventLog;
		ListenerGroup							listeners;
		Event<const NetMessage&>::ListenerHandle	chatHandle;

		LabelWidgetComponent*					statusText	= nullptr;
		LabelWidgetComponent*					logText		= nullptr;
		LabelWidgetComponent*					helpText	= nullptr;

		void Start();
		void BuildScene();
		void HandleKeys();
		bool Pressed(sf::Keyboard::Key _key);		// true once per press, only in the window that has the focus
		void SpawnMyPlayer();
		void AddLog(const std::string& _line);
		void RefreshTexts();
	};

}
