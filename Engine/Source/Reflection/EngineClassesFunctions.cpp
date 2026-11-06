#include "EngineClassHelpers.h"

using namespace Krampus;
using namespace Krampus::EngineClassHelpers;

namespace Krampus::EngineClassHelpers
{
	void RegisterFunctionClasses()
	{
	// ================= Functions the editor can call (event lists) =================
	{
		ClassInfo& _info = Describe("IGameObject", "");
		_info.AddFunction<IGameObject, bool>("SetActive", [](IGameObject& _object, const bool& _active) { _object.SetActive(_active); });
	}

	{
		ClassInfo& _info = Describe("Actor", "IGameObject");
		_info.AddFunction<Actor>("Destroy", [](Actor& _actor) { _actor.Destroy(); });
		_info.AddFunction<Actor, FVector2>("SetPosition", [](Actor& _actor, const FVector2& _position) { _actor.SetPosition(_position); });
		_info.AddFunction<Actor, float>("SetRotation", [](Actor& _actor, const float& _degrees) { _actor.SetRotation(Angle::FromDegrees(_degrees)); });
		_info.AddFunction<Actor, FVector2>("SetScale", [](Actor& _actor, const FVector2& _scale) { _actor.SetScale(_scale); });
	}

	{
		ClassInfo& _info = Describe("Widget", "IGameObject");
		_info.AddFunction<Widget>("Destroy", [](Widget& _widget) { _widget.Destroy(); });
		_info.AddFunction<Widget, FVector2>("SetPosition", [](Widget& _widget, const FVector2& _position) { _widget.SetPosition(_position); });
		_info.AddFunction<Widget, float>("SetRotation", [](Widget& _widget, const float& _degrees) { _widget.SetRotation(Angle::FromDegrees(_degrees)); });
		_info.AddFunction<Widget, FVector2>("SetScale", [](Widget& _widget, const FVector2& _scale) { _widget.SetScale(_scale); });
	}

	AddAnimationFunctions<AnimationComponent>(Describe("AnimationComponent", "Component"));
	AddAnimationFunctions<AnimationComponentSM>(Describe("AnimationComponentSM", "Component"));
	AddAnimationFunctions<AnimationWidgetComponent>(Describe("AnimationWidgetComponent", "WidgetComponent"));
	Describe("AnimationComponent", "Component").AddFunction<AnimationComponent, std::string>("Play", [](AnimationComponent& _c, const std::string& _name) { _c.SetCurrentAnimation(String(_name)); _c.StartAnimation(); });
	Describe("AnimationWidgetComponent", "WidgetComponent").AddFunction<AnimationWidgetComponent, std::string>("Play", [](AnimationWidgetComponent& _c, const std::string& _name) { _c.SetCurrentAnimation(String(_name)); _c.StartAnimation(); });

	Describe("LabelComponent", "Component").AddFunction<LabelComponent, std::string>("SetText", [](LabelComponent& _c, const std::string& _text) { _c.SetString(_text); });
	Describe("LabelWidgetComponent", "WidgetComponent").AddFunction<LabelWidgetComponent, std::string>("SetText", [](LabelWidgetComponent& _c, const std::string& _text) { _c.SetString(_text); });
	Describe("TextWidget", "Widget").AddFunction<TextWidget, std::string>("SetText", [](TextWidget& _w, const std::string& _text) { _w.SetText(String(_text)); });
	Describe("SliderWidget", "ControlWidget").AddFunction<SliderWidget, float>("SetValue", [](SliderWidget& _w, const float& _value) { _w.SetValue(_value, true); });
	Describe("ProgressBarWidget", "ControlWidget").AddFunction<ProgressBarWidget, float>("SetValue", [](ProgressBarWidget& _w, const float& _value) { _w.SetValue(_value); });
	Describe("CheckBoxWidget", "ControlWidget").AddFunction<CheckBoxWidget, bool>("SetChecked", [](CheckBoxWidget& _w, const bool& _checked) { _w.SetChecked(_checked, true); });
	{
		ClassInfo& _info = Describe("TextEntryWidget", "ControlWidget");
		_info.AddFunction<TextEntryWidget, std::string>("SetText", [](TextEntryWidget& _w, const std::string& _text) { _w.SetText(_text); });
		_info.AddFunction<TextEntryWidget>("Clear", [](TextEntryWidget& _w) { _w.SetText(""); });
		_info.AddFunction<TextEntryWidget>("Focus", [](TextEntryWidget& _w) { _w.SetFocused(true); });
	}
	Describe("DropdownWidget", "ControlWidget").AddFunction<DropdownWidget, int>("SetSelected", [](DropdownWidget& _w, const int& _index) { _w.SetSelected(_index, true); });
	Describe("ScrollBoxWidget", "PanelWidget").AddFunction<ScrollBoxWidget, float>("ScrollTo", [](ScrollBoxWidget& _w, const float& _scroll) { _w.scroll = _scroll; });

	{
		ClassInfo& _info = Describe("SpatialSoundComponent", "Component");
		_info.AddFunction<SpatialSoundComponent>("Play", [](SpatialSoundComponent& _c) { _c.Play(); });
		_info.AddFunction<SpatialSoundComponent>("Pause", [](SpatialSoundComponent& _c) { _c.Pause(); });
		_info.AddFunction<SpatialSoundComponent>("Stop", [](SpatialSoundComponent& _c) { _c.Stop(); });
	}

	Describe("CameraComponent", "Component").AddFunction<CameraComponent>("MakeCurrent", [](CameraComponent& _c) { _c.SetCurrent(); });

	{
		ClassInfo& _info = Describe("PhysicsComponent", "Component");
		_info.AddFunction<PhysicsComponent, FVector2>("SetLinearVelocity", [](PhysicsComponent& _c, const FVector2& _velocity) { _c.SetLinearVelocity(_velocity); });
		_info.AddFunction<PhysicsComponent, float>("SetAngularVelocity", [](PhysicsComponent& _c, const float& _velocity) { _c.SetAngularVelocity(Float(_velocity)); });
	}

	// "Game" is the target of the functions that are not about one object.
	{
		ClassInfo& _info = Describe("Game", "");
		_info.AddFunction<IGameObject, std::string>("OpenScene", [](IGameObject&, const std::string& _scene)
		{
			if (Engine* _engine = Reflection::CurrentEngine()) Scene::Open(*_engine, _scene);
		});
		_info.SetLastFunctionArgumentType(PropertyType::Scene);
		_info.AddFunction<IGameObject>("Quit", [](IGameObject&)
		{
			if (Engine* _engine = Reflection::CurrentEngine()) _engine->QuitGame();
		});
	}

	// ================= Network =================
	{
		ClassInfo& _info = Describe("NetworkSettings", "IGameObject");
		AddLive(_info, "port", &NetworkSettings::port, 1.f, 65535.f);
		AddLive(_info, "address", &NetworkSettings::address);
		AddLive(_info, "playerName", &NetworkSettings::playerName);
		AddLive(_info, "maxClients", &NetworkSettings::maxClients, 1.f, 64.f);
		AddLive(_info, "timeout", &NetworkSettings::timeout, 1.f, 120.f);
		AddLive(_info, "discoveryPort", &NetworkSettings::discoveryPort, 1.f, 65535.f);
		AddLive(_info, "lanVisible", &NetworkSettings::lanVisible);
		AddLive(_info, "steamEnabled", &NetworkSettings::steamEnabled);
		AddLive(_info, "steamAppId", &NetworkSettings::steamAppId, 0.f, 2000000000.f);
		AddLive(_info, "steamFriendsOnly", &NetworkSettings::steamFriendsOnly);
	}

	{
		ClassInfo& _info = Describe("NetworkSyncComponent", "Component");
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<NetworkSyncComponent>(); };
		_info.AddProperty<NetworkSyncComponent, Int>("networkId", &NetworkSyncComponent::networkId);
		_info.AddProperty<NetworkSyncComponent, Bool>("syncPosition", &NetworkSyncComponent::syncPosition);
		_info.AddProperty<NetworkSyncComponent, Bool>("syncRotation", &NetworkSyncComponent::syncRotation);
		_info.AddProperty<NetworkSyncComponent, Bool>("syncScale", &NetworkSyncComponent::syncScale);
		_info.AddProperty<NetworkSyncComponent, Float>("sendRate", &NetworkSyncComponent::sendRate, 1.f, 60.f);
		_info.AddProperty<NetworkSyncComponent, Bool>("smoothing", &NetworkSyncComponent::smoothing);
		_info.AddProperty<NetworkSyncComponent, Bool>("reliable", &NetworkSyncComponent::reliable);
		_info.AddProperty<NetworkSyncComponent, String>("properties", &NetworkSyncComponent::properties);

		// Calls a KFUNCTION of this actor (or of one of its components) on other machines: the argument is the function's name.
		_info.AddFunction<NetworkSyncComponent, std::string>("RpcAll", [](NetworkSyncComponent& _c, const std::string& _function) { _c.CallRPC(_function, NetTarget::All); });
		_info.AddFunction<NetworkSyncComponent, std::string>("RpcOthers", [](NetworkSyncComponent& _c, const std::string& _function) { _c.CallRPC(_function, NetTarget::Others); });
		_info.AddFunction<NetworkSyncComponent, std::string>("RpcHost", [](NetworkSyncComponent& _c, const std::string& _function) { _c.CallRPC(_function, NetTarget::Host); });
		_info.AddFunction<NetworkSyncComponent>("Despawn", [](NetworkSyncComponent& _c) { _c.GetWorld()->GetNetwork()->Despawn(_c.GetNetworkId()); });
	}

