#include "NetworkSyncComponent.h"
#include "Actors/Actor.h"
#include "Reflection/Reflection.h"

Krampus::NetworkSyncComponent::NetworkSyncComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(NetworkSyncComponent);
}

int Krampus::NetworkSyncComponent::IdFromName(const std::string& _name)
{
	// Below 0x40000000: the ids above are given to spawned actors.
	const int _id = static_cast<int>(std::hash<std::string>{}(_name) & 0x3FFFFFFF);
	return _id == 0 ? 1 : _id;
}

int Krampus::NetworkSyncComponent::GetNetworkId()
{
	if (static_cast<int>(networkId) != 0) return networkId;
	return IdFromName(owner->GetName().StdString());
}

bool Krampus::NetworkSyncComponent::IsLocalOwner()
{
	NetworkManager* _net = GetWorld()->GetNetwork();
	return !_net->IsOnline() || _net->GetLocalId() == ownerId;
}

void Krampus::NetworkSyncComponent::Register()
{
	if (registered) GetWorld()->GetNetwork()->Unregister(resolvedId, this);
	resolvedId = GetNetworkId();
	GetWorld()->GetNetwork()->Register(resolvedId, this);
	registered = true;
}

void Krampus::NetworkSyncComponent::AssignNetworkId(const int _networkId, const int _ownerId)
{
	networkId = Int(_networkId);
	ownerId = _ownerId;
	spawnedByNetwork = true;
	Register();		// now, not at the first Tick: a function call (RPC) may arrive for it in the same frame
}

void Krampus::NetworkSyncComponent::CallRPC(const std::string& _function, const NetTarget _target, const std::vector<PropertyValue>& _args)
{
	if (!registered) Register();
	GetWorld()->GetNetwork()->CallRPC(resolvedId, _function, _target, _args);
}

void Krampus::NetworkSyncComponent::Deconstruct()
{
	Component::Deconstruct();
	if (registered)
	{
		GetWorld()->GetNetwork()->Unregister(resolvedId, this);
		registered = false;
	}
}

void Krampus::NetworkSyncComponent::BeginDestroy()
{
	// The host destroyed an actor it spawned: the others must lose it too.
	if (spawnedByNetwork && registered && GetWorld()->GetNetwork()->IsHost())
		GetWorld()->GetNetwork()->ForgetSpawn(resolvedId);
	Component::BeginDestroy();
}

std::string Krampus::NetworkSyncComponent::ToString() const
{
	return name + " -> id " + std::to_string(resolvedId) + ", owner " + std::to_string(ownerId);
}

// ---- Properties to share

void Krampus::NetworkSyncComponent::ParseProperties()
{
	const std::string _text = properties.StdString();
	if (_text == parsedFor) return;
	parsedFor = _text;
	parsed.clear();

	size_t _start = 0;
	while (_start <= _text.size())
	{
		size_t _end = _text.find_first_of(";,", _start);
		if (_end == std::string::npos) _end = _text.size();

		std::string _entry = _text.substr(_start, _end - _start);
		_entry.erase(std::remove_if(_entry.begin(), _entry.end(), [](const unsigned char _c) { return std::isspace(_c); }), _entry.end());
		if (const size_t _dot = _entry.find('.'); _dot != std::string::npos && _dot > 0 && _dot + 1 < _entry.size())
			parsed.emplace_back(_entry.substr(0, _dot), _entry.substr(_dot + 1));

		if (_end >= _text.size()) break;
		_start = _end + 1;
	}
}

Krampus::IGameObject* Krampus::NetworkSyncComponent::FindObject(const std::string& _className)
{
	if (ClassNameOf(*owner) == _className) return owner;
	for (const std::unique_ptr<Component>& _component : owner->GetComponents())
		if (ClassNameOf(*_component) == _className) return _component.get();
	return nullptr;
}

// ---- Sending and receiving

