#include "NetworkManager.h"
#include "Core/Engine.h"
#include "Actors/Actor.h"
#include "Components/NetworkSyncComponent.h"
#include "GameFramework/Level.h"
#include "Managers/LevelManager.h"
#include "Reflection/Reflection.h"

// The part of the NetworkManager that makes actors exist on every machine: states ("__sync"), creation ("__spawn"), removal ("__despawn"),
// function calls on actors ("__rpc") and the shared values ("__shared"). These messages have names that start with "__": they are handled
// here and never reach the game's subscribers.

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

NetworkSyncComponent* NetworkManager::FindSynced(const int _networkId) const
{
	const auto _found = synced.find(_networkId);
	return _found == synced.end() ? nullptr : _found->second;
}

bool NetworkManager::HandleInternal(const NetMessage& _message)
{
	if (!_message.name.starts_with("__")) return false;

	if (_message.name == "__sync")
	{
		if (NetworkSyncComponent* _component = FindSynced(_message.Int(0)))
			_component->ReceiveState(_message);
	}
	else if (_message.name == "__spawn")			// host -> clients: id, class, owner, x, y
	{
		if (role == NetRole::Client && !FindSynced(_message.Int(0)))
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
			if (NetworkSyncComponent* _component = FindSynced(_message.Int(0)))
				_component->GetOwner()->Destroy();
	}
	else if (_message.name == "__despawnreq")		// client -> host: id (only its own actors)
	{
		if (role == NetRole::Host)
		{
			const int _id = _message.Int(0);
			const auto _record = std::find_if(spawned.begin(), spawned.end(), [&](const SpawnRecord& _r) { return _r.id == _id; });
			if (_record != spawned.end() && _record->owner == _message.sender) Despawn(_id);
			else LOG_WARNING("Network: player " + std::to_string(_message.sender) + " asked to despawn an actor that is not its own");
		}
	}
	else if (_message.name == "__rpc")				// anybody: id, function, then the arguments
	{
		InvokeRPC(_message.Int(0), _message.String(1), std::vector<PropertyValue>(_message.args.begin() + std::min<size_t>(2, _message.args.size()), _message.args.end()));
	}
	else if (_message.name == "__shared")			// host -> clients: key, value
	{
		if (role == NetRole::Client && _message.args.size() >= 2)
			ApplyShared(_message.String(0), _message.args[1]);
	}
	else if (_message.name == "__setreq")			// client -> host: key, value
	{
		if (role == NetRole::Host && _message.args.size() >= 2)
			SetShared(_message.String(0), _message.args[1]);
	}
	else if (_message.name == "__addreq")			// client -> host: key, amount
	{
		if (role == NetRole::Host)
			AddShared(_message.String(0), _message.Float(1));
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
	_actor->SetPosition(_position);
	_actor->SetName(String(_className + " " + std::to_string(_id & 0x3FFFFFFF)));
	_sync->AssignNetworkId(_id, _owner);
	LOG(VerbosityType::Log, "Network: created " + _className + " (id " + std::to_string(_id) + ", owner " + std::to_string(_owner) + ")");
	onActorSpawned.Broadcast(_actor);
	return _actor;
}

void NetworkManager::Spawn(const std::string& _className, const FVector2& _position)
{
	if (role == NetRole::Host)
		SpawnFor(0, _className, _position);
	else if (role == NetRole::Client)
		Send("__spawnreq", { _className, _position.x, _position.y }, NetTarget::Host);
	else if (role == NetRole::Offline)
		CreateNetworkedActor(nextSpawnId++, _className, localId, _position);	// alone: the same actor, here only
	else
		LOG_WARNING("Network: Spawn while joining: wait for onConnected");
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
	NetworkSyncComponent* _component = FindSynced(_networkId);
	if (role == NetRole::Client)
	{
		if (_component && _component->ownerId == localId) Send("__despawnreq", { _networkId }, NetTarget::Host);
		else LOG_WARNING("Network: a client can only despawn its own actors");
		return;
	}
	if (role == NetRole::Offline)
	{
		if (_component) _component->GetOwner()->Destroy();
		return;
	}

	if (_component)
		_component->GetOwner()->Destroy();		// its component then calls ForgetSpawn
	else
		ForgetSpawn(_networkId);
}

void NetworkManager::CallRPC(const int _networkId, const std::string& _function, const NetTarget _target, const std::vector<PropertyValue>& _args)
{
	if (!IsOnline())
	{
		// Alone, this machine is everybody (and the host): only "the others" do not exist.
		if (role == NetRole::Offline && _target != NetTarget::Others) InvokeRPC(_networkId, _function, _args);
		return;
	}

	std::vector<PropertyValue> _message = { _networkId, _function };
	_message.insert(_message.end(), _args.begin(), _args.end());
	Send("__rpc", _message, _target, NetChannel::Reliable);
}

void NetworkManager::InvokeRPC(const int _networkId, const std::string& _function, const std::vector<PropertyValue>& _args)
{
	NetworkSyncComponent* _component = FindSynced(_networkId);
	if (!_component)
	{
		LOG(VerbosityType::Verbose, "Network: RPC \"" + _function + "\" for the unknown network id " + std::to_string(_networkId) + " (actor gone?)");
		return;
	}
	Actor* _actor = _component->GetOwner();
	const PropertyValue* _argument = _args.empty() ? nullptr : &_args[0];

	// The actor first, then its components: the first class that has a function with this name is called.
	const auto _tryCall = [&](IGameObject& _object) -> bool
	{
		const ClassInfo* _class = ClassRegistry::Find(ClassNameOf(_object));
		if (!_class) return false;
		for (const FunctionInfo* _info : _class->GetFunctions())
		{
			if (_info->name != _function) continue;
			if (_info->hasArgument && !_argument)
			{
				LOG_WARNING("Network: RPC \"" + _function + "\" needs an argument");
				return true;
			}
			try { _info->call(&_object, _argument); }
			catch (const std::bad_variant_access&) { LOG_WARNING("Network: RPC \"" + _function + "\" got an argument of the wrong type"); }
			return true;
		}
		return false;
	};

	if (_tryCall(*_actor)) return;
	for (const std::unique_ptr<Component>& _owned : _actor->GetComponents())
		if (_tryCall(*_owned)) return;
	LOG_WARNING("Network: RPC \"" + _function + "\": no KFUNCTION with this name on \"" + _actor->GetName().StdString() + "\" or its components");
}

void NetworkManager::ReplicationPeerJoined(const int _id)
{
	// The newcomer gets the shared values and the actors that were spawned before it came.
	for (const auto& [_key, _value] : shared)
		SendTo(_id, "__shared", { _key, _value });
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
	spawned.clear();	// nextSpawnId keeps counting: an actor spawned alone before hosting keeps an id nobody else gets
}

void NetworkManager::DestroySpawnedActors()
{
	if (!destroySpawnedOnDisconnect) return;

	// Collected first: destroying an actor unregisters its component from 'synced'.
	std::vector<Actor*> _actors;
	for (const auto& [_id, _component] : synced)
		if (_component->spawnedByNetwork) _actors.push_back(_component->GetOwner());
	for (Actor* _actor : _actors)
		_actor->Destroy();
	if (!_actors.empty())
		LOG(VerbosityType::Log, "Network: " + std::to_string(_actors.size()) + " network actor(s) removed with the end of the game");
}
