#include "BlackoutMenus.h"
#include "BlackoutGameMode.h"
#include "BlackoutHero.h"
#include "BlackoutChat.h"
#include "BlackoutKeys.h"
#include "Layers/PauseLayer.h"
#include "Core/Engine.h"
#include "Core/Settings.h"
#include "Scene/Scene.h"
#include <sstream>

namespace Krampus
{

// ---- BlackoutButton

BlackoutButton::BlackoutButton(Level* _level)
	: ButtonWidget(_level, RectangleShapeData(FVector2(460.f, 72.f), "Blackout/Solid"))
{
	name = "BlackoutButton";
	imageComponent->GetShapeObject()->GetShape()->SetOutlineThickness(2.f);
	imageComponent->GetShapeObject()->GetShape()->SetOutlineColor(Color(110, 130, 170, 200));
	label = CreateComponent<LabelWidgetComponent>(TextData("Bouton", 32u));
	label->SetOutlineColor(Color::Black());
	label->SetOutlineThickness(2.f);
}

void BlackoutButton::Construct()
{
	ButtonWidget::Construct();
	label->SetZOrder(static_cast<std::uint8_t>(60));	// over its picture (50): a label is at 0 by default

	hoverHandle = onHover.AddListener([this]() { hovered = true; Paint(); });
	unhoverHandle = onUnhover.AddListener([this]() { hovered = false; Paint(); });
	pressHandle = onPress.AddListener([this]()
		{
			if (!IsActive() || command.empty()) return;		// hidden: the click goes nowhere
			BlackoutMenuController::Dispatch(GetLevel(), command);
		});
	Paint();
}

void BlackoutButton::Tick(const Float& _deltaTime)
{
	ButtonWidget::Tick(_deltaTime);
	if (text != shownText)
	{
		shownText = text;
		label->SetString(text);
	}
	Paint();
}

void BlackoutButton::Paint()
{
	imageComponent->GetShapeObject()->GetShape()->SetFillColor(hovered ? hoverColor : (selected ? selectedColor : idleColor));
}

// ---- BlackoutMenuController

bool BlackoutMenuController::steamLobby = false;

BlackoutMenuController::BlackoutMenuController(Level* _level)
	: Actor(_level)
{
	name = "MenuController";
}

void BlackoutMenuController::Dispatch(Level* _level, const std::string& _command)
{
	// In a game: the buttons of the HUD (the shop) belong to the game mode.
	if (BlackoutGameMode* _mode = BlackoutGameMode::Of(_level))
	{
		_mode->Command(_command);
		return;
	}
	const std::vector<BlackoutMenuController*> _controllers = _level->GetActorManager().GetAllActorOfClass<BlackoutMenuController>();
	if (_controllers.empty()) LOG_WARNING("Blackout: no BlackoutMenuController in the scene for the command \"" + _command + "\"");
	else _controllers.front()->Command(_command);
}

Widget* BlackoutMenuController::Find(const std::string& _name) const
{
	for (const std::unique_ptr<Widget>& _widget : level->GetHUD().GetWidgets())
		if (_widget->GetName() == _name && !_widget->IsPendingDestroy()) return _widget.get();
	return nullptr;
}

void BlackoutMenuController::SetText(const std::string& _widget, const std::string& _text, const Color& _color)
{
	if (TextWidget* _found = Cast<TextWidget>(Find(_widget)))
	{
		if (_found->GetLabel()->GetString() != _text) _found->SetText(String(_text));
		_found->GetLabel()->SetFillColor(_color);
	}
}

void BlackoutMenuController::SetInfo(const std::string& _text, const Color& _color)
{
	SetText("InfoText", _text, _color);
}

void BlackoutMenuController::ShowList(const std::string& _names, const bool& _show, const std::string& _skip)
{
	std::stringstream _stream(_names);
	std::string _name;
	const std::string _skipped = ";" + _skip + ";";
	while (std::getline(_stream, _name, ';'))
		if (Widget* _widget = Find(_name))
		{
			const bool _visible = _show && _skipped.find(";" + _name + ";") == std::string::npos;
			if (_widget->IsActive() != _visible) _widget->SetActive(_visible);
		}
}

void BlackoutMenuController::OpenScene(const std::string& _scene)
{
	leaving = true;
	Scene::Open(*GetWorld(), _scene);
}

void BlackoutMenuController::ApplyName()
{
	if (TextEntryWidget* _entry = Cast<TextEntryWidget>(Find("NameEntry")))
		if (!_entry->GetText().empty()) GetWorld()->GetNetwork()->SetPlayerName(_entry->GetText());
}

void BlackoutMenuController::BeginPlay()
{
	Actor::BeginPlay();
	NetworkManager* _net = GetWorld()->GetNetwork();
	// No pause in Blackout (Escape belongs to its menus).
	if (PauseLayer* _pause = GetWorld()->GetLayerStack().Find<PauseLayer>()) _pause->SetEnabled(false);

	// The main menu is outside any game.
	if (!isLobby)
	{
		// Out of every session, even half made (connecting, a Steam lobby on its way): friends can no longer join a game left here.
		_net->Disconnect();

		// The keys menu, and its button when the scene has no "keys" button: in the place of the lowest button (Quitter), which goes
		// one step down.
		keysMenu = GetLevel()->SpawnActor<BlackoutKeysMenu>();
		BlackoutButton* _lowest = nullptr;
		bool _hasKeys = false;
		float _step = 100.f;
		for (const std::unique_ptr<Widget>& _widget : level->GetHUD().GetWidgets())
			if (BlackoutButton* _button = Cast<BlackoutButton>(_widget.get()))
			{
				mainButtons.push_back(_button);
				_hasKeys = _hasKeys || _button->command == "keys";
				if (_lowest && std::abs(_button->GetPosition().y - _lowest->GetPosition().y) > 1.f)
					_step = std::min(_step, std::abs(_button->GetPosition().y - _lowest->GetPosition().y));
				if (!_lowest || _button->GetPosition().y > _lowest->GetPosition().y) _lowest = _button;
			}
		if (!_hasKeys && _lowest)
		{
			BlackoutButton* _keys = GetLevel()->SpawnWidget<BlackoutButton>();
			_keys->SetName("KeysButton");
			_keys->SetPosition(_lowest->GetPosition());
			_keys->SetScale(_lowest->GetScale());
			_lowest->SetPosition(_lowest->GetPosition() + FVector2(0.f, std::max(80.f, _step)));
			_keys->text = "Touches";
			_keys->command = "keys";
			mainButtons.push_back(_keys);
		}

		// Steam now (Network settings, steamEnabled): an invitation accepted in Steam reaches the game only once Steam is started
		// (Tick then opens the lobby). Off: Steam starts when the Steam lobby opens.
		if (GetWorld()->GetSettings().network.steamEnabled) _net->InitSteam();
		BuildModeChoice();
		return;
	}

	// The lobby of a game that goes on (back from a match, an invitation accepted): the kind of the game, not the last choice.
	if (_net->GetRole() != NetRole::Offline) steamLobby = _net->IsSteamMode();
	if (steamLobby)
	{
		steamReady = _net->InitSteam();
		if (steamReady && _net->playerName == "Player") _net->playerName = _net->GetSteamName();
		if (BlackoutButton* _host = Cast<BlackoutButton>(Find("HostButton"))) _host->text = "Creer un salon Steam";
		BuildFriends();
	}

	if (TextEntryWidget* _name = Cast<TextEntryWidget>(Find("NameEntry")))
		if (_name->GetText().empty()) _name->SetText(_net->playerName);
	if (TextEntryWidget* _address = Cast<TextEntryWidget>(Find("AddressEntry")))
		if (_address->GetText().empty()) _address->SetText(GetWorld()->GetSettings().network.address);

	connectedHandle = _net->onConnected.AddListener([this]() { SetInfo("Connecte : choisis ton equipe", Color(140, 230, 150)); });
	failedHandle = _net->onConnectionFailed.AddListener([this]()
		{
			SetInfo("Connexion impossible : " + GetWorld()->GetNetwork()->GetDisconnectReason(), Color(255, 120, 100));
		});
	disconnectedHandle = _net->onDisconnected.AddListener([this]()
		{
			SetInfo("Deconnecte : " + GetWorld()->GetNetwork()->GetDisconnectReason(), Color(255, 170, 100));
		});
	lanHandle = _net->onLanGameFound.AddListener([this](const NetLanGame& _game)
		{
			if (TextEntryWidget* _address = Cast<TextEntryWidget>(Find("AddressEntry"))) _address->SetText(_game.address);
			SetInfo("Partie de " + _game.hostName + " trouvee (" + _game.address + ") : clique Rejoindre", Color(140, 230, 150));
		});
	// The host puts a newcomer in the team that has the fewest players.
	joinedHandle = _net->onClientJoined.AddListener([this](int _id)
		{
			NetworkManager* _network = GetWorld()->GetNetwork();
			if (!_network->IsHost()) return;
			int _count[2] = { 0, 0 };
			for (const NetPlayer& _player : _network->GetPlayers())
				if (_player.id != _id) ++_count[BlackoutGameMode::TeamOf(_network, _player.id)];
			_network->SetShared("team_" + std::to_string(_id), _count[1] < _count[0] ? 1 : 0);
		});
	// The host starts the game: everybody opens the scene.
	subscriptions.push_back(_net->Subscribe("BO_Start", [this](const NetMessage& _message) { OpenScene(_message.String(0)); }));

	// The chat of the lobby (bottom left).
	chat = GetLevel()->SpawnActor<BlackoutChat>();
	chat->Build(FVector2(40.f, 880.f), 560.f);

	if (_net->IsOnline()) SetInfo("De retour au salon");
	else if (!steamLobby) SetInfo("Heberge une partie, ou tape l'adresse de l'hote et rejoins-la");
	else if (steamReady) SetInfo("Cree un salon Steam et invite tes amis, ou rejoins la partie d'un ami (liste a droite)", Color(140, 230, 150));
	else SetInfo("Steam indisponible : " + _net->GetSteamError(), Color(255, 120, 100));
}

void BlackoutMenuController::Command(const std::string& _command)
{
	if (!leaving) pending.push_back(_command);
}

void BlackoutMenuController::RunCommand(const std::string& _command)
{
	if (leaving) return;
	NetworkManager* _net = GetWorld()->GetNetwork();
	LOG(VerbosityType::Log, "Blackout menu: " + _command);

	if (_command == "multi" && !modeButtons.empty()) ShowModeChoice(true);
	else if (_command == "multiback") ShowModeChoice(false);
	else if (_command == "lan" || _command == "steam")
	{
		steamLobby = _command == "steam";
		OpenScene(lobbyScene);
	}
	else if (_command == "steamoverlay")
	{
		_net->InviteSteamFriends();
		SetInfo("Overlay Steam ouvert (sinon : bouton Inviter de la liste)", Color(140, 230, 150));
	}
	else if (_command.starts_with("steamfriend"))
	{
		int _index = -1;
		std::istringstream(_command.substr(11)) >> _index;
		if (_index < 0 || _index >= static_cast<int>(friends.size())) return;
		const SteamFriend _friend = friends[static_cast<size_t>(_index)];
		if (_net->IsHost())
		{
			if (_net->InviteSteamFriend(_friend.id))
			{
				invited[_friend.id] = 5.f;
				SetInfo("Invitation envoyee a " + _friend.name + " : il la recoit dans Steam", Color(140, 230, 150));
			}
			else SetInfo("Invitation impossible : " + _net->GetSteamError(), Color(255, 120, 100));
		}
		else if (_net->GetRole() == NetRole::Offline && _friend.lobby != 0)
		{
			ApplyName();
			_net->JoinSteamLobby(_friend.lobby);
			SetInfo("Connexion a la partie de " + _friend.name + "...");
		}
	}
	else if (_command.starts_with("key"))		// "keys" opens the keys menu, its own buttons are "key<n>", "keysdefault", "keysback"
	{
		if (!keysMenu) keysMenu = GetLevel()->SpawnActor<BlackoutKeysMenu>();
		keysMenu->Command(_command);
	}
	else if (_command == "solo")
	{
		if (_net->IsOnline() || _net->IsClient()) _net->Disconnect();
		OpenScene(gameScene);
	}
	else if (_command == "multi") OpenScene(lobbyScene);
	else if (_command == "quit") GetWorld()->QuitGame();
	else if (_command == "back")
	{
		if (_net->IsOnline() || _net->IsClient()) _net->Disconnect();
		OpenScene(menuScene);
	}
	else if (_command == "host" && steamLobby)
	{
		ApplyName();
		if (_net->HostSteam())
		{
			_net->SetShared("team_0", 0);
			_net->SetShared("score_0", 0);
			_net->SetShared("score_1", 0);
			friendsTimer = 0.f;
			SetInfo("Salon Steam ouvert : invite tes amis (liste a droite)", Color(140, 230, 150));
		}
		else SetInfo("Impossible de creer le salon Steam : " + _net->GetDisconnectReason(), Color(255, 120, 100));
	}
	else if (_command == "host")
	{
		ApplyName();
		if (_net->Host())
		{
			_net->SetShared("team_0", 0);
			_net->SetShared("score_0", 0);
			_net->SetShared("score_1", 0);
			SetInfo("Salon ouvert : tes amis te rejoignent avec ton adresse", Color(140, 230, 150));
		}
		else SetInfo("Impossible d'heberger : le port " + std::to_string(GetWorld()->GetSettings().network.port) + " est deja pris", Color(255, 120, 100));
	}
	else if (_command == "join")
	{
		ApplyName();
		std::string _address = GetWorld()->GetSettings().network.address;
		if (TextEntryWidget* _entry = Cast<TextEntryWidget>(Find("AddressEntry")))
			if (!_entry->GetText().empty()) _address = _entry->GetText();
		_net->Join(_address);
		SetInfo("Connexion a " + _address + "...");
	}
	else if (_command == "search")
	{
		_net->SearchLan(3.f);
		SetInfo("Recherche de parties sur le reseau local...");
	}
	else if (_command == "alpha" || _command == "bravo")
	{
		if (_net->IsOnline()) _net->SetShared("team_" + std::to_string(_net->GetLocalId()), _command == "alpha" ? 0 : 1);
	}
	else if (_command == "start")
	{
		if (!_net->IsHost()) return;
		int _count[2] = { 0, 0 };
		for (const NetPlayer& _player : _net->GetPlayers()) ++_count[BlackoutGameMode::TeamOf(_net, _player.id)];
		if (_count[0] == 0 || _count[1] == 0)
		{
			SetInfo("Il faut au moins un joueur dans chaque equipe", Color(255, 170, 100));
			return;
		}
		_net->SetShared("score_0", 0);
		_net->SetShared("score_1", 0);
		// Round 1. Everybody starts with a pistol and the credits of the first round (the shop opens at the start of each round).
		_net->SetShared("round", 1);
		for (const NetPlayer& _player : _net->GetPlayers())
		{
			_net->SetShared("credits_" + std::to_string(_player.id), BlackoutGameMode::START_CREDITS);
			_net->SetShared("primary_" + std::to_string(_player.id), -1);
			_net->SetShared("sidearm_" + std::to_string(_player.id), BlackoutWeapon::PISTOL);
			_net->SetShared("shield_" + std::to_string(_player.id), 0);
		}
		_net->Send("BO_Start", NetworkManager::Args(gameScene), NetTarget::All);
	}
	else if (_command == "leave")
	{
		_net->Disconnect();
		SetInfo("Salon quitte");
	}
	else LOG_WARNING("Blackout menu: unknown command \"" + _command + "\"");
}

void BlackoutMenuController::Tick(const Float& _deltaTime)
{
	Actor::Tick(_deltaTime);
	if (leaving) return;
	const std::vector<std::string> _commands = std::move(pending);
	pending.clear();
	for (const std::string& _command : _commands) RunCommand(_command);
	if (leaving) return;
	NetworkManager* _net = GetWorld()->GetNetwork();

	if (isLobby)
	{
		UpdateLobby();
		UpdateFriends(_deltaTime);
	}
	else
	{
		// An invitation accepted in Steam (or the game of a friend joined from Steam): to the lobby.
		if (_net->GetRole() != NetRole::Offline)
		{
			steamLobby = _net->IsSteamMode();
			OpenScene(lobbyScene);
			return;
		}
		const bool _escape = GetWorld()->GetWindowRef().HasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Escape);
		if (_escape && !escapeHeld && choosingMode) ShowModeChoice(false);
		escapeHeld = _escape;
	}

