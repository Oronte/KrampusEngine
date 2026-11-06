#include "BlackoutChat.h"
#include "BlackoutGameMode.h"
#include "BlackoutHero.h"
#include "BlackoutKeys.h"
#include "Core/Engine.h"

namespace Krampus
{

namespace
{
	const float		LINE_HEIGHT		= 30.f;
	const float		SHOW_TIME		= 12.f;		// seconds a message stays, then it fades
	const float		FADE_TIME		= 2.f;

	// The label of a TextWidget is centered on it: its left side goes to the widget's position instead.
	void SetLeftText(TextWidget* _widget, const std::string& _text)
	{
		if (_widget->GetLabel()->GetString() != _text) _widget->SetText(String(_text));
		const FRect _bounds = _widget->GetLabel()->GetLocalBounds();
		_widget->GetLabel()->SetOrigin(FVector2(_bounds.GetPosition().x, _bounds.GetPosition().y));
	}
}

BlackoutChat::BlackoutChat(Level* _level)
	: Actor(_level)
{
	name = "BlackoutChat";
}

BlackoutChat* BlackoutChat::Of(Level* _level)
{
	if (!_level) return nullptr;
	const std::vector<BlackoutChat*> _chats = _level->GetActorManager().GetAllActorOfClass<BlackoutChat>();
	return _chats.empty() ? nullptr : _chats.front();
}

void BlackoutChat::Build(const FVector2& _bottomLeft, const float& _width)
{
	Level* _level = GetLevel();
	for (int _i = 0; _i < LINES; ++_i)
	{
		TextWidget* _line = _level->SpawnWidget<TextWidget>(TextData("", 24u));
		_line->SetPosition(FVector2(_bottomLeft.x, _bottomLeft.y - LINE_HEIGHT * static_cast<float>(LINES - 1 - _i)));
		_line->GetLabel()->SetOutlineColor(Color::Black());
		_line->GetLabel()->SetOutlineThickness(2.f);
		_line->GetLabel()->SetZOrder(static_cast<std::uint8_t>(60));
		shown.push_back(_line);
	}

	prompt = _level->SpawnWidget<TextWidget>(TextData("", 24u));
	prompt->SetPosition(FVector2(_bottomLeft.x, _bottomLeft.y + 42.f));
	prompt->GetLabel()->SetOutlineColor(Color::Black());
	prompt->GetLabel()->SetOutlineThickness(2.f);
	prompt->GetLabel()->SetZOrder(static_cast<std::uint8_t>(60));

	entry = _level->SpawnWidget<TextEntryWidget>();
	entry->size = FVector2(_width, 40.f);
	entry->SetPosition(FVector2(_bottomLeft.x + 110.f + _width / 2.f, _bottomLeft.y + 54.f));
	entry->placeholder = String("Ton message...");
	entry->maxLength = 90;
	entry->characterSize = 22;
	entry->backgroundColor = Color(10, 12, 18, 220);
	entry->outlineColor = Color(110, 130, 170, 220);
	entry->outlineThickness = 2.f;
	entry->SetActive(false);
	submitHandle = entry->onSubmit.AddListener([this](const std::string& _text)
		{
			NetworkManager* _net = GetWorld()->GetNetwork();
			if (!_text.empty() && _net->IsOnline())
			{
				const int _team = toTeam ? BlackoutGameMode::TeamOf(_net, _net->GetLocalId()) : -1;
				_net->Send("BO_Chat", NetworkManager::Args(_team, _net->GetPlayerName(_net->GetLocalId()), _text), NetTarget::All);
			}
			Close();
		});

	// Every machine gets every message: a team message only shows to the players of that team.
	messageHandle = GetWorld()->GetNetwork()->Subscribe("BO_Chat", [this](const NetMessage& _message)
		{
			NetworkManager* _net = GetWorld()->GetNetwork();
			const int _team = _message.Int(0);
			if (_team >= 0 && _team != BlackoutGameMode::TeamOf(_net, _net->GetLocalId())) return;
			const std::string _who = _message.String(1) + " : " + _message.String(2);
			if (_team >= 0) AddInfo("[EQUIPE] " + _who, BlackoutHero::TeamColor(_team));
			else AddInfo("[TOUS] " + _who, Color(235, 235, 240));
		});
	Refresh();
}

void BlackoutChat::AddInfo(const std::string& _text, const Color& _color)
{
	lines.push_back(Line{ _text, _color, 0.f });
	if (lines.size() > static_cast<size_t>(LINES)) lines.erase(lines.begin());
	Refresh();
}

void BlackoutChat::Open(const bool& _team)
{
	typing = true;
	toTeam = _team;
	entry->SetActive(true);
	entry->SetText("");
	entry->SetFocused(true);
	Refresh();
}

void BlackoutChat::Close()
{
	typing = false;
	sinceClosed = 0.f;
	enterHeld = true;		// the Enter that sent the message must not open the chat again
	teamHeld = true;
	entry->SetFocused(false);
	entry->SetActive(false);
	Refresh();
}

void BlackoutChat::Tick(const Float& _deltaTime)
{
	Actor::Tick(_deltaTime);
	if (!entry) return;
	const float _delta = static_cast<float>(_deltaTime);
	sinceClosed += _delta;
	for (Line& _line : lines) _line.age += _delta;

	NetworkManager* _net = GetWorld()->GetNetwork();
	const bool _focus = GetWorld()->GetWindowRef().HasFocus();
	const bool _enter = _focus && BlackoutKeys::Down(BlackoutKeys::ChatAll);		// the keys chosen in the menu (Enter, T)
	const bool _team = _focus && BlackoutKeys::Down(BlackoutKeys::ChatTeam);

	if (typing)
	{
		// Escape, or a click elsewhere (the line lost the keyboard): nothing is sent.
		if ((_focus && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) || !entry->IsFocused()) Close();
	}
	else if (_net->IsOnline())
	{
		// Not while another line of text has the keyboard (the name in the lobby).
		bool _otherTyping = false;
		for (const std::unique_ptr<Widget>& _widget : GetLevel()->GetHUD().GetWidgets())
			if (const TextEntryWidget* _other = Cast<TextEntryWidget>(_widget.get()); _other && _other != entry && _other->IsFocused()) _otherTyping = true;

		if (!_otherTyping && _enter && !enterHeld) Open(false);
		else if (!_otherTyping && _team && !teamHeld) Open(true);
	}
	enterHeld = _enter;
	teamHeld = _team;
	Refresh();
}

void BlackoutChat::Refresh()
{
	if (shown.empty()) return;

	// The newest message at the bottom.
	const int _first = static_cast<int>(lines.size()) - LINES;
	for (int _i = 0; _i < LINES; ++_i)
	{
		const int _index = _first + _i;
		TextWidget* _widget = shown[static_cast<size_t>(_i)];
		if (_index < 0)
		{
			SetLeftText(_widget, "");
			continue;
		}
		const Line& _line = lines[static_cast<size_t>(_index)];
		Color _color = _line.color;
		const float _fade = typing ? 1.f : std::clamp(1.f - (_line.age - SHOW_TIME) / FADE_TIME, 0.f, 1.f);
		_color.a = static_cast<std::uint8_t>(255.f * _fade);
		SetLeftText(_widget, _line.text);
		_widget->GetLabel()->SetFillColor(_color);
		_widget->GetLabel()->SetOutlineColor(Color(0, 0, 0, static_cast<std::uint8_t>(255.f * _fade)));
	}

	if (typing)
	{
		SetLeftText(prompt, toTeam ? "[EQUIPE]" : "[TOUS]");
		NetworkManager* _net = GetWorld()->GetNetwork();
		prompt->GetLabel()->SetFillColor(toTeam ? BlackoutHero::TeamColor(BlackoutGameMode::TeamOf(_net, _net->GetLocalId())) : Color(235, 235, 240));
	}
	else
	{
		const bool _online = GetWorld()->GetNetwork()->IsOnline();
		SetLeftText(prompt, _online ? BlackoutKeys::KeyName(BlackoutKeys::ChatAll) + " : chat general    " + BlackoutKeys::KeyName(BlackoutKeys::ChatTeam) + " : chat d'equipe" : std::string());
		prompt->GetLabel()->SetFillColor(Color(150, 155, 170, 200));
	}
}

}
