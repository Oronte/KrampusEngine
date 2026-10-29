#include "NetworkEventsComponent.h"
#include "Network/NetworkManager.h"

Krampus::NetworkEventsComponent::NetworkEventsComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(NetworkEventsComponent);

	NetworkManager* _net = GetWorld()->GetNetwork();
	handles.push_back(_net->onHosted.AddListener([this]() { onHosted.Invoke(this); }));
	handles.push_back(_net->onConnected.AddListener([this]() { onConnected.Invoke(this); }));
	handles.push_back(_net->onConnectionFailed.AddListener([this]() { onConnectionFailed.Invoke(this); }));
	handles.push_back(_net->onDisconnected.AddListener([this]() { onDisconnected.Invoke(this); }));
	handles.push_back(_net->onClientJoined.AddListener([this](int) { onPlayerJoined.Invoke(this); }));
	handles.push_back(_net->onClientLeft.AddListener([this](int) { onPlayerLeft.Invoke(this); }));
}

void Krampus::NetworkEventsComponent::ApplyMessages()
{
	messageHandles.clear();
	NetworkManager* _net = GetWorld()->GetNetwork();
	for (const InputComponent::Binding& _binding : messages)
		messageHandles.push_back(_net->Subscribe(_binding.source, [this, _calls = _binding.calls](const NetMessage&) { _calls.Invoke(this); }));
}