	// The chosen team is shown in green.
	const int _team = BlackoutGameMode::TeamOf(_net, _net->GetLocalId());
	if (BlackoutButton* _alpha = Cast<BlackoutButton>(Find("AlphaButton"))) _alpha->SetSelected(_net->IsOnline() && _team == 0);
	if (BlackoutButton* _bravo = Cast<BlackoutButton>(Find("BravoButton"))) _bravo->SetSelected(_net->IsOnline() && _team == 1);
}

void BlackoutMenuController::UpdateLobby()
{
	NetworkManager* _net = GetWorld()->GetNetwork();
	const bool _online = _net->IsOnline();
	const bool _steam = _net->GetRole() != NetRole::Offline ? _net->IsSteamMode() : steamLobby;
	ShowList(offlineWidgets, !_online && !_net->IsClient(), _steam ? lanWidgets : std::string());
	ShowList(onlineWidgets, _online);
	if (Widget* _start = Find("StartButton")) if (_start->IsActive() && !_net->IsHost()) _start->SetActive(false);

	// Where we are.
	std::string _status;
	switch (_net->GetRole())
	{
	case NetRole::Offline:
		if (!_steam) _status = "Hors ligne  -  reseau local (LAN)";
		else _status = steamReady ? "Steam  -  " + _net->GetSteamName() : std::string("Steam indisponible (lance Steam puis reviens)");
		break;
	case NetRole::Connecting:	_status = _steam ? "Connexion via Steam..." : "Connexion..."; break;
	case NetRole::Host:
	{
		if (_steam)
		{
			_status = "Tu heberges via Steam  -  invite tes amis";
			break;
		}
		const std::optional<sf::IpAddress> _ip = sf::IpAddress::getLocalAddress();
		_status = "Tu heberges  -  ton adresse : " + (_ip ? _ip->toString() : std::string("?")) + ":" + std::to_string(GetWorld()->GetSettings().network.port);
		break;
	}
	case NetRole::Client:
		_status = std::string(_steam ? "Connecte via Steam" : "Connecte a l'hote") + "  -  ping " + std::to_string(static_cast<int>(_net->GetPingMs())) + " ms";
		break;
	}
	SetText("StatusText", _status);

	// The players of each team.
	std::string _lists[2];
	for (const NetPlayer& _player : _net->GetPlayers())
	{
		std::string& _list = _lists[BlackoutGameMode::TeamOf(_net, _player.id)];
		_list += _player.name + (_player.isHost ? "  (hote)" : "") + (_player.isLocal ? "  <- toi" : "") + "\n";
	}
	SetText("AlphaList", _lists[0].empty() ? "personne" : _lists[0], BlackoutHero::TeamColor(0));
	SetText("BravoList", _lists[1].empty() ? "personne" : _lists[1], BlackoutHero::TeamColor(1));

	if (_online && !_net->IsHost())
		if (TextWidget* _info = Cast<TextWidget>(Find("InfoText")))
			if (_info->GetLabel()->GetString().starts_with("Connecte")) SetInfo("Connecte : choisis ton equipe, l'hote lance la partie", Color(140, 230, 150));
}

