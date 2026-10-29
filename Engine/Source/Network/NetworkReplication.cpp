#include "NetworkManager.h"
#include "Core/Engine.h"
#include "Actors/Actor.h"
#include "Components/NetworkSyncComponent.h"
#include "GameFramework/Level.h"
#include "Managers/LevelManager.h"
#include "Reflection/Reflection.h"

// The part of the NetworkManager that makes actors exist on every machine: states ("__sync"), creation ("__spawn") and removal ("__despawn").
// These messages have names that start with "__": they are handled here and never reach the game's subscribers.

using namespace Krampus;

void NetworkManager::Register(const int _networkId, NetworkSyncComponent* _component)
{
	if (const auto _found = synced.find(_networkId); _found != synced.end() && _found->second != _component)
		LOG_WARNING("Network: two actors have the network id " + std::to_string(_networkId) + " (\"" + _component->GetOwner()->GetName().StdString() + "\"): give them different names");
	synced[_networkId] = _component;
}

void NetworkManager::Unregister(const int _networkId, NetworkSyncComponent* _component)
{
	if (const auto _found = synced.find(_networkId); _found != synced.end() && _found->second == _component)
		synced.erase(_found);
}

bool NetworkManager::HandleInternal(const NetMessage& _message)
{
	if (!_message.name.starts_with("__")) return false;

	if (_message.name == "__sync")
	{
		if (const auto _found = synced.find(_message.Int(0)); _found != synced.end())
			_found->second->ReceiveState(_message);
	}
	else if (_message.name == "__spawn")			// host -> clients: id, class, owner, x, y
	{
		if (role == NetRole::Client)
			CreateNetworkedActor(_message.Int(0), _message.String(1), _message.Int(2), FVector2(_message.Float(3), _message.Float(4)));
	}
	else if (_message.name == "__spawnreq")		// client -> host: class, x, y
	{
		if (role == NetRole::Host)
			SpawnFor(_message.sender, _message.String(0), FVector2(_message.Float(1), _message.Float(2)));
	}
	else if (_message.name == "__despawn")			// host -> clients: id
	{
		if (role == NetRole::Client)
			if (const auto _found = synced.find(_message.Int(0)); _found != synced.end())
				_found->second->GetOwner()->Destroy();
	}
	return true;
}

Actor* NetworkManager::CreateNetworkedActor(const int _id, const std::string& _className, const int _owner, const FVector2& _position)
{
	Level* _level = GetWorld()->GetLevelManager()->GetCurrentLevel();
	const ClassInfo* _class = ClassRegistry::Find(_className);
	if (!_level || !_class || !_class->factory)
	{
		LOG_WARNING("Network: cannot create an actor of class \"" + _className + "\" (unknown class, or no level)");
		return nullptr;
	}

	Actor* _actor = _class->factory(_level);
	if (!_actor) return nullptr;

	NetworkSyncComponent* _sync = _actor->GetComponent<NetworkSyncComponent>();
	if (!_sync) _sync = _actor->AddComponent<NetworkSyncComponent>();
	_sync->networkId = Int(_id);
	_sync->ownerId = _owner;
	_sync->spawnedByNetwork = true;
	_actor->SetPosition(_position);
	_actor->SetName(String(_className + " " + std::to_string(_id & 0x3FFFFFFF)));
	LOG(VerbosityType::Log, "Network: created " + _className + " (id " + std::to_string(_id) + ", owner " + std::to_string(_owner) + ")");
	return _actor;
}

void NetworkManager::Spawn(const std::string& _className, const FVector2& _position)
{
	if (role == NetRole::Host)
		SpawnFor(0, _className, _position);
	else if (role == NetRole::Client)
		Send("__spawnreq", { _className, _position.x, _position.y }, NetTarget::Host);
	else
		LOG_WARNING("Network: Spawn needs a game to host or join first");
}

void NetworkManager::SpawnFor(const int _owner, const std::string& _className, const FVector2& _position)
{
	const int _id = nextSpawnId++;
	if (!CreateNetworkedActor(_id, _className, _owner, _position)) return;

	spawned.push_back(SpawnRecord{ _id, _className, _owner, _position });
	Send("__spawn", { _id, _className, _owner, _position.x, _position.y }, NetTarget::Others);
}

void NetworkManager::ForgetSpawn(const int _networkId)
{
	if (role != NetRole::Host) return;
	const auto _record = std::find_if(spawned.begin(), spawned.end(), [&](const SpawnRecord& _r) { return _r.id == _networkId; });
	if (_record == spawned.end()) return;

	spawned.erase(_record);
	Send("__despawn", { _networkId }, NetTarget::Others);
}

void NetworkManager::Despawn(const int _networkId)
{
	if (role != NetRole::Host)
	{
		LOG_WARNING("Network: only the host can despawn an actor");
		return;
	}
	if (const auto _found = synced.find(_networkId); _found != synced.end())
		_found->second->GetOwner()->Destroy();		// its component then calls ForgetSpawn
	else
		ForgetSpawn(_networkId);
}

void NetworkManager::ReplicationPeerJoined(const int _id)
{
	// The newcomer gets the actors that were spawned before it came.
	for (const SpawnRecord& _record : spawned)
		SendTo(_id, "__spawn", { _record.id, _record.className, _record.owner, _record.position.x, _record.position.y });
}

void NetworkManager::ReplicationPeerLeft(const int _id)
{
	// A player's actors go away with it.
	std::vector<int> _owned;
	for (const SpawnRecord& _record : spawned)
		if (_record.owner == _id) _owned.push_back(_record.id);
	for (const int _networkId : _owned)
		Despawn(_networkId);
}

void NetworkManager::ReplicationReset()
{
	spawned.clear();
	nextSpawnId = 0x40000000;
}