void Krampus::NetworkSyncComponent::SendState()
{
	NetworkManager* _net = GetWorld()->GetNetwork();
	ParseProperties();

	const FVector2 _position = owner->GetPosition();
	const float _rotation = owner->GetRotation().ToDegrees();
	const FVector2 _scale = owner->GetScale();

	const bool _moved = !sentOnce || _position.x != lastSent.position.x || _position.y != lastSent.position.y || _rotation != lastSent.rotation
		|| _scale.x != lastSent.scale.x || _scale.y != lastSent.scale.y;
	if (!_moved && parsed.empty() && idle < 1.f) return;	// nothing new (a state is still sent every second, in case one was lost)
	idle = 0.f;
	sentOnce = true;
	lastSent = Target{ _position, _rotation, _scale };

	std::vector<PropertyValue> _args = { resolvedId, _position.x, _position.y, _rotation, _scale.x, _scale.y };
	for (const auto& [_className, _property] : parsed)
	{
		PropertyValue _value = 0;
		if (IGameObject* _object = FindObject(_className))
			if (const ClassInfo* _class = ClassRegistry::Find(ClassNameOf(*_object)))
				for (const PropertyInfo* _info : _class->GetProperties())
					if (_info->name == _property) { _value = _info->get(_object); break; }
		_args.push_back(_value);
	}

	_net->Send("__sync", _args, NetTarget::Others, static_cast<bool>(reliable) ? NetChannel::Reliable : NetChannel::Unreliable);
}

void Krampus::NetworkSyncComponent::ReceiveState(const NetMessage& _message)
{
	if (IsLocalOwner()) return;		// the owner is the source of truth

	target.position = FVector2(_message.Float(1), _message.Float(2));
	target.rotation = _message.Float(3);
	target.scale = FVector2(_message.Float(4), _message.Float(5));
	if (!hasTarget || !static_cast<bool>(smoothing))
	{
		if (syncPosition) owner->SetPosition(target.position);
		if (syncRotation) owner->SetRotation(Angle::FromDegrees(target.rotation));
		if (syncScale) owner->SetScale(target.scale);
	}
	hasTarget = true;

	ParseProperties();
	for (size_t _i = 0; _i < parsed.size() && 6 + _i < _message.args.size(); ++_i)
		if (IGameObject* _object = FindObject(parsed[_i].first))
			if (const ClassInfo* _class = ClassRegistry::Find(ClassNameOf(*_object)))
				for (const PropertyInfo* _info : _class->GetProperties())
					if (_info->name == parsed[_i].second && !_info->readOnly)
					{
						try { _info->set(_object, _message.args[6 + _i]); }
						catch (const std::bad_variant_access&) { }
						break;
					}
}

void Krampus::NetworkSyncComponent::Tick(const Float& _deltaTime)
{
	Component::Tick(_deltaTime);

	if (!registered) Register();	// the actor has its final name by now

	NetworkManager* _net = GetWorld()->GetNetwork();
	if (!_net->IsOnline()) return;

	const float _dt = _deltaTime;
	if (IsLocalOwner())
	{
		timer += _dt;
		idle += _dt;
		if (timer >= 1.f / std::max(1.f, static_cast<float>(sendRate)))
		{
			timer = 0.f;
			SendState();
		}
	}
	else if (hasTarget && smoothing)
	{
		const float _t = std::min(1.f, _dt * 15.f);
		if (syncPosition)
		{
			const FVector2 _current = owner->GetPosition();
			owner->SetPosition(FVector2(_current.x + (target.position.x - _current.x) * _t, _current.y + (target.position.y - _current.y) * _t));
		}
		if (syncRotation)
		{
			float _difference = std::fmod(target.rotation - owner->GetRotation().ToDegrees(), 360.f);
			if (_difference > 180.f) _difference -= 360.f;
			if (_difference < -180.f) _difference += 360.f;
			owner->SetRotation(Angle::FromDegrees(owner->GetRotation().ToDegrees() + _difference * _t));
		}
		if (syncScale)
		{
			const FVector2 _current = owner->GetScale();
			owner->SetScale(FVector2(_current.x + (target.scale.x - _current.x) * _t, _current.y + (target.scale.y - _current.y) * _t));
		}
	}
}