// ---- Main menu: LAN or Steam

void BlackoutMenuController::BuildModeChoice()
{
	if (mainButtons.empty()) return;
	// In the places of the three top buttons of the menu.
	std::vector<Widget*> _sorted = mainButtons;
	std::sort(_sorted.begin(), _sorted.end(), [](const Widget* _a, const Widget* _b) { return _a->GetPosition().y < _b->GetPosition().y; });
	const float _step = _sorted.size() > 1 ? std::max(60.f, _sorted[1]->GetPosition().y - _sorted[0]->GetPosition().y) : 100.f;
	const bool _steamOk = GetWorld()->GetNetwork()->IsSteamReady();
	const std::pair<std::string, std::string> _choices[3] =
	{
		{ "Reseau local (LAN)", "lan" },
		{ _steamOk || !GetWorld()->GetSettings().network.steamEnabled ? "Steam (amis)" : "Steam (pas lance)", "steam" },
		{ "Retour", "multiback" },
	};
	for (int _i = 0; _i < 3; ++_i)
	{
		BlackoutButton* _button = GetLevel()->SpawnWidget<BlackoutButton>();
		_button->SetName("Mode" + _choices[_i].second);
		_button->SetPosition(_sorted.front()->GetPosition() + FVector2(0.f, _step * static_cast<float>(_i)));
		_button->SetScale(_sorted.front()->GetScale());
		_button->text = _choices[_i].first;
		_button->command = _choices[_i].second;
		_button->SetActive(false);
		modeButtons.push_back(_button);
	}
}

