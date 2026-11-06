#include "BlackoutKeys.h"
#include "BlackoutMenus.h"
#include "Core/Engine.h"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace Krampus
{

namespace
{
	using Scan = sf::Keyboard::Scancode;

	std::string KeysFile()
	{
		return CONTENT_PATH + "Saves/BlackoutKeys.cfg";
	}

	const char* const ACTION_IDS[BlackoutKeys::COUNT] =		// in the file (the names never change, the labels may)
	{
		"MoveUp", "MoveDown", "MoveLeft", "MoveRight", "Walk", "Reload", "Grenade", "Flash", "Smoke", "Use", "Shop", "Primary", "Sidearm",
		"ChatAll", "ChatTeam", "Ping"
	};
}

// ---- BlackoutKeys

std::string BlackoutKeys::Label(const int& _action)
{
	static const char* const LABELS[COUNT] =
	{
		"Avancer", "Reculer", "Aller a gauche", "Aller a droite", "Marcher (silencieux)", "Recharger", "Grenade", "Flash", "Smoke",
		"Poser / desamorcer / ramasser", "Boutique", "Arme principale", "Pistolet", "Chat general", "Chat d'equipe",
		"Ping (aussi clic molette)"
	};
	return _action >= 0 && _action < COUNT ? LABELS[_action] : "?";
}

sf::Keyboard::Scancode BlackoutKeys::Default(const int& _action)
{
	static const Scan DEFAULTS[COUNT] =
	{
		Scan::W, Scan::S, Scan::A, Scan::D, Scan::LShift, Scan::R, Scan::G, Scan::F, Scan::C, Scan::E, Scan::B, Scan::Num1, Scan::Num2,
		Scan::Enter, Scan::T, Scan::V
	};
	return _action >= 0 && _action < COUNT ? DEFAULTS[_action] : Scan::Unknown;
}

std::array<sf::Keyboard::Scancode, BlackoutKeys::COUNT>& BlackoutKeys::Keys()
{
	static std::array<Scan, COUNT> _keys{};
	static bool _loaded = false;
	if (_loaded) return _keys;
	_loaded = true;

	for (int _action = 0; _action < COUNT; ++_action) _keys[static_cast<size_t>(_action)] = Default(_action);
	std::ifstream _file(KeysFile());
	std::string _line;
	while (std::getline(_file, _line))
	{
		const size_t _equal = _line.find('=');
		if (_equal == std::string::npos) continue;
		std::string _id = _line.substr(0, _equal);
		_id.erase(std::remove_if(_id.begin(), _id.end(), [](unsigned char _c) { return std::isspace(_c); }), _id.end());
		int _code = -1;
		std::istringstream(_line.substr(_equal + 1)) >> _code;
		for (int _action = 0; _action < COUNT; ++_action)
			if (_id == ACTION_IDS[_action] && _code >= 0 && _code < static_cast<int>(sf::Keyboard::ScancodeCount))
				_keys[static_cast<size_t>(_action)] = static_cast<Scan>(_code);
	}
	return _keys;
}

void BlackoutKeys::Save()
{
	std::error_code _error;
	std::filesystem::create_directories(std::filesystem::path(KeysFile()).parent_path(), _error);
	std::ofstream _file(KeysFile(), std::ios::trunc);
	if (!_file)
	{
		LOG_WARNING("Blackout: cannot write " + KeysFile());
		return;
	}
	_file << "# Blackout: the keys (scancodes: places on the keyboard). Changed in the main menu > Touches.\n";
	for (int _action = 0; _action < COUNT; ++_action)
		_file << ACTION_IDS[_action] << " = " << static_cast<int>(Keys()[static_cast<size_t>(_action)]) << "\n";
	LOG(VerbosityType::Log, "Blackout: keys saved");
}

sf::Keyboard::Scancode BlackoutKeys::Get(const int& _action)
{
	return _action >= 0 && _action < COUNT ? Keys()[static_cast<size_t>(_action)] : Scan::Unknown;
}

void BlackoutKeys::Set(const int& _action, const sf::Keyboard::Scancode& _key)
{
	if (_action < 0 || _action >= COUNT) return;
	std::array<Scan, COUNT>& _keys = Keys();
	const Scan _old = _keys[static_cast<size_t>(_action)];
	for (Scan& _other : _keys)
		if (_other == _key) _other = _old;		// the action that had it takes the old key
	_keys[static_cast<size_t>(_action)] = _key;
	Save();
}

void BlackoutKeys::ResetAll()
{
	for (int _action = 0; _action < COUNT; ++_action) Keys()[static_cast<size_t>(_action)] = Default(_action);
	Save();
}

bool BlackoutKeys::Down(const int& _action)
{
	const Scan _key = Get(_action);
	return _key != Scan::Unknown && sf::Keyboard::isKeyPressed(_key);
}

std::string BlackoutKeys::KeyName(const sf::Keyboard::Scancode& _key)
{
	if (_key == Scan::Unknown) return "-";
	// The top row: its digit (an AZERTY keyboard would say "&", "é"...).
	if (_key >= Scan::Num1 && _key <= Scan::Num9) return std::to_string(static_cast<int>(_key) - static_cast<int>(Scan::Num1) + 1);
	if (_key == Scan::Num0) return "0";
	const std::string _name = sf::Keyboard::getDescription(_key).toAnsiString();
	return _name.empty() ? "?" : _name;
}

std::string BlackoutKeys::KeyName(const int& _action)
{
	return KeyName(Get(_action));
}

// ---- BlackoutKeysMenu

namespace
{
	const float		ROW_TOP		= 200.f;
	const float		ROW_STEP	= 46.f;
}

BlackoutKeysMenu::BlackoutKeysMenu(Level* _level)
	: Actor(_level)
{
	name = "BlackoutKeysMenu";
}

TextWidget* BlackoutKeysMenu::MakeText(const FVector2& _position, const unsigned& _size, const std::string& _text, const float& _align)
{
	TextWidget* _widget = GetLevel()->SpawnWidget<TextWidget>(TextData(_text, _size));
	_widget->SetPosition(_position);
	_widget->GetLabel()->SetOutlineColor(Color::Black());
	_widget->GetLabel()->SetOutlineThickness(2.f);
	_widget->GetLabel()->SetZOrder(static_cast<std::uint8_t>(60));
	const FRect _bounds = _widget->GetLabel()->GetLocalBounds();
	_widget->GetLabel()->SetOrigin(FVector2(_bounds.GetPosition().x + _bounds.GetSize().x * _align, _bounds.GetPosition().y));
	widgets.push_back(_widget);
	return _widget;
}

void BlackoutKeysMenu::Build()
{
	built = true;
	panel = GetLevel()->SpawnWidget<ImageWidget>(RectangleShapeData(FVector2(1000.f, 990.f), "Blackout/Solid"));
	panel->SetPosition(FVector2(960.f, 540.f));
	panel->GetImage()->GetShapeObject()->GetShape()->SetFillColor(Color(10, 12, 20, 240));
	panel->GetImage()->GetShapeObject()->GetShape()->SetOutlineThickness(2.f);
	panel->GetImage()->GetShapeObject()->GetShape()->SetOutlineColor(Color(110, 130, 170, 220));
	panel->GetImage()->SetZOrder(static_cast<std::uint8_t>(40));
	widgets.push_back(panel);

	MakeText(FVector2(960.f, 70.f), 56u, "TOUCHES", 0.5f);
	hint = MakeText(FVector2(960.f, 145.f), 22u, "", 0.5f);

	// A line per action: its name on the left, its key on a button on the right.
	for (int _action = 0; _action < BlackoutKeys::COUNT; ++_action)
	{
		const float _y = ROW_TOP + ROW_STEP * static_cast<float>(_action);
		MakeText(FVector2(520.f, _y - 14.f), 26u, BlackoutKeys::Label(_action), 0.f);
		BlackoutButton* _button = GetLevel()->SpawnWidget<BlackoutButton>();
		_button->SetPosition(FVector2(1210.f, _y));
		_button->SetSize(FVector2(320.f, 42.f));
		_button->command = "key" + std::to_string(_action);
		keyButtons.push_back(_button);
		widgets.push_back(_button);
	}

	const float _bottom = ROW_TOP + ROW_STEP * static_cast<float>(BlackoutKeys::COUNT) + 30.f;
	BlackoutButton* _reset = GetLevel()->SpawnWidget<BlackoutButton>();
	_reset->SetPosition(FVector2(760.f, _bottom));
	_reset->SetSize(FVector2(340.f, 56.f));
	_reset->text = "Par defaut";
	_reset->command = "keysdefault";
	widgets.push_back(_reset);
	BlackoutButton* _back = GetLevel()->SpawnWidget<BlackoutButton>();
	_back->SetPosition(FVector2(1160.f, _bottom));
	_back->SetSize(FVector2(340.f, 56.f));
	_back->text = "Retour";
	_back->command = "keysback";
	widgets.push_back(_back);
}

void BlackoutKeysMenu::Open()
{
	if (open) return;
	// The widgets of the scene hide (their buttons would take the clicks), the menu's show.
	hidden.clear();
	for (const std::unique_ptr<Widget>& _widget : GetLevel()->GetHUD().GetWidgets())
		if (_widget->IsActive() && !_widget->IsPendingDestroy() && std::find(widgets.begin(), widgets.end(), _widget.get()) == widgets.end())
			hidden.push_back(_widget.get());
	for (Widget* _widget : hidden) _widget->SetActive(false);		// after the walk over the list
	if (!built) Build();
	for (Widget* _widget : widgets) _widget->SetActive(true);
	open = true;
	listening = -1;
	Refresh();
	LOG(VerbosityType::Log, "Blackout: keys menu opened");
}

void BlackoutKeysMenu::Close()
{
	if (!open) return;
	open = false;
	listening = -1;
	for (Widget* _widget : widgets) _widget->SetActive(false);
	for (Widget* _widget : hidden) _widget->SetActive(true);
	hidden.clear();
}

void BlackoutKeysMenu::Command(const std::string& _command)
{
	// A click comes while the HUD goes through its widgets: showing, hiding or making widgets now would break that walk. Done in Tick.
	pending = _command;
}

void BlackoutKeysMenu::RunCommand(const std::string& _command)
{
	if (_command == "keys")
	{
		Open();
		return;
	}
	if (!open) return;
	if (_command == "keysback") Close();
	else if (_command == "keysdefault")
	{
		BlackoutKeys::ResetAll();
		listening = -1;
		Refresh();
	}
	else if (_command.starts_with("key"))
	{
		int _action = -1;
		std::istringstream(_command.substr(3)) >> _action;
		if (_action < 0 || _action >= BlackoutKeys::COUNT) return;
		listening = _action;
		released = false;
		Refresh();
	}
}

void BlackoutKeysMenu::Refresh()
{
	for (int _action = 0; _action < static_cast<int>(keyButtons.size()); ++_action)
	{
		BlackoutButton* _button = keyButtons[static_cast<size_t>(_action)];
		_button->text = _action == listening ? std::string("...") : BlackoutKeys::KeyName(_action);
		_button->SetSelected(_action == listening);
	}
	const std::string _text = listening >= 0
		? "Appuie sur la nouvelle touche pour \"" + BlackoutKeys::Label(listening) + "\"   (Echap : annuler)"
		: "Clique une touche pour la changer. Toujours actifs : fleches, clic gauche (tirer), clic droit (grenade), clic molette (ping), Echap.";
	hint->SetText(_text);
	const FRect _bounds = hint->GetLabel()->GetLocalBounds();
	hint->GetLabel()->SetOrigin(FVector2(_bounds.GetPosition().x + _bounds.GetSize().x * 0.5f, _bounds.GetPosition().y));
	hint->GetLabel()->SetFillColor(listening >= 0 ? Color(255, 215, 120) : Color(190, 195, 210));
}

void BlackoutKeysMenu::Tick(const Float& _deltaTime)
{
	Actor::Tick(_deltaTime);
	const bool _focus = GetWorld()->GetWindowRef().HasFocus();
	const bool _escape = _focus && sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Escape);
	const bool _escapePressed = _escape && !escapeHeld;
	escapeHeld = _escape;
	if (!pending.empty())
	{
		const std::string _command = pending;
		pending.clear();
		RunCommand(_command);
	}
	if (!open || !_focus) return;
	if (listening < 0)
	{
		if (_escapePressed) Close();		// Escape closes the menu
		return;
	}

	// The first key pressed once every key is up (the click that asked for it is a mouse button: nothing to wait for, but a key
	// held from before must be released first).
	sf::Keyboard::Scancode _pressed = sf::Keyboard::Scancode::Unknown;
	for (int _code = 0; _code < static_cast<int>(sf::Keyboard::ScancodeCount); ++_code)
		if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Scancode>(_code)))
		{
			_pressed = static_cast<sf::Keyboard::Scancode>(_code);
			break;
		}
	if (!released)
	{
		released = _pressed == sf::Keyboard::Scancode::Unknown;
		return;
	}
	if (_pressed == sf::Keyboard::Scancode::Unknown) return;

	if (_pressed != sf::Keyboard::Scancode::Escape)
	{
		BlackoutKeys::Set(listening, _pressed);
		LOG(VerbosityType::Log, "Blackout: " + BlackoutKeys::Label(listening) + " = " + BlackoutKeys::KeyName(_pressed));
	}
	listening = -1;
	Refresh();
}

}
