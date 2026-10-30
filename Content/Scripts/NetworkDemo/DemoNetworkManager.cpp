#include "DemoNetworkManager.h"
#include "DemoNetworkPlayer.h"
#include "Core/Engine.h"
#include "Actors/EmptyActor.h"
#include "Components/SpriteComponent.h"
#include "UI/Widgets/Widget.h"
#include "UI/Widgets/WidgetComponents/LabelWidgetComponent.h"
#include <random>

namespace Krampus
{

namespace
{
	std::mt19937& Random()
	{
		static std::mt19937 _random(std::random_device{}());
		return _random;
	}

	template <typename T>
	const T& Pick(const std::vector<T>& _list)
	{
		return _list[std::uniform_int_distribution<size_t>(0, _list.size() - 1)(Random())];
	}

	std::string Describe(const PropertyValue& _value)
	{
		return std::visit([](const auto& _v) -> std::string
		{
			using V = std::decay_t<decltype(_v)>;
			if constexpr (std::is_same_v<V, bool>)				return _v ? "true" : "false";
			else if constexpr (std::is_same_v<V, int>)			return std::to_string(_v);
			else if constexpr (std::is_same_v<V, float>)		return std::format("{:.2f}", _v);
			else if constexpr (std::is_same_v<V, std::string>)	return _v;
			else if constexpr (std::is_same_v<V, FVector2>)		return std::format("({:.0f}, {:.0f})", _v.x, _v.y);
			else												return "a color";
		}, _value);
	}

	std::string KiloBytes(const float _bytes)
	{
		return std::format("{:.1f} KB/s", _bytes / 1024.f);
	}