void BlackoutMenuController::ShowModeChoice(const bool& _show)
{
	choosingMode = _show;
	for (Widget* _widget : mainButtons) _widget->SetActive(!_show);
	for (Widget* _widget : modeButtons) _widget->SetActive(_show);
}

// ---- Lobby: Steam friends

TextWidget* BlackoutMenuController::MakeText(const FVector2& _position, const unsigned& _size, const std::string& _text)
{
	TextWidget* _widget = GetLevel()->SpawnWidget<TextWidget>(TextData(_text, _size));
	_widget->SetPosition(_position);
	_widget->GetLabel()->SetOutlineColor(Color::Black());
	_widget->GetLabel()->SetOutlineThickness(2.f);
	_widget->GetLabel()->SetZOrder(static_cast<std::uint8_t>(60));
	SetLeftText(_widget, _text);
	friendWidgets.push_back(_widget);
	return _widget;
}

void BlackoutMenuController::SetLeftText(TextWidget* _widget, const std::string& _text)
{
	if (_widget->GetLabel()->GetString() != _text) _widget->SetText(_text);
	const FRect _bounds = _widget->GetLabel()->GetLocalBounds();
	_widget->GetLabel()->SetOrigin(_bounds.GetPosition());		// SetText centers it
}

void BlackoutMenuController::BuildFriends()
{
	const FVector2 _origin = friendsPosition;
	friendsTitle = MakeText(_origin, 30u, "AMIS STEAM");
	overlayButton = GetLevel()->SpawnWidget<BlackoutButton>();
	overlayButton->SetPosition(_origin + FVector2(480.f, 18.f));
	overlayButton->SetSize(FVector2(200.f, 40.f));
	overlayButton->SetTextSize(22u);
	overlayButton->text = "Overlay Steam";
	overlayButton->command = "steamoverlay";
	friendWidgets.push_back(overlayButton);

	for (int _i = 0; _i < FRIEND_ROWS; ++_i)
	{
		const float _y = _origin.y + 70.f + FRIEND_STEP * static_cast<float>(_i);
		FriendRow _row;
		_row.name = MakeText(FVector2(_origin.x, _y - 14.f), 24u, " ");
		_row.state = MakeText(FVector2(_origin.x + 250.f, _y - 11.f), 19u, " ");
		_row.state->GetLabel()->SetFillColor(Color(160, 165, 180));
		_row.button = GetLevel()->SpawnWidget<BlackoutButton>();
		_row.button->SetPosition(FVector2(_origin.x + 480.f, _y));
		_row.button->SetSize(FVector2(200.f, 40.f));
		_row.button->SetTextSize(22u);
		_row.button->command = "steamfriend" + std::to_string(_i);
		friendWidgets.push_back(_row.button);
		friendRows.push_back(_row);
	}
	for (Widget* _widget : friendWidgets) _widget->SetActive(false);
}