	{
		ClassInfo& _info = Describe("NetworkEventsComponent", "Component");
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<NetworkEventsComponent>(); };
		_info.AddProperty<NetworkEventsComponent, EventList>("onHosted", &NetworkEventsComponent::onHosted);
		_info.AddProperty<NetworkEventsComponent, EventList>("onConnected", &NetworkEventsComponent::onConnected);
		_info.AddProperty<NetworkEventsComponent, EventList>("onConnectionFailed", &NetworkEventsComponent::onConnectionFailed);
		_info.AddProperty<NetworkEventsComponent, EventList>("onDisconnected", &NetworkEventsComponent::onDisconnected);
		_info.AddProperty<NetworkEventsComponent, EventList>("onPlayerJoined", &NetworkEventsComponent::onPlayerJoined);
		_info.AddProperty<NetworkEventsComponent, EventList>("onPlayerLeft", &NetworkEventsComponent::onPlayerLeft);
		_info.AddProperty<NetworkEventsComponent, EventList>("onSharedChanged", &NetworkEventsComponent::onSharedChanged);
		_info.AddProperty<NetworkEventsComponent, EventList>("onLanGameFound", &NetworkEventsComponent::onLanGameFound);
		_info.AddAccessor<NetworkEventsComponent, std::string>("messages",
			[](NetworkEventsComponent& _c) { return InputComponent::EncodeBindings(_c.messages); },
			[](NetworkEventsComponent& _c, const std::string& _v) { _c.messages = InputComponent::DecodeBindings(_v); _c.ApplyMessages(); });
		_info.SetLastPropertyType(PropertyType::MessageBindings);
	}

	// Network functions of the "Game" target.
	{
		ClassInfo& _info = Describe("Game", "");
		const auto _net = []() { return Reflection::CurrentEngine()->GetNetwork(); };
		_info.AddFunction<IGameObject>("Host", [_net](IGameObject&) { _net()->Host(); });
		_info.AddFunction<IGameObject>("Join", [_net](IGameObject&) { _net()->Join(); });
		_info.AddFunction<IGameObject, std::string>("JoinAddress", [_net](IGameObject&, const std::string& _address) { _net()->Join(_address); });
		_info.AddFunction<IGameObject>("Disconnect", [_net](IGameObject&) { _net()->Disconnect(); });
		_info.AddFunction<IGameObject>("HostSteam", [_net](IGameObject&) { _net()->HostSteam(); });
		_info.AddFunction<IGameObject>("InviteSteamFriends", [_net](IGameObject&) { _net()->InviteSteamFriends(); });
		_info.AddFunction<IGameObject>("SearchSteam", [_net](IGameObject&) { _net()->SearchSteam(); });
		_info.AddFunction<IGameObject, std::string>("JoinSteamId", [_net](IGameObject&, const std::string& _id) { _net()->JoinSteam(_id); });
		_info.AddFunction<IGameObject>("JoinFirstSteamLobby", [_net](IGameObject&)
		{
			const std::vector<SteamLobby>& _lobbies = _net()->GetSteamLobbies();
			if (!_lobbies.empty()) _net()->JoinSteamLobby(_lobbies.front());
		});
		_info.AddFunction<IGameObject, std::string>("SendMessage", [_net](IGameObject&, const std::string& _name) { _net()->Send(_name); });
		_info.AddFunction<IGameObject, std::string>("SpawnNetworked", [_net](IGameObject&, const std::string& _className) { _net()->Spawn(_className); });
		_info.AddFunction<IGameObject>("SearchLan", [_net](IGameObject&) { _net()->SearchLan(); });
		_info.AddFunction<IGameObject>("JoinFirstLanGame", [_net](IGameObject&)
		{
			if (_net()->GetLanGames().empty()) LOG_WARNING("Network: no LAN game found yet (call SearchLan first)");
			else _net()->Join(_net()->GetLanGames().front());
		});
		_info.AddFunction<IGameObject, std::string>("SetPlayerName", [_net](IGameObject&, const std::string& _name) { _net()->SetPlayerName(_name); });
		_info.AddFunction<IGameObject, std::string>("AddSharedOne", [_net](IGameObject&, const std::string& _key) { _net()->AddShared(_key, 1.f); });
	}
	}
}