	LabelWidgetComponent* MakeText(Level* _level, const std::string& _name, const FVector2& _position, const unsigned int _size, const Color& _color)
	{
		Widget* _widget = _level->SpawnWidget<Widget>();
		_widget->SetName(String(_name));
		LabelWidgetComponent* _text = _widget->AddComponent<LabelWidgetComponent>(TextData("", _size));
		_text->SetFillColor(_color);
		_text->SetOutlineColor(Color::Black());
		_text->SetOutlineThickness(2.f);
		_widget->SetPosition(_position);
		return _text;
	}
}

DemoNetworkManager::DemoNetworkManager(Level* _level)
	: Actor(_level)
{
	name = "NetworkDemo";
}

void DemoNetworkManager::Tick(const Float& _deltaTime)
{
	Actor::Tick(_deltaTime);

	// Tick only runs while the game plays (not in the editor before Play): the demo builds its things here, so they are never saved
	// into the scene.
	if (!started) Start();

	HandleKeys();

	refresh -= static_cast<float>(_deltaTime);
	if (refresh <= 0.f)
	{
		refresh = 0.15f;
		RefreshTexts();
	}
}

void DemoNetworkManager::BeginDestroy()
{
	listeners.Clear();
	chatHandle.Disconnect();
	Actor::BeginDestroy();
}

void DemoNetworkManager::Start()
{
	started = true;
	BuildScene();

	NetworkManager* _net = GetWorld()->GetNetwork();

	// What the NetworkManager tells, written in the event log.
	listeners += _net->onHosted.AddListener([this]()
	{
		AddLog("You host the game. Other windows: J, or L then 1");
		SpawnMyPlayer();
	});
	listeners += _net->onConnected.AddListener([this, _net]()
	{
		AddLog("Connected to " + _net->GetPlayerName(0) + "'s game");
		SpawnMyPlayer();
	});
	listeners += _net->onConnectionFailed.AddListener([this, _net]() { AddLog("Could not join: " + _net->GetDisconnectReason()); });
	listeners += _net->onDisconnected.AddListener([this, _net]() { AddLog("Game over: " + _net->GetDisconnectReason()); });
	listeners += _net->onClientJoined.AddListener([this, _net](int _id) { AddLog(_net->GetPlayerName(_id) + " is in the game"); });
	listeners += _net->onClientLeft.AddListener([this](int _id) { AddLog("Player " + std::to_string(_id) + " left"); });
	listeners += _net->onSharedChanged.AddListener([this](const std::string& _key, const PropertyValue& _value)
	{
		AddLog("Shared \"" + _key + "\" = " + Describe(_value));
	});
	listeners += _net->onLanGameFound.AddListener([this, _net](const NetLanGame& _game)
	{
		AddLog("LAN: " + _game.gameName + " by " + _game.hostName + ": press " + std::to_string(_net->GetLanGames().size()) + " to join");
	});

	// A message of the game itself: "Chat", with the text as its one value.
	chatHandle = _net->Subscribe("Chat", [this, _net](const NetMessage& _message)
	{
		AddLog(_net->GetPlayerName(_message.sender) + ": " + _message.String(0));
	});

	AddLog("Welcome! H to host, J to join, L to search the LAN");
	if (_net->IsOnline()) SpawnMyPlayer();	// started with --host / --join before the scene was there
	RefreshTexts();
}

void DemoNetworkManager::BuildScene()
{
	// The area the players move in, drawn as a frame.
	const FRect _arena = DemoArena(GetLevel());
	EmptyActor* _frame = GetLevel()->SpawnActor<EmptyActor>();
	_frame->SetName("DemoArena");
	SpriteComponent* _shape = _frame->AddComponent<SpriteComponent>(RectangleShapeData(_arena.GetSize()));
	_frame->SetPosition(FVector2(1920, 1080) / 2.0f);
	_shape->GetShapeObject()->GetShape()->ClearTexture();
	_shape->GetShapeObject()->GetShape()->SetFillColor(Color(255, 255, 255, 10));
	_shape->GetShapeObject()->GetShape()->SetOutlineColor(Color(110, 120, 150));
	_shape->GetShapeObject()->GetShape()->SetOutlineThickness(3.f);

	// The texts, in the HUD (1920 x 1080 whatever the window).
	statusText	= MakeText(GetLevel(), "DemoStatus", FVector2(400.f, 240.f), 24u, Color::White());
	logText		= MakeText(GetLevel(), "DemoLog", FVector2(1180.f, 240.f), 22u, Color(200, 220, 255));
	helpText	= MakeText(GetLevel(), "DemoHelp", FVector2(400.f, 990.f), 21u, Color(170, 175, 190));
	helpText->SetString(String(
		"H host   J join   L search LAN, then 1-9 join   X leave   |   ZQSD / WASD / arrows move   Space flash (RPC)   F / G size (synced property)\n"
		"E +1 shared score   R score to 0   C chat   N random name   K kick the last player (host)"));
}

bool DemoNetworkManager::Pressed(const sf::Keyboard::Key _key)
{
	// Two windows on one PC share the keyboard: only the one with the focus reacts.
	const bool _down = sf::Keyboard::isKeyPressed(_key);
	bool& _was = held[_key];
	const bool _pressed = _down && !_was;
	_was = _down;
	return _pressed && GetWorld()->GetWindowRef().HasFocus();
}

void DemoNetworkManager::HandleKeys()
{
	using Key = sf::Keyboard::Key;
	NetworkManager* _net = GetWorld()->GetNetwork();

	if (Pressed(Key::H))
	{
		if (_net->GetRole() == NetRole::Offline) _net->Host();
		else AddLog("Already in a game (X to leave it first)");
	}
	if (Pressed(Key::J))
	{
		if (_net->GetRole() == NetRole::Offline) _net->Join();
		else AddLog("Already in a game (X to leave it first)");
	}
	if (Pressed(Key::L)) _net->SearchLan(3.f);
	if (Pressed(Key::X)) _net->Disconnect();

	// 1..9 (top row or keypad): join the game found by the LAN search.
	static constexpr Key ROW[] = { Key::Num1, Key::Num2, Key::Num3, Key::Num4, Key::Num5, Key::Num6, Key::Num7, Key::Num8, Key::Num9 };
	static constexpr Key PAD[] = { Key::Numpad1, Key::Numpad2, Key::Numpad3, Key::Numpad4, Key::Numpad5, Key::Numpad6, Key::Numpad7, Key::Numpad8, Key::Numpad9 };
	for (size_t _i = 0; _i < std::size(ROW); ++_i)
	{
		const bool _row = Pressed(ROW[_i]);
		const bool _pad = Pressed(PAD[_i]);
		if (!_row && !_pad) continue;
		if (_i >= _net->GetLanGames().size()) AddLog("No LAN game " + std::to_string(_i + 1) + " (L to search)");
		else if (_net->GetRole() != NetRole::Offline) AddLog("Already in a game (X to leave it first)");
		else
		{
			const NetLanGame _game = _net->GetLanGames()[_i];
			_net->Join(_game);
		}
	}

	// Shared values: the host keeps them, every machine sees the same.
	if (Pressed(Key::E)) _net->AddShared("score", 1.f);
	if (Pressed(Key::R)) _net->SetShared("score", 0);

	// A message with a value, to everybody.
	if (Pressed(Key::C))
	{
		static const std::vector<std::string> PHRASES = { "Hello!", "Nice one!", "Over here!", "GG", "Watch this!", "Who wants to race?", "Ho ho ho" };
		_net->Send("Chat", NetworkManager::Args(Pick(PHRASES)));
	}

	// A new name: the others see it at once.
	if (Pressed(Key::N))
	{
		static const std::vector<std::string> NAMES = { "Krampus", "Elf", "Yeti", "Reindeer", "Snowman", "Gnome", "Imp", "Goblin" };
		_net->SetPlayerName(Pick(NAMES) + std::to_string(std::uniform_int_distribution<int>(10, 99)(Random())));
		AddLog("You are now " + _net->playerName);
	}

	// The host sends the last player away.
	if (Pressed(Key::K))
	{
		if (!_net->IsHost()) AddLog("Only the host can kick");
		else if (const std::vector<int> _others = _net->GetOtherPlayers(); _others.empty()) AddLog("Nobody to kick");
		else _net->Kick(_others.back(), "demo: K pressed by the host");
	}
}

void DemoNetworkManager::SpawnMyPlayer()
{
	NetworkManager* _net = GetWorld()->GetNetwork();

	// One player per machine: not again after a reconnection that kept it.
	for (DemoNetworkPlayer* _player : GetLevel()->GetActorManager().GetAllActorOfClass<DemoNetworkPlayer>())
		if (const NetworkSyncComponent* _sync = _player->GetComponent<NetworkSyncComponent>(); _sync && _sync->GetOwnerId() == _net->GetLocalId() && !_player->IsPendingDestroy())
			return;

	const FRect _arena = DemoArena(GetLevel());
	const FVector2 _position(
		std::uniform_real_distribution<float>(_arena.GetPosition().x + 60.f, _arena.GetPosition().x + _arena.GetSize().x - 60.f)(Random()),
		std::uniform_real_distribution<float>(_arena.GetPosition().y + 60.f, _arena.GetPosition().y + _arena.GetSize().y - 60.f)(Random()));
	_net->Spawn(playerClass.StdString(), _position);
}

void DemoNetworkManager::AddLog(const std::string& _line)
{
	eventLog.push_back(_line);
	while (eventLog.size() > static_cast<size_t>(std::max(1, static_cast<int>(chatLines)))) eventLog.pop_front();
	if (logText) RefreshTexts();
}

void DemoNetworkManager::RefreshTexts()
{
	if (!statusText || !logText) return;
	NetworkManager* _net = GetWorld()->GetNetwork();

	std::string _status = "KRAMPUS ENGINE - NETWORK DEMO\n" + _net->GetStatusText() + "\n";
	_status += "You: " + _net->playerName + (_net->IsOnline() ? " (player " + std::to_string(_net->GetLocalId()) + ")" : std::string()) + "\n\n";

	if (_net->IsOnline())
	{
		_status += "Players:\n";
		for (const NetPlayer& _player : _net->GetPlayers())
			_status += std::format("  [{}] {}{}{}\n", _player.id, _player.name, _player.isHost ? "  host" : std::format("  {:.0f} ms", _player.pingMs), _player.isLocal ? "  (you)" : "");
	}
	else
		_status += "Players: not in a game\n";

	_status += "\nShared score: " + Describe(_net->GetShared("score", PropertyValue(0))) + "   (E +1, R reset)\n";

	_status += "\nLAN games" + std::string(_net->IsSearchingLan() ? " (searching...)" : "") + ":\n";
	if (_net->GetLanGames().empty()) _status += "  none (L to search)\n";
	for (size_t _i = 0; _i < _net->GetLanGames().size() && _i < 9; ++_i)
	{
		const NetLanGame& _game = _net->GetLanGames()[_i];
		_status += std::format("  {}) {} by {}  {}:{}  {}/{}\n", _i + 1, _game.gameName, _game.hostName, _game.address, _game.port, _game.players, _game.maxPlayers);
	}

	const NetStats& _stats = _net->GetStats();
	_status += "\nTraffic: up " + KiloBytes(_stats.sentPerSecond) + ", down " + KiloBytes(_stats.receivedPerSecond);
	statusText->SetString(String(_status));

	std::string _events = "Events\n";
	for (const std::string& _line : eventLog)
		_events += "  " + _line + "\n";
	logText->SetString(String(_events));
}

}