void BlackoutMenuController::UpdateFriends(const float& _deltaTime)
{
	if (friendRows.empty()) return;
	NetworkManager* _net = GetWorld()->GetNetwork();
	const bool _host = _net->IsHost();
	const bool _shown = steamReady && (_host || _net->GetRole() == NetRole::Offline);
	const auto _show = [](Widget* _widget, const bool& _visible) { if (_widget->IsActive() != _visible) _widget->SetActive(_visible); };

	for (auto _it = invited.begin(); _it != invited.end();)
	{
		_it->second -= _deltaTime;
		_it = _it->second <= 0.f ? invited.erase(_it) : std::next(_it);
	}
	friendsTimer -= _deltaTime;
	if (_shown && friendsTimer <= 0.f)
	{
		friends = _net->GetSteamFriends();
		if (friends.size() > static_cast<size_t>(FRIEND_ROWS)) friends.resize(static_cast<size_t>(FRIEND_ROWS));
		friendsTimer = 2.f;
	}

	_show(friendsTitle, _shown);
	_show(overlayButton, _shown && _host);
	if (_shown)
	{
		const std::string _title = friends.empty() ? "AMIS STEAM : aucun en ligne" : (_host ? "AMIS STEAM : invite-les" : "AMIS STEAM");
		SetLeftText(friendsTitle, _title);
	}
	for (size_t _i = 0; _i < friendRows.size(); ++_i)
	{
		const FriendRow& _row = friendRows[_i];
		const bool _used = _shown && _i < friends.size();
		_show(_row.name, _used);
		_show(_row.state, _used);
		if (!_used)
		{
			_show(_row.button, false);
			continue;
		}
		const SteamFriend& _friend = friends[_i];
		const std::string _name = _friend.name.size() > 16 ? _friend.name.substr(0, 15) + "." : _friend.name;
		const std::string _state = !_friend.inThisGame ? "en ligne" : (_friend.lobby != 0 ? "en partie" : "dans le jeu");
		SetLeftText(_row.name, _name);
		SetLeftText(_row.state, _state);
		_row.name->GetLabel()->SetFillColor(_friend.inThisGame ? Color(140, 230, 150) : Color(230, 232, 240));

		// Host: invite anybody. Before a game: join the game of a friend who has one.
		const bool _wasInvited = invited.contains(_friend.id);
		_row.button->text = _host ? (_wasInvited ? "Invite !" : "Inviter") : "Rejoindre";
		_row.button->SetSelected(_host && _wasInvited);
		_show(_row.button, _host || _friend.lobby != 0);
	}
}

}
