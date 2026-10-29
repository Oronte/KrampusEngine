#pragma once
#include "Component.h"
#include "InputComponent.h"
#include "Network/NetworkManager.h"
#include "Reflection/EventList.h"

namespace Krampus
{

	/// <summary>
	/// Reacts to what happens on the network by calling functions chosen in the Inspector (event lists), with no code:
	/// the game is hosted / joined / lost, a player comes or goes, or a message with a given name arrives.
	/// </summary>
	class NetworkEventsComponent : public Component
	{
		std::vector<Event<>::ListenerHandle>	handles;
		std::vector<Event<const NetMessage&>::ListenerHandle>	messageHandles;

	public:
		EventList	onHosted;
		EventList	onConnected;
		EventList	onConnectionFailed;
		EventList	onDisconnected;
		EventList	onPlayerJoined;
		EventList	onPlayerLeft;
		/// <summary>A shared value changed (NetworkManager::SetShared / AddShared).</summary>
		EventList	onSharedChanged;
		/// <summary>SearchLan found a game.</summary>
		EventList	onLanGameFound;

		/// <summary>Messages (by name) that call functions when they arrive. Call ApplyMessages after changing the list.</summary>
		std::vector<InputComponent::Binding>	messages;

		NetworkEventsComponent(Actor* _owner);

		void ApplyMessages();
	};

}
