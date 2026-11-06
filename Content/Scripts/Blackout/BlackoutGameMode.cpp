#include "BlackoutGameMode.h"
#include "BlackoutHero.h"
#include "BlackoutEnemy.h"
#include "BlackoutGrenade.h"
#include "BlackoutCharge.h"
#include "BlackoutChat.h"
#include "BlackoutKeys.h"
#include "BlackoutMenus.h"
#include "Core/Engine.h"
#include "Components/CameraComponent.h"
#include "Components/NetworkSyncComponent.h"
#include "Components/StainComponent.h"
#include "Graphics/Particles/ParticleEmitter.h"
#include "Scene/Scene.h"
#include "Navigation/NavigationComponents.h"
#include "Graphics/Shapes/PolygonShape.h"
#include "Layers/PauseLayer.h"

namespace Krampus
{

namespace
{
	const size_t	ECHO_COUNT		= 32;
	const float		ECHO_DURATION	= 0.9f;		// seconds a ring lives
	const FVector2	HUD_SIZE		= FVector2(1920.f, 1080.f);
	const float		NEXT_ROUND_TIME	= 5.f;		// seconds between the end of a round and the next one (versus)
	const float		PICKUP_RADIUS	= 50.f;		// an attacker this close to the dropped charge takes it
	const float		DEFUSE_RADIUS	= 80.f;		// a defender this close to the planted charge can defuse it
	const float		BLAST_RADIUS	= 520.f;	// the charge kills everybody this close
	const Color		CHARGE_COLOR	= Color(255, 205, 80);
	const float		CHARGE_START	= 150.f;	// pixels from the first attacker's spot to the charge at the start of a round
	const float		CONE_LENGTH		= 650.f;	// pixels: the red cone of the spread (shorter when the gun shoots less far)
	const float		HEAD_HEIGHT		= 45.f;		// pixels over the center of a soldier: where his name is
	const float		NEAR_SIGHT		= 190.f;	// pixels: seen all around (the light around a soldier)
	const float		WEAPON_PICKUP	= 70.f;		// pixels: E takes a gun on the floor this close
	const FVector2	PORTRAIT_SIZE	= FVector2(56.f, 42.f);		// Player.png in the bar at the top
	const float		PORTRAIT_STEP	= 64.f;
	const float		DEGREES			= 3.14159265f / 180.f;
	const Color		ALLY_COLOR		= Color(90, 160, 255);
	const Color		PING_COLOR		= Color(90, 235, 255);
	const Color		ENEMY_COLOR		= Color(255, 70, 60);

	std::string Clock(const float& _seconds)
	{
		const int _total = static_cast<int>(std::ceil(std::max(0.f, _seconds)));
		const std::string _secs = std::to_string(_total % 60);
		return std::to_string(_total / 60) + ":" + (_secs.size() < 2 ? "0" : "") + _secs;
	}
}

// ---- BlackoutRingWidget

BlackoutRingWidget::BlackoutRingWidget(Level* _level)
	: Widget(_level)
{
	name = "BlackoutRing";
	ring = CreateComponent<ImageWidgetComponent>(CircleShapeData(20.f));
	ring->GetShapeObject()->GetShape()->ClearTexture();
	ring->GetShapeObject()->GetShape()->SetFillColor(Color(0, 0, 0, 0));
	ring->GetShapeObject()->GetShape()->SetOutlineThickness(3.f);
	ring->GetShapeObject()->GetShape()->SetOutlineColor(Color(0, 0, 0, 0));
}

// ---- BlackoutGameMode

BlackoutGameMode::BlackoutGameMode(Level* _level)
	: Actor(_level)
{
	name = "BlackoutGameMode";

	// Almost black: only the lights of the hero (and of the bullets) show the level.
	ambient = CreateComponent<GlobalLightComponent>();
	ambient->color = Color(9, 10, 15);

	// The enemies find their way around the walls (static bodies) on this navigation mesh.
	NavMeshBoundsComponent* _navigation = CreateComponent<NavMeshBoundsComponent>();
	_navigation->size = arenaSize;
	_navigation->cellSize = 16.f;
	_navigation->agentRadius = 26.f;
}

BlackoutGameMode* BlackoutGameMode::Of(Level* _level)
{
	if (!_level) return nullptr;
	const std::vector<BlackoutGameMode*> _modes = _level->GetActorManager().GetAllActorOfClass<BlackoutGameMode>();
	return _modes.empty() ? nullptr : _modes.front();
}

int BlackoutGameMode::TeamOf(const NetworkManager* _net, const int& _playerId)
{
	return std::clamp(_net->GetShared<int>("team_" + std::to_string(_playerId), _playerId % 2), 0, 1);
}

std::string BlackoutGameMode::TeamName(const int& _team)
{
	return _team == 0 ? "ALPHA" : "BRAVO";
}

bool BlackoutGameMode::IsMoveBlocked() const
{
	return action != 0 || (chat && chat->IsTyping());
}

bool BlackoutGameMode::IsFireBlocked() const
{
	return IsMoveBlocked() || shopOpen || buyLeft > 0.f;
}

void BlackoutGameMode::Blind(const float& _seconds)
{
	blindLeft = std::max(blindLeft, _seconds);
}

int BlackoutGameMode::Credits(const int& _player) const
{
	return GetWorld()->GetNetwork()->GetShared<int>("credits_" + std::to_string(_player), versus ? START_CREDITS : static_cast<int>(soloCredits));
}

void BlackoutGameMode::HeroStep(const BlackoutCharacter& _hero)
{
	Echo(_hero.GetPosition(), Color(200, 205, 220), 26.f);		// the noise you make
	if (versus) return;
	for (BlackoutEnemy* _enemy : GetLevel()->GetActorManager().GetAllActorOfClass<BlackoutEnemy>())
		if (!_enemy->IsDead() && (_enemy->GetPosition() - _hero.GetPosition()).Length() < 520.f)
			_enemy->Hear(_hero.GetPosition());
}

// ---- The shop

void BlackoutGameMode::BuildShop()
{
	// Like a tactical shooter: a big panel, a column per kind of gun and one for the equipment, a card per item (its picture, its
	// name, its price). The columns: an item, or a heading (-1 - its index in _headings).
	shopPanel = GetLevel()->SpawnWidget<ImageWidget>(RectangleShapeData(FVector2(1460.f, 790.f), "Blackout/Solid"));
	shopPanel->SetPosition(FVector2(915.f, 575.f));
	shopPanel->GetImage()->GetShapeObject()->GetShape()->SetFillColor(Color(8, 10, 16, 238));
	shopPanel->GetImage()->GetShapeObject()->GetShape()->SetOutlineThickness(2.f);
	shopPanel->GetImage()->GetShapeObject()->GetShape()->SetOutlineColor(Color(110, 130, 170, 220));
	shopPanel->GetImage()->SetZOrder(static_cast<std::uint8_t>(45));
	shopTitle = MakeText(FVector2(915.f, 196.f), 34u);

	const std::vector<std::string> _headings = { BlackoutWeapon::CategoryName(BlackoutWeapon::Sidearm), BlackoutWeapon::CategoryName(BlackoutWeapon::Shotgun),
		BlackoutWeapon::CategoryName(BlackoutWeapon::Sniper), BlackoutWeapon::CategoryName(BlackoutWeapon::Rifle), "BOUCLIERS", "EQUIPEMENT" };
	const std::vector<std::pair<float, std::vector<int>>> _columns =
	{
		{ 390.f,	{ -1, 0, 1, 2, 3 } },
		{ 740.f,	{ -2, 4, 5, -3, 9, 10 } },
		{ 1090.f,	{ -4, 6, 7, 8 } },
		{ 1440.f,	{ -5, ITEM_LIGHT_SHIELD, ITEM_HEAVY_SHIELD, -6, ITEM_GRENADE, ITEM_FLASH, ITEM_SMOKE } },
	};
	const FVector2 _card(320.f, 104.f);
	for (const auto& [_x, _entries] : _columns)
	{
		float _y = 240.f;		// the top of what comes next
		for (const int _entry : _entries)
		{
			if (_entry < 0)
			{
				TextWidget* _heading = MakeText(FVector2(_x - _card.x / 2.f, _y), 24u);
				SetText(_heading, _headings[static_cast<size_t>(-1 - _entry)], 0.f);
				_heading->GetLabel()->SetFillColor(Color(150, 165, 195));
				shopHeaders.push_back(_heading);
				_y += 40.f;
				continue;
			}

			ShopCard _shopCard;
			_shopCard.item = _entry;
			const FVector2 _center(_x, _y + _card.y / 2.f);
			_shopCard.button = GetLevel()->SpawnWidget<BlackoutButton>();
			_shopCard.button->SetPosition(_center);
			_shopCard.button->SetSize(_card);
			_shopCard.button->text = String(" ");		// the card's texts are apart (an empty text would keep "Bouton")
			_shopCard.button->command = String("buy" + std::to_string(_entry));

			// The picture: the gun from the side; a colored chip for the rest.
			if (_entry < BlackoutWeapon::COUNT)
			{
				_shopCard.icon = GetLevel()->SpawnWidget<ImageWidget>(RectangleShapeData(FVector2(100.f, 40.f), BlackoutWeapon::Get(_entry).icon));
				ShowWeaponIcon(_shopCard.icon, _entry, FVector2(190.f, 52.f));
				_shopCard.icon->SetPosition(_center + FVector2(-40.f, -12.f));
			}
			else
			{
				const bool _shield = _entry == ITEM_LIGHT_SHIELD || _entry == ITEM_HEAVY_SHIELD;
				_shopCard.icon = GetLevel()->SpawnWidget<ImageWidget>(RectangleShapeData(FVector2(34.f, _entry == ITEM_HEAVY_SHIELD ? 46.f : 34.f), "Blackout/Solid"));
				if (!_shield) _shopCard.icon->GetImage()->SetRadius(16.f);		// a grenade: a round chip
				const Color _chip = _entry == ITEM_LIGHT_SHIELD ? Color(120, 190, 255) : _entry == ITEM_HEAVY_SHIELD ? Color(60, 120, 255) :
					_entry == ITEM_GRENADE ? Color(90, 120, 70) : _entry == ITEM_FLASH ? Color(225, 230, 240) : Color(130, 135, 145);
				_shopCard.icon->GetImage()->GetShapeObject()->GetShape()->SetFillColor(_chip);
				_shopCard.icon->SetPosition(_center + FVector2(-110.f, -12.f));
			}
			_shopCard.icon->GetImage()->SetZOrder(static_cast<std::uint8_t>(55));		// over the card

			_shopCard.name = MakeText(_center + FVector2(-_card.x / 2.f + 14.f, 14.f), 22u);
			_shopCard.name->GetLabel()->SetOutlineThickness(2.f);
			_shopCard.price = MakeText(_center + FVector2(_card.x / 2.f - 12.f, -_card.y / 2.f + 8.f), 22u);
			_shopCard.price->GetLabel()->SetOutlineThickness(2.f);
			shopCards.push_back(_shopCard);
			_y += _card.y + 12.f;
		}
	}
	SetShopOpen(false);
}

void BlackoutGameMode::ShowWeaponIcon(ImageWidget* _image, const int& _weapon, const FVector2& _box)
{
	// As big as fits in _box, keeping the shape of the picture, in the color of the gun.
	const BlackoutWeapon& _stats = BlackoutWeapon::Get(_weapon);
	ShapeObject* _shape = _image->GetImage()->GetShapeObject();
	if (_shape->GetTexturePath() != _stats.icon) _shape->SetTexture(_stats.icon);
	const UVector2 _pixels = _shape->GetTextureRef().GetSize();
	const FVector2 _picture(static_cast<float>(_pixels.x), static_cast<float>(_pixels.y));
	const float _scale = _picture.x > 0.f && _picture.y > 0.f ? std::min(_box.x / _picture.x, _box.y / _picture.y) : 1.f;
	_image->GetImage()->SetSize(FVector2(_picture.x * _scale, _picture.y * _scale));
	_shape->GetShape()->SetFillColor(_stats.tint);
}

int BlackoutGameMode::SellValue(const int& _player, const int& _slot)
{
	const std::string _key = (_slot == 0 ? "primary_" : "sidearm_") + std::to_string(_player);
	const int _weapon = GetWorld()->GetNetwork()->GetShared<int>(_key, _slot == 0 ? -1 : BlackoutWeapon::PISTOL);
	if (_weapon < 0 || _weapon >= BlackoutWeapon::COUNT || _weapon == BlackoutWeapon::PISTOL) return 0;
	const auto _fresh = freshWeapon.find(_player * 2 + _slot);
	const int _price = BlackoutWeapon::Get(_weapon).price;
	return _fresh != freshWeapon.end() && _fresh->second == _weapon ? _price : _price / 2;
}

void BlackoutGameMode::ResetLoadout(const int& _player)
{
	NetworkManager* _net = GetWorld()->GetNetwork();
	const std::string _id = std::to_string(_player);
	_net->SetShared("primary_" + _id, -1);
	_net->SetShared("sidearm_" + _id, BlackoutWeapon::PISTOL);
	_net->SetShared("shield_" + _id, 0);
	freshWeapon.erase(_player * 2);
	freshWeapon.erase(_player * 2 + 1);
}

void BlackoutGameMode::SetShopOpen(const bool& _open)
{
	shopOpen = _open;
	shopKeyHeld = true;
	if (!shopPanel) return;
	shopPanel->SetActive(_open);
	shopTitle->SetActive(_open);
	for (TextWidget* _heading : shopHeaders) _heading->SetActive(_open);
	for (ShopCard& _card : shopCards)
	{
		_card.button->SetActive(_open);
		_card.icon->SetActive(_open);
		_card.name->SetActive(_open);
		_card.price->SetActive(_open);
	}
	UpdateShop();
}

void BlackoutGameMode::UpdateShop()
{
	if (!shopOpen) return;
	NetworkManager* _net = GetWorld()->GetNetwork();
	const int _me = _net->GetLocalId();
	const int _credits = Credits(_me);
	BlackoutHero* _hero = GetHero();
	SetText(shopTitle, "BOUTIQUE          Credits : " + std::to_string(_credits) + "          B : fermer", 0.5f);

	for (ShopCard& _card : shopCards)
	{
		const int _item = _card.item;
		const int _price = ItemPrice(_item);
		bool _owned = false, _full = false;
		int _budget = _credits;
		std::string _name, _priceText = _price == 0 ? std::string("GRATUIT") : std::to_string(_price);

		if (_item < BlackoutWeapon::COUNT)
		{
			// A gun: the one of its kind is sold for it (all its price if bought in this round, half otherwise).
			const BlackoutWeapon& _stats = BlackoutWeapon::Get(_item);
			const int _slot = _stats.IsSidearm() ? 1 : 0;
			const int _sell = SellValue(_me, _slot);
			_owned = _hero && (_slot == 0 ? _hero->GetPrimary() : _hero->GetSidearm()) == _item;
			_budget += _sell;
			_name = _stats.name;
			if (_owned) _priceText = "EQUIPE";
			else if (_sell > 0) _priceText += "  (+" + std::to_string(_sell) + ")";
		}
		else if (_item == ITEM_LIGHT_SHIELD || _item == ITEM_HEAVY_SHIELD)
		{
			const float _value = _item == ITEM_LIGHT_SHIELD ? BlackoutWeapon::LIGHT_SHIELD : BlackoutWeapon::HEAVY_SHIELD;
			_full = _hero && _hero->GetShield() >= _value;
			_name = std::string(_item == ITEM_LIGHT_SHIELD ? "Bouclier leger" : "Bouclier lourd") + "  +" + std::to_string(static_cast<int>(_value));
			if (_full) _priceText = "EQUIPE";
		}
		else
		{
			const int _have = !_hero ? 0 : (_item == ITEM_GRENADE ? _hero->GetGrenades() : (_item == ITEM_FLASH ? _hero->GetFlashes() : _hero->GetSmokes()));
			const int _max = _item == ITEM_GRENADE ? BlackoutWeapon::MAX_GRENADES : (_item == ITEM_FLASH ? BlackoutWeapon::MAX_FLASHES : BlackoutWeapon::MAX_SMOKES);
			_full = _have >= _max;
			_name = std::string(_item == ITEM_GRENADE ? "Grenade" : (_item == ITEM_FLASH ? "Flash" : "Smoke")) + "  " + std::to_string(_have) + "/" + std::to_string(_max);
		}

		SetText(_card.name, _name, 0.f);
		SetText(_card.price, _priceText, 1.f);
		const bool _affordable = _price <= _budget && !_full;
		_card.price->GetLabel()->SetFillColor(_owned || _full ? Color(140, 220, 150) : (_affordable ? Color(235, 235, 240) : Color(220, 90, 80)));
		_card.button->SetSelected(_owned || (_full && _item < ITEM_GRENADE));
		_card.button->idleColor = _affordable || _owned ? Color(26, 30, 42, 240) : Color(36, 20, 22, 240);
	}
}

void BlackoutGameMode::Command(const std::string& _command)
{
	if (!_command.starts_with("buy") || _command.size() < 4 || !shopOpen || buyLeft <= 0.f) return;
	int _item = -1;
	try { _item = std::stoi(_command.substr(3)); }
	catch (...) { return; }
	if (_item < 0 || _item >= SHOP_ITEMS) return;
	BlackoutHero* _hero = GetHero();
	if (!_hero) return;
	if (_item == ITEM_GRENADE && _hero->GetGrenades() >= BlackoutWeapon::MAX_GRENADES) return;
	if (_item == ITEM_FLASH && _hero->GetFlashes() >= BlackoutWeapon::MAX_FLASHES) return;
	if (_item == ITEM_SMOKE && _hero->GetSmokes() >= BlackoutWeapon::MAX_SMOKES) return;

	// The host checks the credits (it keeps them); alone, this machine is the host.
	NetworkManager* _net = GetWorld()->GetNetwork();
	if (_net->HasAuthority()) HostBuy(_net->GetLocalId(), _item);
	else _net->Send("BO_Buy", NetworkManager::Args(_item), NetTarget::Host);
}

void BlackoutGameMode::HostBuy(const int& _player, const int& _item)
{
	if (buyLeft <= 0.f || state != 0 || _item < 0 || _item >= SHOP_ITEMS) return;
	NetworkManager* _net = GetWorld()->GetNetwork();
	int _credits = Credits(_player);
	const int _price = ItemPrice(_item);
	BlackoutHero* _hero = versus ? FindHero(_player) : GetHero();

	if (_item < BlackoutWeapon::COUNT)
	{
		// The gun of the same kind is sold first.
		const int _slot = BlackoutWeapon::Get(_item).IsSidearm() ? 1 : 0;
		const std::string _key = (_slot == 0 ? "primary_" : "sidearm_") + std::to_string(_player);
		if (_net->GetShared<int>(_key, _slot == 0 ? -1 : BlackoutWeapon::PISTOL) == _item) return;
		const int _sell = SellValue(_player, _slot);
		if (_price > _credits + _sell) return;
		_credits += _sell - _price;
		_net->SetShared(_key, _item);
	}
	else
	{
		if (_price > _credits) return;
		if (_item == ITEM_LIGHT_SHIELD || _item == ITEM_HEAVY_SHIELD)
			if (!_hero || _hero->GetShield() >= (_item == ITEM_LIGHT_SHIELD ? BlackoutWeapon::LIGHT_SHIELD : BlackoutWeapon::HEAVY_SHIELD)) return;
		_credits -= _price;
	}

	_net->SetShared("credits_" + std::to_string(_player), std::min(MAX_CREDITS, _credits));
	ApplyBought(_player, _item);
	if (_net->IsOnline()) _net->Send("BO_Bought", NetworkManager::Args(_player, _item), NetTarget::Others);
	LOG(VerbosityType::Log, "Blackout: player " + std::to_string(_player) + " bought item " + std::to_string(_item));
}

void BlackoutGameMode::ApplyBought(const int& _player, const int& _item)
{
	// Every machine knows what was bought in this round (sold back at its full price).
	if (_item < BlackoutWeapon::COUNT)
	{
		const int _slot = BlackoutWeapon::Get(_item).IsSidearm() ? 1 : 0;
		if (_item != BlackoutWeapon::PISTOL) freshWeapon[_player * 2 + _slot] = _item;
		else freshWeapon.erase(_player * 2 + _slot);
	}

	BlackoutHero* _hero = versus ? FindHero(_player) : GetHero();
	if (!_hero) return;
	if (_item < BlackoutWeapon::COUNT) _hero->GiveWeapon(_item);		// every copy: its bullets and its picture are the same
	else if (_item == ITEM_LIGHT_SHIELD) _hero->SetShield(BlackoutWeapon::LIGHT_SHIELD);		// every copy: the same hits
	else if (_item == ITEM_HEAVY_SHIELD) _hero->SetShield(BlackoutWeapon::HEAVY_SHIELD);
	else if (!_hero->IsLocalPlayer()) return;
	else if (_item == ITEM_GRENADE) _hero->AddGrenade();
	else if (_item == ITEM_FLASH) _hero->AddFlash();
	else _hero->AddSmoke();
}

int BlackoutGameMode::ItemPrice(const int& _item)
{
	if (_item < BlackoutWeapon::COUNT) return BlackoutWeapon::Get(_item).price;
	switch (_item)
	{
	case ITEM_LIGHT_SHIELD:	return BlackoutWeapon::LIGHT_SHIELD_PRICE;
	case ITEM_HEAVY_SHIELD:	return BlackoutWeapon::HEAVY_SHIELD_PRICE;
	case ITEM_GRENADE:		return BlackoutWeapon::GRENADE_PRICE;
	case ITEM_FLASH:		return BlackoutWeapon::FLASH_PRICE;
	default:				return BlackoutWeapon::SMOKE_PRICE;
	}
}

void BlackoutGameMode::HostRoundRewards(const int& _winner)
{
	// Credits for everybody. A player still alive keeps his guns and what is left of his shield; a dead one starts with the pistol.
	NetworkManager* _net = GetWorld()->GetNetwork();
	for (const NetPlayer& _player : _net->GetPlayers())
	{
		const int _earned = TeamOf(_net, _player.id) == _winner ? WIN_CREDITS : LOSS_CREDITS;
		_net->SetShared("credits_" + std::to_string(_player.id), std::min(MAX_CREDITS, Credits(_player.id) + _earned));
		BlackoutHero* _hero = FindHero(_player.id);
		if (!_hero || _hero->IsDead()) ResetLoadout(_player.id);
		else _net->SetShared("shield_" + std::to_string(_player.id), static_cast<int>(std::ceil(_hero->GetShield())));
	}
}

BlackoutHero* BlackoutGameMode::GetHero()
{
	const std::vector<BlackoutHero*> _heroes = GetLevel()->GetActorManager().GetAllActorOfClass<BlackoutHero>();
	for (BlackoutHero* _hero : _heroes)
		if (_hero->IsLocalPlayer() && !_hero->IsPendingDestroy()) return _hero;
	return nullptr;
}

BlackoutHero* BlackoutGameMode::FindHero(const int& _playerId)
{
	for (BlackoutHero* _hero : GetLevel()->GetActorManager().GetAllActorOfClass<BlackoutHero>())
		if (_hero->GetPlayerId() == _playerId && !_hero->IsPendingDestroy()) return _hero;
	return nullptr;
}

void BlackoutGameMode::SendHit(const int& _playerId, const float& _damage, const FVector2& _from, const int& _attacker)
{
	GetWorld()->GetNetwork()->Send("BO_Hit", NetworkManager::Args(_playerId, _damage, _from, _attacker), NetTarget::All);
}

FVector2 BlackoutGameMode::RandomPoint(const float& _margin) const
{
	const FVector2 _half(std::max(0.f, arenaSize.x / 2.f - _margin), std::max(0.f, arenaSize.y / 2.f - _margin));
	return GetPosition() + FVector2(BlackoutRandom(-_half.x, _half.x), BlackoutRandom(-_half.y, _half.y));
}

FVector2 BlackoutGameMode::SpawnPoint(const int& _side, const int& _index)
{
	// The attackers in the south-west corner, the defenders in the north-east one, near the sites (scene Blackout).
	static const FVector2 ATTACK[] = { { -1350.f, 850.f }, { -1200.f, 950.f }, { -1450.f, 650.f }, { -1050.f, 820.f } };
	static const FVector2 DEFENSE[] = { { 1350.f, -850.f }, { 1200.f, -950.f }, { 1450.f, -650.f }, { 1050.f, -820.f } };
	const FVector2 _base = (_side == 0 ? ATTACK : DEFENSE)[_index % 4];
	const float _ring = static_cast<float>(_index / 4) * 70.f;	// more than 4 players: a little further
	return _base + FVector2(_side == 0 ? _ring : -_ring, 0.f);
}

TextWidget* BlackoutGameMode::MakeText(const FVector2& _position, const unsigned& _size)
{
	TextWidget* _text = GetLevel()->SpawnWidget<TextWidget>(TextData("", _size));
	_text->SetPosition(_position);
	_text->GetLabel()->SetOutlineColor(Color::Black());
	_text->GetLabel()->SetOutlineThickness(3.f);
	_text->GetLabel()->SetZOrder(static_cast<std::uint8_t>(60));	// over the pictures of the HUD (50)
	return _text;
}

void BlackoutGameMode::BeginPlay()
{
	Actor::BeginPlay();
	versus = GetWorld()->GetNetwork()->IsOnline();
	// No pause in Blackout: Escape is the game's (shop, menu), and a network game cannot stop for one player.
	if (PauseLayer* _pause = GetWorld()->GetLayerStack().Find<PauseLayer>()) _pause->SetEnabled(false);

	// The rings first: the texts are drawn over them.
	for (size_t _i = 0; _i < ECHO_COUNT; ++_i)
		echoes.push_back(EchoRing{ GetLevel()->SpawnWidget<BlackoutRingWidget>() });

	// The spread of the gun: a red fan from the muzzle (its points are set each frame, in HUD pixels around the widget).
	spreadCone = GetLevel()->SpawnWidget<ImageWidget>(RectangleShapeData(FVector2(10.f, 10.f), "Blackout/Solid"));
	spreadCone->GetImage()->GetShapeObject()->SetShapeType(ShapeType::Polygon);
	spreadCone->GetImage()->GetShapeObject()->GetShape()->SetFillColor(Color(255, 40, 40, 0));
	spreadCone->GetImage()->SetZOrder(static_cast<std::uint8_t>(20));

	// The charge carried: its picture over the carrier's head.
	chargeIcon = GetLevel()->SpawnWidget<ImageWidget>(RectangleShapeData(FVector2(34.f, 29.f), "Bomb"));
	chargeIcon->GetImage()->GetShapeObject()->GetShape()->SetFillColor(Color(0, 0, 0, 0));

	// The bar at the top: a dark band, the scores on both sides of the timer, a portrait per player.
	topPanel = GetLevel()->SpawnWidget<ImageWidget>(RectangleShapeData(FVector2(1180.f, 96.f), "Blackout/Solid"));
	topPanel->SetPosition(FVector2(960.f, 48.f));
	topPanel->GetImage()->GetShapeObject()->GetShape()->SetFillColor(Color(5, 7, 12, 165));
	topPanel->GetImage()->SetZOrder(static_cast<std::uint8_t>(40));

	hurtOverlay = GetLevel()->SpawnWidget<ImageWidget>(RectangleShapeData(HUD_SIZE, "Blackout/Solid"));
	hurtOverlay->SetPosition(HUD_SIZE / 2.f);
	hurtOverlay->GetImage()->GetShapeObject()->GetShape()->SetFillColor(Color(200, 0, 0, 0));

	healthBack = GetLevel()->SpawnWidget<ImageWidget>(RectangleShapeData(FVector2(408.f, 30.f), "Blackout/Solid"));
	healthBack->SetPosition(FVector2(60.f + 204.f, 1010.f));
	healthBack->GetImage()->GetShapeObject()->GetShape()->SetFillColor(Color(0, 0, 0, 170));
	healthBar = GetLevel()->SpawnWidget<ImageWidget>(RectangleShapeData(FVector2(400.f, 22.f), "Blackout/Solid"));
	healthBar->SetPosition(FVector2(64.f + 200.f, 1010.f));
	// The shield: a blue bar over the health (50 points = the whole width).
	shieldBar = GetLevel()->SpawnWidget<ImageWidget>(RectangleShapeData(FVector2(400.f, 8.f), "Blackout/Solid"));
	shieldBar->SetPosition(FVector2(64.f + 200.f, 986.f));
	shieldBar->GetImage()->GetShapeObject()->GetShape()->SetFillColor(Color(80, 150, 255, 0));

	// Planting / defusing: a bar under the middle of the screen.
	progressBack = GetLevel()->SpawnWidget<ImageWidget>(RectangleShapeData(FVector2(408.f, 22.f), "Blackout/Solid"));
	progressBack->SetPosition(FVector2(960.f, 660.f));
	progressBack->GetImage()->GetShapeObject()->GetShape()->SetFillColor(Color(0, 0, 0, 0));
	progressBar = GetLevel()->SpawnWidget<ImageWidget>(RectangleShapeData(FVector2(400.f, 14.f), "Blackout/Solid"));
	progressBar->SetPosition(FVector2(960.f, 660.f));
	progressBar->GetImage()->GetShapeObject()->GetShape()->SetFillColor(Color(0, 0, 0, 0));

	statusText = MakeText(FVector2(60.f, 40.f), 34u);
	ammoText = MakeText(FVector2(1860.f, 985.f), 40u);
	grenadeText = MakeText(FVector2(1860.f, 935.f), 30u);
	messageText = MakeText(FVector2(960.f, 430.f), 72u);
	timerText = MakeText(FVector2(960.f, 12.f), 44u);
	objectiveText = MakeText(FVector2(960.f, 104.f), 26u);
	roundText = MakeText(FVector2(960.f, 64.f), 18u);
	roundText->GetLabel()->SetOutlineThickness(2.f);
	scoreLeft = MakeText(FVector2(805.f, 14.f), 50u);
	scoreRight = MakeText(FVector2(1115.f, 14.f), 50u);
	pickupText = MakeText(FVector2(960.f, 700.f), 26u);
	pickupText->GetLabel()->SetFillColor(Color(235, 235, 245));

	// The gun in hand, seen from the side, over the ammo (on a grey plate: the guns are dark).
	ImageWidget* _plate = GetLevel()->SpawnWidget<ImageWidget>(RectangleShapeData(FVector2(240.f, 78.f), "Blackout/Solid"));
	_plate->SetPosition(FVector2(1860.f - 120.f, 830.f));
	_plate->GetImage()->GetShapeObject()->GetShape()->SetFillColor(Color(70, 76, 90, 150));
	_plate->GetImage()->SetZOrder(static_cast<std::uint8_t>(45));
	slotsText = MakeText(FVector2(1860.f, 754.f), 22u);
	slotsText->GetLabel()->SetOutlineThickness(2.f);
	weaponIcon = GetLevel()->SpawnWidget<ImageWidget>(RectangleShapeData(FVector2(100.f, 40.f), BlackoutWeapon::Get(0).icon));
	weaponIcon->SetPosition(FVector2(1750.f, 820.f));
	weaponIcon->GetImage()->SetZOrder(static_cast<std::uint8_t>(55));
	helpText = MakeText(FVector2(60.f, 940.f), 22u);
	{
		// The keys chosen in the menu (BlackoutKeys).
		const auto _key = [](const int& _action) { return BlackoutKeys::KeyName(_action); };
		SetText(helpText, _key(BlackoutKeys::MoveUp) + _key(BlackoutKeys::MoveLeft) + _key(BlackoutKeys::MoveDown) + _key(BlackoutKeys::MoveRight) +
			" courir   " + _key(BlackoutKeys::Walk) + " marcher   " + _key(BlackoutKeys::Reload) + " recharger   " +
			_key(BlackoutKeys::Primary) + "/" + _key(BlackoutKeys::Sidearm) + " armes   " + _key(BlackoutKeys::Grenade) + " grenade   " +
			_key(BlackoutKeys::Flash) + " flash   " + _key(BlackoutKeys::Smoke) + " smoke   " + _key(BlackoutKeys::Use) + (versus ? " poser/prendre   " : " prendre   ") +
			_key(BlackoutKeys::Shop) + " boutique   " + _key(BlackoutKeys::Ping) + "/molette ping", 0.f);
	}
	helpText->GetLabel()->SetFillColor(Color(200, 200, 210));
	creditsText = MakeText(FVector2(1860.f, 885.f), 28u);
	moveText = MakeText(FVector2(490.f, 996.f), 22u);

	// Shopping at the start of the round: the shop opens by itself. Alone, the credits and the gun start again each game.
	buyLeft = static_cast<float>(buyTime);
	if (!versus)
	{
		NetworkManager* _net = GetWorld()->GetNetwork();
		_net->SetShared("credits_" + std::to_string(_net->GetLocalId()), static_cast<int>(soloCredits));
		ResetLoadout(_net->GetLocalId());
		if (BlackoutHero* _hero = GetHero())
		{
			_hero->SetLoadout(-1, BlackoutWeapon::PISTOL);
			_hero->SetShield(0.f);
		}
	}
	BuildShop();
	if (buyLeft > 0.f) SetShopOpen(true);

	// The edge of the buy zone (a ring on the HUD: seen in the dark), and the white screen of a flash, over everything.
	buyZone = GetLevel()->SpawnActor<BlackoutBuyZone>();		// in the world: under the HUD and the shop
	blindOverlay = GetLevel()->SpawnWidget<ImageWidget>(RectangleShapeData(HUD_SIZE, "Blackout/Solid"));
	blindOverlay->SetPosition(HUD_SIZE / 2.f);
	blindOverlay->GetImage()->GetShapeObject()->GetShape()->SetFillColor(Color(255, 255, 255, 0));
	blindOverlay->GetImage()->SetZOrder(static_cast<std::uint8_t>(80));

	crosshair = GetLevel()->SpawnWidget<BlackoutRingWidget>();
	crosshair->GetRing()->SetRadius(14.f);
	crosshair->GetRing()->GetShapeObject()->GetShape()->SetOutlineThickness(2.f);
	crosshair->GetRing()->GetShapeObject()->GetShape()->SetOutlineColor(Color(255, 255, 255, 200));

	if (versus)
	{
		chat = GetLevel()->SpawnActor<BlackoutChat>();
		chat->Build(FVector2(60.f, 840.f), 820.f);
	}
	else SetMessage("Achete ton arme, puis nettoie la zone", Color(230, 230, 240));
	started = true;
}

void BlackoutGameMode::Tick(const Float& _deltaTime)
{
	Actor::Tick(_deltaTime);
	if (!started || leaving) return;

	const float _delta = static_cast<float>(_deltaTime);
	stateTime += _delta;

	// The buy phase (versus: the host's time, counted here between two of its states).
	if (buyLeft > 0.f)
	{
		buyLeft = std::max(0.f, buyLeft - _delta);
		if (buyLeft <= 0.f && shopOpen) SetShopOpen(false);
	}

	if (versus) TickVersus(_delta);
	else TickSolo();
	if (leaving) return;

	// B: the shop, during the buy phase only.
	const bool _focus = GetWorld()->GetWindowRef().HasFocus();
	const bool _chatTyping = chat && chat->IsTyping();
	const bool _shopKey = _focus && !_chatTyping && BlackoutKeys::Down(BlackoutKeys::Shop);
	if (_shopKey && !shopKeyHeld)
	{
		if (buyLeft > 0.f && state == 0) SetShopOpen(!shopOpen);
		else if (chat) chat->AddInfo("La boutique n'est ouverte qu'au debut de la manche", Color(200, 200, 210));
	}
	shopKeyHeld = _shopKey;

	UpdatePickup();
	UpdatePings(_delta);
	UpdateFog(_delta);
	UpdateEchoes(_delta);
	UpdateHud(_delta);
	UpdateShop();

	// Escape: closes the shop, else back to the menu (not the Escape that closes the chat).
	const bool _escape = _focus && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);
	const bool _chatBusy = chat && (chat->IsTyping() || chat->JustClosed());
	if (_escape && !escapeHeld && !_chatBusy)
	{
		if (shopOpen) SetShopOpen(false);
		else LeaveToMenu();
	}
	escapeHeld = _escape || _chatBusy;
}

void BlackoutGameMode::LeaveToMenu()
{
	leaving = true;
	GetWorld()->GetNetwork()->Disconnect();		// out of the session (and of the Steam lobby), whatever its state
	Scene::Open(*GetWorld(), menuScene);
}

// ---- Solo

void BlackoutGameMode::TickSolo()
{
	SetText(timerText, buyLeft > 0.f ? "ACHAT " + Clock(buyLeft) : std::string("SOLO"), 0.5f);
	timerText->GetLabel()->SetFillColor(Color(140, 220, 150));
	SetText(objectiveText, buyLeft > 0.f ? "B : boutique" : std::string(), 0.5f);
	objectiveText->GetLabel()->SetFillColor(Color(200, 200, 210));

	BlackoutHero* _hero = GetHero();
	if (state == 0)
	{
		int _left = 0;
		for (BlackoutEnemy* _enemy : GetLevel()->GetActorManager().GetAllActorOfClass<BlackoutEnemy>())
			if (!_enemy->IsDead() && !_enemy->IsPendingDestroy()) ++_left;

		if (_hero && _hero->IsDead())
		{
			state = 2;
			stateTime = 0.f;
			SetMessage("ELIMINE\nEntree pour rejouer", Color(255, 80, 70));
			LOG_MSG("Blackout: the hero is dead (" + std::to_string(kills) + " kills)");
		}
		else if (_left == 0)
		{
			state = 1;
			stateTime = 0.f;
			SetMessage("ZONE NETTOYEE\nEntree pour rejouer", Color(120, 255, 150));
			LOG_MSG("Blackout: every enemy is dead");
		}
		else if (stateTime > 3.f) SetMessage("", Color::White());
	}
	else if (stateTime > 1.f && GetWorld()->GetWindowRef().HasFocus() &&
		(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)))
	{
		leaving = true;
		Scene::Open(*GetWorld(), sceneName);
	}
}

// ---- Versus

void BlackoutGameMode::StartVersus()
{
	NetworkManager* _net = GetWorld()->GetNetwork();
	Level* _level = GetLevel();

	// The hero and the enemies of the solo game are not used.
	for (BlackoutHero* _hero : _level->GetActorManager().GetAllActorOfClass<BlackoutHero>()) _hero->Destroy();
	for (BlackoutEnemy* _enemy : _level->GetActorManager().GetAllActorOfClass<BlackoutEnemy>()) _enemy->Destroy();

	// The round: who attacks (the sides swap at SWAP_ROUND).
	round = std::max(1, _net->GetShared<int>("round", 1));
	attackers = AttackersOf(round);

	// One hero per player, made the same way on every machine: no need to wait for everybody to load the scene.
	localTeam = TeamOf(_net, _net->GetLocalId());
	int _count[2] = { 0, 0 };
	for (const NetPlayer& _player : _net->GetPlayers())
	{
		const int _team = TeamOf(_net, _player.id);
		const int _side = _team == attackers ? 0 : 1;
		BlackoutHero* _hero = _level->SpawnActor<BlackoutHero>();
		_hero->SetName(String("Hero " + std::to_string(_player.id)));
		_hero->SetPosition(SpawnPoint(_side, _count[_side]++));
		_hero->LookAt(GetPosition());		// toward the middle of the arena
		NetworkSyncComponent* _sync = _hero->AddComponent<NetworkSyncComponent>();
		_sync->AssignNetworkId(HERO_NETWORK_ID + _player.id, _player.id);
		_hero->SetupPlayer(_player.id, _team, _player.isLocal, _team == localTeam);
	}

	// The charge lies on the floor in front of the attackers' corner: one of them walks on it to take it (the same on every machine,
	// then the host's state is copied). Already "Dropped": no "it fell" message.
	charge = _level->SpawnActor<BlackoutCharge>();
	roundLeft = static_cast<float>(roundTime);
	chargeState = Dropped;
	SetChargeState(Dropped, -1, ChargeStartPoint(), -1, roundLeft, 0.f);

	// The letters of the sites on the HUD.
	for (BlackoutSite* _site : GetSites())
	{
		(void)_site;
		TextWidget* _marker = MakeText(FVector2(-1000.f, -1000.f), 40u);
		_marker->GetLabel()->SetFillColor(CHARGE_COLOR);
		siteMarkers.push_back(_marker);
	}

	// What the other machines tell.
	subscriptions.push_back(_net->Subscribe("BO_Shot", [this](const NetMessage& _message)
		{
			// player, muzzle, speed, range, then one direction per pellet
			BlackoutHero* _shooter = FindHero(_message.Int(0));
			if (!_shooter) return;
			for (size_t _i = 4; _i < _message.args.size(); ++_i)
			{
				BlackoutBullet* _bullet = GetLevel()->SpawnActor<BlackoutBullet>();
				_bullet->SetPosition(_message.Vector(1));
				_bullet->Launch(*_shooter, _message.Vector(_i), 0.f, false, _message.Float(2), _message.Float(3));
			}
			Shot(*_shooter, _message.Vector(1));
		}));
	subscriptions.push_back(_net->Subscribe("BO_Grenade", [this](const NetMessage& _message)
		{
			BlackoutHero* _thrower = FindHero(_message.Int(0));
			if (!_thrower) return;
			BlackoutGrenade* _grenade = GetLevel()->SpawnActor<BlackoutGrenade>();
			_grenade->SetPosition(_message.Vector(1));
			_grenade->Throw(*_thrower, _message.Vector(2), false, _message.Int(3));		// player, start, target, kind (frag, flash, smoke)
		}));
	subscriptions.push_back(_net->Subscribe("BO_Hit", [this](const NetMessage& _message)
		{
			// victim, damage, from, attacker (-1: the charge)
			BlackoutHero* _victim = FindHero(_message.Int(0));
			if (!_victim) return;
			const bool _wasAlive = !_victim->IsDead();
			_victim->TakeDamage(_message.Float(1), _message.Vector(2));

			// Killed: the host drops his gun (the primary, else the sidearm) on every machine; he will start the next round with the pistol.
			NetworkManager* _network = GetWorld()->GetNetwork();
			if (_network->IsHost() && _wasAlive && _victim->IsDead())
			{
				DropWeapon(_victim->GetPrimary() >= 0 ? _victim->GetPrimary() : _victim->GetSidearm(), _victim->GetPosition());
				ResetLoadout(_victim->GetPlayerId());
			}

			// A kill is paid to the attacker (by the host, who keeps the credits).
			const int _attacker = _message.Int(3, -1);
			if (_network->IsHost() && _wasAlive && _victim->IsDead() && _attacker >= 0 && TeamOf(_network, _attacker) != _victim->GetTeam())
				_network->SetShared("credits_" + std::to_string(_attacker), std::min(MAX_CREDITS, Credits(_attacker) + KILL_CREDITS));
		}));
	subscriptions.push_back(_net->Subscribe("BO_Step", [this](const NetMessage& _message)
		{
			// A player of the other team runs: an echo where he is.
			BlackoutHero* _runner = FindHero(_message.Int(0));
			if (_runner && _runner->GetTeam() != localTeam) Echo(_message.Vector(1), Color(255, 70, 70), 34.f);
		}));
	subscriptions.push_back(_net->Subscribe("BO_Ping", [this](const NetMessage& _message)
		{
			// player, team, position: only the team of the player sees it.
			if (_message.Int(1) == localTeam) ShowPing(_message.Int(0), _message.Vector(2));
		}));
	subscriptions.push_back(_net->Subscribe("BO_Slot", [this](const NetMessage& _message)
		{
			// player, slot: a player took his other gun (its picture, its bullets).
			if (BlackoutHero* _hero = FindHero(_message.Int(0))) _hero->Equip(_message.Int(1));
		}));
	subscriptions.push_back(_net->Subscribe("BO_Drop", [this](const NetMessage& _message)
		{
			ApplyDrop(_message.Int(0), _message.Int(1), _message.Vector(2), _message.Float(3));		// id, gun, position, angle
		}));
	subscriptions.push_back(_net->Subscribe("BO_Pickup", [this](const NetMessage& _message)
		{
			if (GetWorld()->GetNetwork()->IsHost()) HostPickup(_message.sender, _message.Int(0));
		}));
	subscriptions.push_back(_net->Subscribe("BO_Picked", [this](const NetMessage& _message)
		{
			ApplyPicked(_message.Int(0), _message.Int(1), _message.Int(2));		// id, player, gun
		}));
	subscriptions.push_back(_net->Subscribe("BO_Buy", [this](const NetMessage& _message)
		{
			if (GetWorld()->GetNetwork()->IsHost()) HostBuy(_message.sender, _message.Int(0));
		}));
	subscriptions.push_back(_net->Subscribe("BO_Bought", [this](const NetMessage& _message)
		{
			ApplyBought(_message.Int(0), _message.Int(1));
		}));
	subscriptions.push_back(_net->Subscribe("BO_Start", [this](const NetMessage& _message)
		{
			leaving = true;
			Scene::Open(*GetWorld(), _message.String(0));
		}));
	subscriptions.push_back(_net->Subscribe("BO_Charge", [this](const NetMessage& _message)
		{
			if (GetWorld()->GetNetwork()->IsHost()) return;		// the host made it
			SetChargeState(_message.Int(0), _message.Int(1), _message.Vector(2), _message.Int(3), _message.Float(4), _message.Float(5));
			const bool _wasBuying = buyLeft > 0.f;
			buyLeft = _message.Float(6);
			if (_wasBuying && buyLeft <= 0.f && shopOpen) SetShopOpen(false);
		}));
	subscriptions.push_back(_net->Subscribe("BO_Plant", [this](const NetMessage& _message)
		{
			if (GetWorld()->GetNetwork()->IsHost()) HostPlant(_message.sender, _message.Vector(0), _message.Int(1));
		}));
	subscriptions.push_back(_net->Subscribe("BO_Defuse", [this](const NetMessage& _message)
		{
			if (GetWorld()->GetNetwork()->IsHost()) HostDefuse(_message.sender);
		}));
	subscriptions.push_back(_net->Subscribe("BO_Noise", [this](const NetMessage& _message)
		{
			Echo(_message.Vector(0), CHARGE_COLOR, 60.f);		// somebody plants or defuses: everybody hears it
		}));
	subscriptions.push_back(_net->Subscribe("BO_RoundEnd", [this](const NetMessage& _message)
		{
			if (state != 0) return;
			state = 1;
			stateTime = 0.f;
			action = 0;
			// winner, reason, the match is over (the host counted the point before it sent this)
			const int _winner = _message.Int(0);
			matchOver = _message.Bool(2);
			NetworkManager* _network = GetWorld()->GetNetwork();
			if (_network->IsHost() && !matchOver) HostRoundRewards(_winner);
			if (shopOpen) SetShopOpen(false);
			const std::string _reason = _message.String(1);
			const std::string _score = "ALPHA " + std::to_string(_network->GetShared<int>("score_0", 0)) + " - " + std::to_string(_network->GetShared<int>("score_1", 0)) + " BRAVO";
			if (matchOver)
				SetMessage(std::string(_winner == localTeam ? "VICTOIRE DU MATCH" : "DEFAITE DU MATCH") + "\n" + _score, BlackoutHero::TeamColor(_winner));
			else if (_winner < 0) SetMessage("MANCHE NULLE\n" + _reason, Color(230, 230, 240));
			else SetMessage(std::string(_winner == localTeam ? "VICTOIRE" : "DEFAITE") + "\n" + _reason, BlackoutHero::TeamColor(_winner));
			if (chat) chat->AddInfo("Manche " + std::to_string(round) + " : " + (_winner < 0 ? std::string("nulle") : TeamName(_winner) + " gagne") + " (" + _reason + ")", Color(200, 200, 210));
			if (chat && matchOver) chat->AddInfo("Fin du match : " + _score + ". Retour au salon...", Color(240, 220, 120));
			else if (chat && round + 1 == SWAP_ROUND) chat->AddInfo("Prochaine manche : changement de cote !", Color(240, 220, 120));
			LOG_MSG("Blackout: round " + std::to_string(round) + " over, winner " + (_winner < 0 ? std::string("nobody") : TeamName(_winner)) + " (" + _reason + ")" + (matchOver ? ", match over" : ""));
		}));
	disconnectHandle = _net->onDisconnected.AddListener([this]()
		{
			connectionLost = true;
			stateTime = 0.f;
			SetMessage("Connexion perdue\n" + GetWorld()->GetNetwork()->GetDisconnectReason(), Color(255, 120, 90));
		});

	const std::string _swap = round == SWAP_ROUND ? "CHANGEMENT DE COTE\n" : "";
	SetMessage(_swap + "Manche " + std::to_string(round) + " - " + TeamName(localTeam) + (localTeam == attackers ? " attaque" : " defend"), BlackoutHero::TeamColor(localTeam));
	if (chat) chat->AddInfo("Manche " + std::to_string(round) + " : " + TeamName(localTeam) + (localTeam == attackers ? " attaque, posez la charge sur A ou B" : " defend A et B"), BlackoutHero::TeamColor(localTeam));
	if (chat && localTeam == attackers) chat->AddInfo("La charge est au sol devant votre base : marchez dessus pour la prendre", CHARGE_COLOR);
	LOG_MSG("Blackout: versus round " + std::to_string(round) + ", " + std::to_string(_net->GetPlayerCount()) + " players, local team " + TeamName(localTeam) + ", attackers " + TeamName(attackers));
}

void BlackoutGameMode::TickVersus(const float& _deltaTime)
{
	NetworkManager* _net = GetWorld()->GetNetwork();
	if (!heroesMade)
	{
		heroesMade = true;
		StartVersus();
		return;
	}

	if (connectionLost)
	{
		if (stateTime > 3.f) LeaveToMenu();
		return;
	}

	// A player who left: his hero falls.
	const std::vector<NetPlayer> _players = _net->GetPlayers();
	int _alive[2] = { 0, 0 };
	for (BlackoutHero* _hero : GetLevel()->GetActorManager().GetAllActorOfClass<BlackoutHero>())
	{
		if (_hero->IsPendingDestroy() || _hero->GetPlayerId() < 0) continue;
		const bool _present = std::any_of(_players.begin(), _players.end(), [&](const NetPlayer& _player) { return _player.id == _hero->GetPlayerId(); });
		if (!_present && !_hero->IsDead())
		{
			if (_net->IsHost()) DropWeapon(_hero->GetPrimary() >= 0 ? _hero->GetPrimary() : _hero->GetSidearm(), _hero->GetPosition());
			_hero->Abandon();
		}
		if (!_hero->IsDead()) ++_alive[std::clamp(_hero->GetTeam(), 0, 1)];
	}

	UpdateChargeHud(_deltaTime);
	if (state == 0)
	{
		UpdateLocalAction(_deltaTime);
		if (_net->IsHost()) HostRules(_deltaTime, _alive);
		if (stateTime > 4.f && stateTime < 4.2f) SetMessage("", Color::White());
	}
	else
	{
		action = 0;
		if (_net->IsHost() && !nextRoundSent && stateTime > (matchOver ? 2.f * NEXT_ROUND_TIME : NEXT_ROUND_TIME))
		{
			nextRoundSent = true;
			if (matchOver)
			{
				_net->Send("BO_Start", NetworkManager::Args(lobbyScene), NetTarget::All);		// back to the lobby
				return;
			}

			// The next round. The swap of sides: everybody starts again with the pistol and the credits of the first round.
			const int _next = round + 1;
			_net->SetShared("round", _next);
			if (_next == SWAP_ROUND)
				for (const NetPlayer& _player : _net->GetPlayers())
				{
					_net->SetShared("credits_" + std::to_string(_player.id), START_CREDITS);
					ResetLoadout(_player.id);
				}
			_net->Send("BO_Start", NetworkManager::Args(sceneName), NetTarget::All);
		}
	}
}

// ---- The charge

std::vector<BlackoutSite*> BlackoutGameMode::GetSites()
{
	std::vector<BlackoutSite*> _sites = GetLevel()->GetActorManager().GetAllActorOfClass<BlackoutSite>();
	std::sort(_sites.begin(), _sites.end(), [](BlackoutSite* _a, BlackoutSite* _b) { return _a->siteName < _b->siteName; });
	return _sites;
}

int BlackoutGameMode::SiteAt(const FVector2& _position)
{
	const std::vector<BlackoutSite*> _sites = GetSites();
	for (size_t _i = 0; _i < _sites.size(); ++_i)
		if (_sites[_i]->Contains(_position)) return static_cast<int>(_i);
	return -1;
}

std::string BlackoutGameMode::SiteName(const int& _site)
{
	const std::vector<BlackoutSite*> _sites = GetSites();
	return _site >= 0 && _site < static_cast<int>(_sites.size()) ? _sites[static_cast<size_t>(_site)]->siteName : std::string("?");
}

void BlackoutGameMode::SetChargeState(const int& _state, const int& _carrier, const FVector2& _position, const int& _site, const float& _roundLeft, const float& _detonateLeft)
{
	const int _old = chargeState;
	chargeState = _state;
	carrierId = _carrier;
	chargePosition = _position;
	chargeSite = _site;
	roundLeft = _roundLeft;
	detonateLeft = _detonateLeft;
	if (_old == _state) return;

	// What everybody sees when it changes.
	NetworkManager* _net = GetWorld()->GetNetwork();
	if (_state == Planted)
	{
		if (chat) chat->AddInfo("La charge est posee en " + SiteName(_site) + " !", CHARGE_COLOR);
		Echo(_position, CHARGE_COLOR, 160.f);
	}
	else if (_state == Defused)
	{
		if (chat) chat->AddInfo("La charge est desamorcee", BlackoutHero::TeamColor(1 - attackers));
	}
	else if (_state == Exploded)
	{
		BlackoutEffect(GetLevel(), ParticlePreset::Explosion, _position);
		BlackoutEffect(GetLevel(), ParticlePreset::Smoke, _position);
		StainComponent::StainAt(GetLevel(), _position, 200.f, Color(15, 12, 10, 225), StainStyle::Scorch);
		Echo(_position, Color(255, 120, 60), 400.f);
		hurtTime = 0.35f;
	}
	else if (_state == Carried && _carrier == _net->GetLocalId())
	{
		if (chat) chat->AddInfo("Tu portes la charge", CHARGE_COLOR);
	}
	else if (_state == Dropped && localTeam == attackers && _old == Carried)
	{
		if (chat) chat->AddInfo("La charge est tombee", CHARGE_COLOR);
	}
}

void BlackoutGameMode::BroadcastCharge()
{
	GetWorld()->GetNetwork()->Send("BO_Charge", NetworkManager::Args(chargeState, carrierId, chargePosition, chargeSite, roundLeft, detonateLeft, buyLeft), NetTarget::Others);
	syncTimer = 0.f;
}

void BlackoutGameMode::UpdateLocalAction(const float& _deltaTime)
{
	// Hold E: the carrier plants on a site, a defender defuses near the planted charge. Moving stops it (the hero stands still while E is down).
	NetworkManager* _net = GetWorld()->GetNetwork();
	BlackoutHero* _hero = GetHero();
	const bool _key = GetWorld()->GetWindowRef().HasFocus() && BlackoutKeys::Down(BlackoutKeys::Use) && !(chat && chat->IsTyping()) &&
		buyLeft <= 0.f && !shopOpen;

	int _wanted = 0;
	int _site = -1;
	if (_hero && !_hero->IsDead() && _key)
	{
		if (chargeState == Carried && carrierId == _net->GetLocalId() && _hero->GetTeam() == attackers)
		{
			_site = SiteAt(_hero->GetPosition());
			if (_site >= 0) _wanted = 1;
		}
		else if (chargeState == Planted && _hero->GetTeam() != attackers && (_hero->GetPosition() - chargePosition).Length() < DEFUSE_RADIUS)
			_wanted = 2;
	}

	if (_wanted != action)
	{
		action = _wanted;
		actionTime = 0.f;
		actionHeard = false;
	}
	if (action == 0) return;

	actionTime += _deltaTime;
	if (!actionHeard)
	{
		actionHeard = true;
		_net->Send("BO_Noise", NetworkManager::Args(_hero->GetPosition()), NetTarget::All);
	}

	if (action == 1 && actionTime >= static_cast<float>(plantTime))
	{
		if (_net->IsHost()) HostPlant(_net->GetLocalId(), _hero->GetPosition(), _site);
		else _net->Send("BO_Plant", NetworkManager::Args(_hero->GetPosition(), _site), NetTarget::Host);
		action = 0;
	}
	else if (action == 2 && actionTime >= static_cast<float>(defuseTime))
	{
		if (_net->IsHost()) HostDefuse(_net->GetLocalId());
		else _net->Send("BO_Defuse", {}, NetTarget::Host);
		action = 0;
	}
}

void BlackoutGameMode::HostPlant(const int& _player, const FVector2& _position, const int& _site)
{
	BlackoutHero* _hero = FindHero(_player);
	if (state != 0 || chargeState != Carried || carrierId != _player || !_hero || _hero->IsDead() || SiteAt(_position) < 0) return;
	SetChargeState(Planted, -1, _position, SiteAt(_position), roundLeft, static_cast<float>(detonateTime));
	BroadcastCharge();
	LOG_MSG("Blackout: charge planted on " + SiteName(_site) + " by player " + std::to_string(_player));
}

void BlackoutGameMode::HostDefuse(const int& _player)
{
	BlackoutHero* _hero = FindHero(_player);
	if (state != 0 || chargeState != Planted || !_hero || _hero->IsDead() || _hero->GetTeam() == attackers) return;
	if ((_hero->GetPosition() - chargePosition).Length() > DEFUSE_RADIUS * 1.5f) return;
	SetChargeState(Defused, -1, chargePosition, chargeSite, roundLeft, detonateLeft);
	BroadcastCharge();
	EndRound(1 - attackers, "Charge desamorcee");
}

void BlackoutGameMode::EndRound(const int& _winner, const std::string& _reason)
{
	if (roundEndSent) return;		// once: the message comes back a frame later
	roundEndSent = true;

	// The point first (the clients get the new score before the message), then: is the match over?
	NetworkManager* _net = GetWorld()->GetNetwork();
	bool _over = false;
	if (_winner >= 0)
	{
		const int _points = _net->GetShared<int>("score_" + std::to_string(_winner), 0) + 1;
		_net->SetShared("score_" + std::to_string(_winner), _points);
		_over = _points >= ROUNDS_TO_WIN;
	}
	_net->Send("BO_RoundEnd", NetworkManager::Args(_winner, _reason, _over), NetTarget::All);
}

void BlackoutGameMode::HostRules(const float& _deltaTime, const int* _alive)
{
	// Shopping: nothing happens yet (buyLeft is counted by Tick).
	syncTimer += _deltaTime;
	if (buyLeft > 0.f)
	{
		if (syncTimer >= 0.25f) BroadcastCharge();
		return;
	}

	bool _changed = false;
	if (chargeState == Planted) detonateLeft -= _deltaTime;
	else roundLeft -= _deltaTime;

	// Who has the charge.
	BlackoutHero* _carrier = FindHero(carrierId);
	if (chargeState == Carried)
	{
		if (!_carrier || _carrier->IsDead())
		{
			SetChargeState(Dropped, -1, _carrier ? _carrier->GetPosition() : chargePosition, -1, roundLeft, detonateLeft);
			_changed = true;
		}
		else chargePosition = _carrier->GetPosition();
	}
	else if (chargeState == Dropped)
	{
		for (BlackoutHero* _hero : GetLevel()->GetActorManager().GetAllActorOfClass<BlackoutHero>())
			if (_hero->GetTeam() == attackers && !_hero->IsDead() && !_hero->IsPendingDestroy() && (_hero->GetPosition() - chargePosition).Length() < PICKUP_RADIUS)
			{
				SetChargeState(Carried, _hero->GetPlayerId(), _hero->GetPosition(), -1, roundLeft, detonateLeft);
				_changed = true;
				break;
			}
	}
	else if (chargeState == Planted && detonateLeft <= 0.f)
	{
		SetChargeState(Exploded, -1, chargePosition, chargeSite, roundLeft, 0.f);
		BroadcastCharge();
		for (BlackoutHero* _hero : GetLevel()->GetActorManager().GetAllActorOfClass<BlackoutHero>())
			if (!_hero->IsDead() && (_hero->GetPosition() - chargePosition).Length() < BLAST_RADIUS)
				SendHit(_hero->GetPlayerId(), 1000.f, chargePosition);
		EndRound(attackers, "La charge a explose en " + SiteName(chargeSite));
		return;
	}

	// The end of the round.
	if (_alive[1 - attackers] == 0) EndRound(attackers, "Defenseurs elimines");
	else if (_alive[attackers] == 0 && chargeState != Planted) EndRound(1 - attackers, "Attaquants elimines");
	else if (roundLeft <= 0.f && chargeState != Planted) EndRound(1 - attackers, "Temps ecoule");

	if (_changed || syncTimer >= 0.25f) BroadcastCharge();
}

void BlackoutGameMode::UpdateChargeHud(const float& _deltaTime)
{
	NetworkManager* _net = GetWorld()->GetNetwork();

	// The others count down between two states of the host.
	if (!_net->IsHost() && state == 0)
	{
		if (chargeState == Planted) detonateLeft = std::max(0.f, detonateLeft - _deltaTime);
		else roundLeft = std::max(0.f, roundLeft - _deltaTime);
	}

	// The charge where it lies; planted, it beeps faster and faster (an echo everybody sees).
	if (chargeState == Carried)
		if (BlackoutHero* _carrier = FindHero(carrierId)) chargePosition = _carrier->GetPosition();
	const bool _onFloor = chargeState == Dropped || chargeState == Planted || chargeState == Defused;
	if (charge)
	{
		charge->Show(_onFloor, chargePosition);
		const float _period = 0.2f + 0.8f * std::clamp(detonateLeft / std::max(1.f, static_cast<float>(detonateTime)), 0.f, 1.f);
		charge->SetBlink(chargeState == Planted ? _period : 0.f);
		if (chargeState == Planted && state == 0)
		{
			beepTimer -= _deltaTime;
			if (beepTimer <= 0.f)
			{
				beepTimer = _period;
				Echo(chargePosition, Color(255, 80, 60), 50.f);
			}
		}
	}

	// Timer and what to do.
	if (buyLeft > 0.f)
	{
		SetText(timerText, "ACHAT " + Clock(buyLeft), 0.5f);
		timerText->GetLabel()->SetFillColor(Color(140, 220, 150));
		SetText(objectiveText, "Phase d'achat : B pour la boutique", 0.5f);
		objectiveText->GetLabel()->SetFillColor(Color(200, 200, 210));
		return;
	}
	if (chargeState == Planted)
	{
		SetText(timerText, "CHARGE " + Clock(detonateLeft), 0.5f);
		timerText->GetLabel()->SetFillColor(Color(255, 90, 70));
	}
	else
	{
		SetText(timerText, Clock(roundLeft), 0.5f);
		timerText->GetLabel()->SetFillColor(Color(235, 235, 240));
	}

	std::string _objective;
	const bool _attacker = localTeam == attackers;
	switch (chargeState)
	{
	case Carried:
		if (carrierId == _net->GetLocalId()) _objective = "Tu portes la charge : maintiens E sur le site A ou B";
		else _objective = _attacker ? _net->GetPlayerName(carrierId) + " porte la charge" : "Defends les sites A et B";
		break;
	case Dropped:	_objective = _attacker ? "La charge est au sol : ramasse-la" : "La charge est au sol"; break;
	case Planted:	_objective = std::string("Charge posee en ") + SiteName(chargeSite) + (_attacker ? " : protege-la" : " : maintiens E dessus pour la desamorcer"); break;
	case Defused:	_objective = "Charge desamorcee"; break;
	default:		_objective = "La charge a explose"; break;
	}
	SetText(objectiveText, _objective, 0.5f);
	objectiveText->GetLabel()->SetFillColor(CHARGE_COLOR);

	// The bar while planting / defusing.
	const float _need = action == 1 ? static_cast<float>(plantTime) : static_cast<float>(defuseTime);
	const float _ratio = action == 0 ? 0.f : std::clamp(actionTime / _need, 0.f, 1.f);
	progressBack->GetImage()->GetShapeObject()->GetShape()->SetFillColor(action == 0 ? Color(0, 0, 0, 0) : Color(0, 0, 0, 180));
	progressBar->GetImage()->SetSize(FVector2(std::max(1.f, 400.f * _ratio), 14.f));
	progressBar->SetPosition(FVector2(760.f + 200.f * _ratio, 660.f));
	progressBar->GetImage()->GetShapeObject()->GetShape()->SetFillColor(action == 0 ? Color(0, 0, 0, 0) : CHARGE_COLOR);

	// The letters of the sites, kept on the edge of the screen when they are out of it.
	const std::vector<BlackoutSite*> _sites = GetSites();
	for (size_t _i = 0; _i < siteMarkers.size() && _i < _sites.size(); ++_i)
	{
		FVector2 _hud = WorldToHud(_sites[_i]->GetPosition());
		_hud.x = std::clamp(_hud.x, 60.f, HUD_SIZE.x - 60.f);
		_hud.y = std::clamp(_hud.y, 150.f, HUD_SIZE.y - 220.f);
		SetText(siteMarkers[_i], _sites[_i]->siteName, 0.5f);
		siteMarkers[_i]->SetPosition(_hud);
	}
}

// ---- HUD

FVector2 BlackoutGameMode::WorldToHud(const FVector2& _position)
{
	Window& _window = GetWorld()->GetWindowRef();
	CameraComponent* _camera = GetLevel()->GetDefaultCamera();
	if (!_camera || !_camera->GetView()) return FVector2(-1000.f, -1000.f);
	const IVector2 _pixel = _window.MapCoordsToPixel(_position, *_camera->GetView());
	return _window.MapPixelToCoords(_pixel, GetLevel()->GetHUD().GetView());
}

void BlackoutGameMode::Echo(const FVector2& _position, const Color& _color, const float& _size)
{
	if (echoes.empty()) return;
	EchoRing& _echo = echoes[nextEcho];
	nextEcho = (nextEcho + 1) % echoes.size();
	_echo.position = _position;
	_echo.color = _color;
	_echo.size = _size;
	_echo.age = 0.f;
}

void BlackoutGameMode::UpdateEchoes(const float& _deltaTime)
{
	for (EchoRing& _echo : echoes)
	{
		Shape* _shape = _echo.widget->GetRing()->GetShapeObject()->GetShape();
		if (_echo.age >= 1.f)
		{
			_shape->SetOutlineColor(Color(0, 0, 0, 0));
			continue;
		}
		_echo.age = std::min(1.f, _echo.age + _deltaTime / ECHO_DURATION);

		// It grows and fades; it follows the world (the camera moves with the hero).
		_echo.widget->GetRing()->SetRadius(_echo.size * (0.35f + 0.65f * _echo.age));
		_echo.widget->SetPosition(WorldToHud(_echo.position));
		Color _color = _echo.color;
		_color.a = static_cast<std::uint8_t>(230.f * (1.f - _echo.age));
		_shape->SetOutlineColor(_color);
	}
}

void BlackoutGameMode::UpdateHud(const float& _deltaTime)
{
	BlackoutHero* _hero = GetHero();

	UpdateTopBar();
	SetText(statusText, "", 0.f);		// the bar at the top tells the scores and who is alive

	if (_hero)
	{
		const float _ratio = std::clamp(_hero->GetHealth() / std::max(1.f, static_cast<float>(_hero->maxHealth)), 0.f, 1.f);
		healthBar->GetImage()->SetSize(FVector2(400.f * _ratio, 22.f));
		healthBar->SetPosition(FVector2(64.f + 200.f * _ratio, 1010.f));
		healthBar->GetImage()->GetShapeObject()->GetShape()->SetFillColor(_ratio > 0.35f ? Color(90, 220, 120) : Color(240, 70, 60));
		const float _shieldRatio = std::clamp(_hero->GetShield() / BlackoutWeapon::HEAVY_SHIELD, 0.f, 1.f);
		shieldBar->GetImage()->SetSize(FVector2(std::max(1.f, 400.f * _shieldRatio), 8.f));
		shieldBar->SetPosition(FVector2(64.f + 200.f * _shieldRatio, 986.f));
		shieldBar->GetImage()->GetShapeObject()->GetShape()->SetFillColor(Color(80, 150, 255, _shieldRatio > 0.f ? 255 : 0));

		ShowWeaponIcon(weaponIcon, _hero->GetWeapon(), FVector2(200.f, 60.f));
		weaponIcon->SetPosition(FVector2(1860.f - weaponIcon->GetImage()->GetSize().x / 2.f, 830.f));
		if (_hero->IsDead()) weaponIcon->GetImage()->GetShapeObject()->GetShape()->SetFillColor(Color(0, 0, 0, 0));
		// The two guns: 1 primary, 2 sidearm (the one in hand between brackets).
		const std::string _primaryName = _hero->GetPrimary() >= 0 ? BlackoutWeapon::Get(_hero->GetPrimary()).name : std::string("-");
		const std::string _sidearmName = BlackoutWeapon::Get(_hero->GetSidearm()).name;
		SetText(slotsText, shopOpen ? std::string() : (_hero->GetSlot() == 0 ? "[1 " + _primaryName + "]" : "1 " + _primaryName) + "     " +
			(_hero->GetSlot() == 1 ? "[2 " + _sidearmName + "]" : "2 " + _sidearmName), 1.f);
		slotsText->GetLabel()->SetFillColor(Color(200, 205, 215));
		const std::string _weapon = BlackoutWeapon::Get(_hero->GetWeapon()).name;
		SetText(ammoText, _hero->IsReloading() ? "Recharge..." : _weapon + "   " + std::to_string(_hero->GetAmmo()) + " / " + std::to_string(_hero->GetMagazine()), 1.f);
		SetText(grenadeText, "Grenades : " + std::to_string(_hero->GetGrenades()) + "    Flash : " + std::to_string(_hero->GetFlashes()) +
			"    Smoke : " + std::to_string(_hero->GetSmokes()), 1.f);
		grenadeText->GetLabel()->SetFillColor(_hero->GetGrenades() + _hero->GetFlashes() + _hero->GetSmokes() > 0 ? Color(170, 220, 140) : Color(130, 130, 140));
		SetText(creditsText, "Credits : " + std::to_string(Credits(GetWorld()->GetNetwork()->GetLocalId())), 1.f);
		creditsText->GetLabel()->SetFillColor(Color(140, 220, 150));
		SetText(moveText, _hero->IsWalking() ? "Marche : silencieux" : "", 0.f);
		moveText->GetLabel()->SetFillColor(Color(160, 200, 255));
		ammoText->GetLabel()->SetFillColor(_hero->GetAmmo() == 0 || _hero->IsReloading() ? Color(255, 170, 80) : Color::White());
	}

	// The red flash of a hit.
	hurtTime = std::max(0.f, hurtTime - _deltaTime);
	hurtOverlay->GetImage()->GetShapeObject()->GetShape()->SetFillColor(Color(200, 0, 0, static_cast<std::uint8_t>(110.f * hurtTime / 0.35f)));

	// Blinded by a flash: all white, then it fades during the last second.
	blindLeft = std::max(0.f, blindLeft - _deltaTime);
	blindOverlay->GetImage()->GetShapeObject()->GetShape()->SetFillColor(Color(255, 255, 255, static_cast<std::uint8_t>(250.f * std::clamp(blindLeft, 0.f, 1.f))));

	// The buy zone: a circle on the floor around where the hero started, during the buy phase.
	const bool _buying = _hero && buyLeft > 0.f && !_hero->IsDead();
	buyZone->Show(_buying, _hero ? _hero->GetSpawnPosition() : FVector2(), static_cast<float>(buyZoneRadius));

	// The crosshair on the mouse.
	Window& _window = GetWorld()->GetWindowRef();
	crosshair->SetPosition(_window.MapPixelToCoords(_window.GetMousePosition(), GetLevel()->GetHUD().GetView()));

	UpdateSpreadCone();
	UpdateNameTags();
}

void BlackoutGameMode::UpdateTopBar()
{
	// Like a tactical shooter: our score on the left of the timer, theirs on the right, and a portrait per player on each side
	// (dark once dead). Alone: the hero on the left, the enemies on the right.
	NetworkManager* _net = GetWorld()->GetNetwork();
	const int _otherTeam = 1 - localTeam;
	std::vector<BlackoutCharacter*> _sides[2];		// 0: ours, 1: theirs
	if (versus)
	{
		std::vector<BlackoutHero*> _heroes = GetLevel()->GetActorManager().GetAllActorOfClass<BlackoutHero>();
		std::sort(_heroes.begin(), _heroes.end(), [](BlackoutHero* _a, BlackoutHero* _b) { return _a->GetPlayerId() < _b->GetPlayerId(); });
		for (BlackoutHero* _hero : _heroes)
			if (_hero->GetPlayerId() >= 0 && !_hero->IsPendingDestroy()) _sides[_hero->GetTeam() == localTeam ? 0 : 1].push_back(_hero);
	}
	else
	{
		if (BlackoutHero* _hero = GetHero()) _sides[0].push_back(_hero);
		for (BlackoutEnemy* _enemy : GetLevel()->GetActorManager().GetAllActorOfClass<BlackoutEnemy>()) _sides[1].push_back(_enemy);
	}

	// The scores (versus), the round and our side.
	if (versus)
	{
		SetText(scoreLeft, std::to_string(_net->GetShared<int>("score_" + std::to_string(localTeam), 0)), 0.5f);
		SetText(scoreRight, std::to_string(_net->GetShared<int>("score_" + std::to_string(_otherTeam), 0)), 0.5f);
		SetText(roundText, "MANCHE " + std::to_string(round) + "   " + TeamName(localTeam) + (localTeam == attackers ? " ATTAQUE" : " DEFEND") +
			"   PREMIER A " + std::to_string(ROUNDS_TO_WIN), 0.5f);
	}
	else
	{
		int _left = 0;
		for (BlackoutCharacter* _enemy : _sides[1]) if (!_enemy->IsDead() && !_enemy->IsPendingDestroy()) ++_left;
		SetText(scoreLeft, std::to_string(kills), 0.5f);
		SetText(scoreRight, std::to_string(_left), 0.5f);
		SetText(roundText, "ELIMINATIONS        ENNEMIS", 0.5f);
	}
	scoreLeft->GetLabel()->SetFillColor(versus ? BlackoutHero::TeamColor(localTeam) : ALLY_COLOR);
	scoreRight->GetLabel()->SetFillColor(versus ? BlackoutHero::TeamColor(_otherTeam) : ENEMY_COLOR);
	roundText->GetLabel()->SetFillColor(Color(200, 205, 215));

	// The portraits: Player.png in the team color, grey and dim once dead; the local player has a white frame.
	size_t _used = 0;
	BlackoutHero* _local = GetHero();
	for (int _side = 0; _side < 2; ++_side)
		for (size_t _i = 0; _i < _sides[_side].size(); ++_i)
		{
			BlackoutCharacter* _character = _sides[_side][_i];
			if (_used == portraits.size())
			{
				ImageWidget* _portrait = GetLevel()->SpawnWidget<ImageWidget>(RectangleShapeData(PORTRAIT_SIZE, "Player"));
				_portrait->GetImage()->SetZOrder(static_cast<std::uint8_t>(55));
				_portrait->GetImage()->GetShapeObject()->GetShape()->SetOutlineThickness(2.f);
				portraits.push_back(_portrait);
			}
			ImageWidget* _portrait = portraits[_used++];
			const float _x = 960.f + (_side == 0 ? -1.f : 1.f) * (225.f + PORTRAIT_STEP * static_cast<float>(_i));
			_portrait->SetPosition(FVector2(_x, 44.f));
			const bool _dead = _character->IsDead() || _character->IsPendingDestroy();
			const Color _team = versus ? BlackoutHero::TeamColor(_side == 0 ? localTeam : _otherTeam) : (_side == 0 ? ALLY_COLOR : ENEMY_COLOR);
			Shape* _shape = _portrait->GetImage()->GetShapeObject()->GetShape();
			_shape->SetFillColor(_dead ? Color(70, 70, 75, 150) : _team);
			_shape->SetOutlineColor(_dead ? Color(60, 60, 65, 200) : (_character == _local ? Color::White() : _team));
		}
	for (size_t _i = _used; _i < portraits.size(); ++_i)
	{
		portraits[_i]->GetImage()->GetShapeObject()->GetShape()->SetFillColor(Color(0, 0, 0, 0));
		portraits[_i]->GetImage()->GetShapeObject()->GetShape()->SetOutlineColor(Color(0, 0, 0, 0));
	}

	// The band behind, as wide as the portraits.
	const size_t _most = std::max<size_t>(std::max(_sides[0].size(), _sides[1].size()), 1);
	topPanel->GetImage()->SetSize(FVector2(2.f * (225.f + PORTRAIT_STEP * static_cast<float>(_most)) + 20.f, 96.f));
}

FVector2 BlackoutGameMode::MouseInWorld()
{
	Window& _window = GetWorld()->GetWindowRef();
	CameraComponent* _camera = GetLevel()->GetDefaultCamera();
	if (!_camera || !_camera->GetView()) return GetPosition();
	return _window.MapPixelToCoords(_window.GetMousePosition(), *_camera->GetView());
}

void BlackoutGameMode::UpdatePings(const float& _deltaTime)
{
	// The ping key or the middle click: a mark where the mouse is, for the team (not while typing or shopping).
	pingCooldown = std::max(0.f, pingCooldown - _deltaTime);
	const bool _key = GetWorld()->GetWindowRef().HasFocus() && !(chat && chat->IsTyping()) && !shopOpen &&
		(BlackoutKeys::Down(BlackoutKeys::Ping) || sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle));
	if (_key && !pingHeld && pingCooldown <= 0.f && state == 0)
	{
		pingCooldown = 0.4f;
		NetworkManager* _net = GetWorld()->GetNetwork();
		const FVector2 _position = MouseInWorld();
		ShowPing(_net->GetLocalId(), _position);
		if (versus) _net->Send("BO_Ping", NetworkManager::Args(_net->GetLocalId(), localTeam, _position), NetTarget::Others);
	}
	pingHeld = _key;

	// The marks: on the HUD (seen in the dark), kept on the edge of the screen when out of it, with who and how far.
	BlackoutHero* _hero = GetHero();
	for (PingMark& _ping : pings)
	{
		_ping.age += _deltaTime;
		Shape* _shape = _ping.mark->GetImage()->GetShapeObject()->GetShape();
		if (_ping.age >= PING_TIME)
		{
			_shape->SetFillColor(Color(0, 0, 0, 0));
			_shape->SetOutlineColor(Color(0, 0, 0, 0));
			SetText(_ping.text, "", 0.5f);
			continue;
		}
		const float _alpha = std::clamp(PING_TIME - _ping.age, 0.f, 1.f);
		FVector2 _hud = WorldToHud(_ping.position);
		_hud.x = std::clamp(_hud.x, 40.f, HUD_SIZE.x - 40.f);
		_hud.y = std::clamp(_hud.y, 150.f, HUD_SIZE.y - 200.f);
		const float _pop = 1.f + 0.6f * std::max(0.f, 1.f - _ping.age / 0.35f);		// it pops when placed
		_ping.mark->SetPosition(_hud);
		_ping.mark->SetScale(FVector2(_pop, _pop));
		_shape->SetFillColor(Color(PING_COLOR.r, PING_COLOR.g, PING_COLOR.b, static_cast<std::uint8_t>(200.f * _alpha)));
		_shape->SetOutlineColor(Color(10, 20, 30, static_cast<std::uint8_t>(230.f * _alpha)));

		const int _meters = _hero ? static_cast<int>((_ping.position - _hero->GetPosition()).Length() / 50.f) : 0;
		SetText(_ping.text, _ping.name + "  " + std::to_string(_meters) + " m", 0.5f);
		_ping.text->SetPosition(_hud - FVector2(0.f, 46.f));
		_ping.text->GetLabel()->SetFillColor(Color(PING_COLOR.r, PING_COLOR.g, PING_COLOR.b, static_cast<std::uint8_t>(255.f * _alpha)));
		_ping.text->GetLabel()->SetOutlineColor(Color(0, 0, 0, static_cast<std::uint8_t>(255.f * _alpha)));
	}
}

void BlackoutGameMode::ShowPing(const int& _player, const FVector2& _position)
{
	auto _found = std::find_if(pings.begin(), pings.end(), [&](const PingMark& _ping) { return _ping.player == _player; });
	if (_found == pings.end())
	{
		PingMark _ping;
		_ping.player = _player;
		_ping.mark = GetLevel()->SpawnWidget<ImageWidget>(RectangleShapeData(FVector2(22.f, 22.f), "Blackout/Solid"));
		_ping.mark->SetRotation(Angle::FromDegrees(45.f));		// a diamond
		_ping.mark->GetImage()->GetShapeObject()->GetShape()->SetOutlineThickness(2.f);
		_ping.mark->GetImage()->SetZOrder(static_cast<std::uint8_t>(58));
		_ping.text = MakeText(FVector2(-1000.f, -1000.f), 22u);
		_ping.text->GetLabel()->SetOutlineThickness(2.f);
		pings.push_back(_ping);
		_found = pings.end() - 1;
	}
	NetworkManager* _net = GetWorld()->GetNetwork();
	_found->position = _position;
	_found->age = 0.f;
	_found->name = !versus || _player == _net->GetLocalId() ? std::string("Toi") : _net->GetPlayerName(_player);
	Echo(_position, PING_COLOR, 70.f);		// a ring where it is
	LOG(VerbosityType::Log, "Blackout: ping of player " + std::to_string(_player));
}

void BlackoutGameMode::UpdateFog(const float& _deltaTime)
{
	// The level is not completely black: an enemy standing in the dark (behind a wall) would still be seen. He is drawn only while
	// one of us sees him (close to us, or in a flashlight, nothing in between: InSight); he fades in and out quickly. The dead stay.
	std::vector<BlackoutHero*> _viewers;
	for (BlackoutHero* _hero : GetLevel()->GetActorManager().GetAllActorOfClass<BlackoutHero>())
		if (!_hero->IsDead() && !_hero->IsPendingDestroy() && _hero->GetTeam() == localTeam && (_hero == GetHero() || _hero->GetPlayerId() >= 0))
			_viewers.push_back(_hero);

	// Our bullets light a little around them: an enemy close to one (no wall, no smoke in between) is seen while it passes.
	std::vector<FVector2> _bulletLights;
	for (BlackoutBullet* _bullet : GetLevel()->GetActorManager().GetAllActorOfClass<BlackoutBullet>())
		if (!_bullet->IsPendingDestroy() && _bullet->GetTeam() == localTeam) _bulletLights.push_back(_bullet->GetPosition());
	const auto _litByBullet = [&](BlackoutCharacter& _target)
	{
		for (const FVector2& _light : _bulletLights)
		{
			if ((_target.GetPosition() - _light).Length() > BlackoutBullet::LIGHT_RADIUS) continue;
			if (BlackoutSmoke::Blocks(GetLevel(), _light, _target.GetPosition())) continue;
			bool _wall = false;
			for (const RaycastHitInfo& _hit : GetLevel()->GetWorldPhysics().RaycastAll(_light, _target.GetPosition()))
				if (_hit.hit && _hit.actor && !Cast<BlackoutCharacter>(_hit.actor)) { _wall = true; break; }
			if (!_wall) return true;
		}
		return false;
	};

	for (BlackoutCharacter* _character : GetLevel()->GetActorManager().GetAllActorOfClass<BlackoutCharacter>())
	{
		if (_character->IsPendingDestroy()) continue;
		const BlackoutHero* _asHero = Cast<BlackoutHero>(_character);
		const bool _enemy = _character->GetTeam() != localTeam && (!versus || (_asHero && _asHero->GetPlayerId() >= 0));
		bool _seen = !_enemy || _character->IsDead();
		if (!_seen) _seen = std::any_of(_viewers.begin(), _viewers.end(), [&](BlackoutHero* _viewer) { return InSight(*_viewer, *_character); });
		if (!_seen) _seen = _litByBullet(*_character);
		const float _target = _seen ? 1.f : 0.f;
		const float _step = _deltaTime * (_seen ? 8.f : 4.f);
		const float _shown = _character->GetShown();
		_character->SetShown(_shown + std::clamp(_target - _shown, -_step, _step));
	}
}

void BlackoutGameMode::UpdateSpreadCone()
{
	// A red fan from the muzzle toward the mouse: each bullet leaves somewhere inside it (BlackoutCharacter::Fire turns it up to the
	// spread, either side). Drawn on the HUD: seen in the dark too.
	Shape* _shape = spreadCone->GetImage()->GetShapeObject()->GetShape();
	BlackoutHero* _hero = GetHero();
	PolygonShape* _polygon = Cast<PolygonShape>(_shape);
	if (!_hero || _hero->IsDead() || shopOpen || !_polygon)
	{
		_shape->SetFillColor(Color(255, 40, 40, 0));
		_shape->SetOutlineThickness(0.f);
		return;
	}

	const FVector2 _muzzle = _hero->MuzzlePosition();
	const FVector2 _aim = _hero->GetAimDirection();		// as the bullets go (straight ahead when the mouse is not in front of the gun)
	const float _half = std::max(0.6f, _hero->GetSpread()) * DEGREES;		// the sniper's: still a thin line
	const float _length = std::min(_hero->GetBulletRange(), CONE_LENGTH);
	const FVector2 _origin = WorldToHud(_muzzle);

	const int STEPS = 8;		// the far side is an arc
	std::vector<FVector2> _points{ FVector2() };
	for (int _step = 0; _step <= STEPS; ++_step)
	{
		const float _turn = -_half + 2.f * _half * static_cast<float>(_step) / static_cast<float>(STEPS);
		const FVector2 _direction(_aim.x * std::cos(_turn) - _aim.y * std::sin(_turn), _aim.x * std::sin(_turn) + _aim.y * std::cos(_turn));
		_points.push_back(WorldToHud(_muzzle + _direction * _length) - _origin);
	}
	_polygon->SetPolygon(Polygon2D(_points));
	spreadCone->SetPosition(_origin);
	_shape->SetFillColor(Color(255, 40, 40, 45));
	_shape->SetOutlineThickness(1.5f);
	_shape->SetOutlineColor(Color(255, 60, 50, 130));
}

bool BlackoutGameMode::InSight(BlackoutHero& _viewer, BlackoutCharacter& _target)
{
	// Close (the light around him), or in the beam of his flashlight; and nothing in between.
	const FVector2 _offset = _target.GetPosition() - _viewer.GetPosition();
	const float _distance = _offset.Length();
	if (_distance > NEAR_SIGHT)
	{
		if (_distance > _viewer.GetLightRange()) return false;
		const FVector2 _look = _viewer.GetRightVector();
		const float _cosine = (_look.x * _offset.x + _look.y * _offset.y) / _distance;
		if (_cosine < std::cos((_viewer.GetLightHalfAngle() + 4.f) * DEGREES)) return false;
	}
	return _viewer.CanSee(&_target, _distance + 1.f);
}

void BlackoutGameMode::UpdateNameTags()
{
	NetworkManager* _net = GetWorld()->GetNetwork();
	BlackoutHero* _local = GetHero();
	const std::vector<BlackoutHero*> _heroes = GetLevel()->GetActorManager().GetAllActorOfClass<BlackoutHero>();

	// Who sees for us: the local hero and his teammates still standing.
	std::vector<BlackoutHero*> _viewers;
	for (BlackoutHero* _hero : _heroes)
		if (!_hero->IsDead() && !_hero->IsPendingDestroy() && _hero->GetTeam() == localTeam && (_hero == _local || _hero->GetPlayerId() >= 0))
			_viewers.push_back(_hero);

	// Who gets a name: the other players (versus), the enemies (solo).
	std::vector<BlackoutCharacter*> _characters;
	if (versus)
	{
		for (BlackoutHero* _hero : _heroes)
			if (_hero->GetPlayerId() >= 0) _characters.push_back(_hero);
	}
	else
		for (BlackoutEnemy* _enemy : GetLevel()->GetActorManager().GetAllActorOfClass<BlackoutEnemy>()) _characters.push_back(_enemy);

	size_t _used = 0;
	for (BlackoutCharacter* _character : _characters)
	{
		if (_character == _local || _character->IsDead() || _character->IsPendingDestroy() || shopOpen) continue;

		// An ally: always (blue). An enemy: only while one of us sees him, never while we are blinded (red).
		const bool _ally = _character->GetTeam() == localTeam;
		if (!_ally)
		{
			if (blindLeft > 0.3f) continue;
			if (std::none_of(_viewers.begin(), _viewers.end(), [&](BlackoutHero* _viewer) { return InSight(*_viewer, *_character); })) continue;
		}

		if (_used == nameTags.size())
		{
			TextWidget* _tag = MakeText(FVector2(-1000.f, -1000.f), 22u);
			_tag->GetLabel()->SetOutlineThickness(2.f);
			nameTags.push_back(_tag);
		}
		TextWidget* _tag = nameTags[_used++];
		const std::string _name = versus ? _net->GetPlayerName(_character->GetPlayerId()) : std::string("Ennemi");
		SetText(_tag, _name.empty() ? "Joueur " + std::to_string(_character->GetPlayerId()) : _name, 0.5f);
		_tag->GetLabel()->SetFillColor(_ally ? ALLY_COLOR : ENEMY_COLOR);
		_tag->SetPosition(WorldToHud(_character->GetPosition() - FVector2(0.f, HEAD_HEIGHT)) - FVector2(0.f, 26.f));
	}
	for (size_t _i = _used; _i < nameTags.size(); ++_i) SetText(nameTags[_i], "", 0.5f);

	// The carrier of the charge: its picture over his head (over his name), seen by his team only.
	Shape* _icon = chargeIcon->GetImage()->GetShapeObject()->GetShape();
	BlackoutHero* _carrier = versus && state == 0 && chargeState == Carried && localTeam == attackers ? FindHero(carrierId) : nullptr;
	if (_carrier && !_carrier->IsDead())
	{
		const float _above = _carrier == _local ? 8.f : 46.f;		// no name over the local hero
		chargeIcon->SetPosition(WorldToHud(_carrier->GetPosition() - FVector2(0.f, HEAD_HEIGHT)) - FVector2(0.f, _above));
		_icon->SetFillColor(Color::White());
	}
	else _icon->SetFillColor(Color(0, 0, 0, 0));
}

// ---- Guns on the floor

BlackoutDroppedWeapon* BlackoutGameMode::FindDrop(const int& _dropId)
{
	for (BlackoutDroppedWeapon* _drop : GetLevel()->GetActorManager().GetAllActorOfClass<BlackoutDroppedWeapon>())
		if (_drop->GetDropId() == _dropId && !_drop->IsPendingDestroy()) return _drop;
	return nullptr;
}

void BlackoutGameMode::DropWeapon(const int& _weapon, const FVector2& _position)
{
	// Beside the soldier (under him, it could not be seen), turned at random; not into a wall.
	const int _dropId = nextDropId++;
	const float _angle = BlackoutRandom(0.f, 360.f);
	const float _side = BlackoutRandom(0.f, 6.2831853f);
	FVector2 _spot = _position + FVector2(std::cos(_side), std::sin(_side)) * 34.f;
	for (const RaycastHitInfo& _hit : GetLevel()->GetWorldPhysics().RaycastAll(_position, _spot))
		if (_hit.hit && _hit.actor && !Cast<BlackoutCharacter>(_hit.actor)) _spot = _position;
	ApplyDrop(_dropId, _weapon, _spot, _angle);
	NetworkManager* _net = GetWorld()->GetNetwork();
	if (_net->IsOnline()) _net->Send("BO_Drop", NetworkManager::Args(_dropId, _weapon, _spot, _angle), NetTarget::Others);
}

void BlackoutGameMode::ApplyDrop(const int& _dropId, const int& _weapon, const FVector2& _position, const float& _angle)
{
	if (FindDrop(_dropId)) return;
	BlackoutDroppedWeapon* _drop = GetLevel()->SpawnActor<BlackoutDroppedWeapon>();
	_drop->SetPosition(_position);
	_drop->Setup(_dropId, _weapon, _angle);
	LOG(VerbosityType::Log, "Blackout: " + BlackoutWeapon::Get(_weapon).name + " dropped (" + std::to_string(_dropId) + ")");
}

void BlackoutGameMode::HostPickup(const int& _player, const int& _dropId)
{
	// The gun is still there, the player alive and close enough: he takes it, his own falls where he stands.
	NetworkManager* _net = GetWorld()->GetNetwork();
	BlackoutHero* _hero = versus ? FindHero(_player) : GetHero();
	BlackoutDroppedWeapon* _drop = FindDrop(_dropId);
	if (!_hero || _hero->IsDead() || !_drop || (_drop->GetPosition() - _hero->GetPosition()).Length() > WEAPON_PICKUP * 1.6f) return;
	if (versus && state != 0) return;

	// It takes the place of the gun of its kind (primary or sidearm), which falls (no primary yet: nothing falls).
	const int _weapon = _drop->GetWeapon();
	const int _slot = BlackoutWeapon::Get(_weapon).IsSidearm() ? 1 : 0;
	const int _old = _slot == 0 ? _hero->GetPrimary() : _hero->GetSidearm();
	_net->SetShared((_slot == 0 ? "primary_" : "sidearm_") + std::to_string(_player), _weapon);
	ApplyPicked(_dropId, _player, _weapon);
	if (_net->IsOnline()) _net->Send("BO_Picked", NetworkManager::Args(_dropId, _player, _weapon), NetTarget::Others);
	if (_old >= 0) DropWeapon(_old, _hero->GetPosition());
}

void BlackoutGameMode::ApplyPicked(const int& _dropId, const int& _player, const int& _weapon)
{
	if (BlackoutDroppedWeapon* _drop = FindDrop(_dropId)) _drop->Destroy();
	freshWeapon.erase(_player * 2 + (BlackoutWeapon::Get(_weapon).IsSidearm() ? 1 : 0));		// not bought: sold back for half its price
	if (BlackoutHero* _hero = versus ? FindHero(_player) : GetHero()) _hero->GiveWeapon(_weapon);
	LOG(VerbosityType::Log, "Blackout: player " + std::to_string(_player) + " took " + BlackoutWeapon::Get(_weapon).name);
}

void BlackoutGameMode::UpdatePickup()
{
	NetworkManager* _net = GetWorld()->GetNetwork();
	BlackoutHero* _hero = GetHero();
	const bool _key = GetWorld()->GetWindowRef().HasFocus() && BlackoutKeys::Down(BlackoutKeys::Use) && !(chat && chat->IsTyping()) && !shopOpen;
	const bool _pressed = _key && !pickupHeld;
	pickupHeld = _key;

	// The nearest gun on the floor.
	BlackoutDroppedWeapon* _nearest = nullptr;
	float _best = WEAPON_PICKUP;
	if (_hero && !_hero->IsDead() && state == 0)
		for (BlackoutDroppedWeapon* _drop : GetLevel()->GetActorManager().GetAllActorOfClass<BlackoutDroppedWeapon>())
		{
			const float _distance = (_drop->GetPosition() - _hero->GetPosition()).Length();
			if (!_drop->IsPendingDestroy() && _distance < _best)
			{
				_best = _distance;
				_nearest = _drop;
			}
		}

	// E plants or defuses first.
	const bool _chargeAction = versus && _hero && (action != 0 ||
		(chargeState == Carried && carrierId == _net->GetLocalId() && SiteAt(_hero->GetPosition()) >= 0) ||
		(chargeState == Planted && _hero->GetTeam() != attackers && (_hero->GetPosition() - chargePosition).Length() < DEFUSE_RADIUS));
	if (!_nearest || _chargeAction || shopOpen)
	{
		SetText(pickupText, "", 0.5f);
		return;
	}
	const BlackoutWeapon& _found = BlackoutWeapon::Get(_nearest->GetWeapon());
	const int _replaced = _found.IsSidearm() ? _hero->GetSidearm() : _hero->GetPrimary();
	SetText(pickupText, "E : prendre " + _found.name + (_replaced >= 0 ? "  (tu laches " + BlackoutWeapon::Get(_replaced).name + ")" : std::string()), 0.5f);
	if (!_pressed) return;

	const int _me = _net->GetLocalId();
	if (_net->HasAuthority()) HostPickup(_me, _nearest->GetDropId());
	else _net->Send("BO_Pickup", NetworkManager::Args(_nearest->GetDropId()), NetTarget::Host);
}

FVector2 BlackoutGameMode::ChargeStartPoint()
{
	// From the first attacker's spot toward the middle of the arena, stopped before a wall (the soldiers do not stop it).
	const FVector2 _from = SpawnPoint(0, 0);
	const FVector2 _toMiddle = GetPosition() - _from;
	const FVector2 _direction = _toMiddle.Length() > 1.f ? _toMiddle.Normalized() : FVector2(1.f, 0.f);
	float _distance = CHARGE_START;
	for (const RaycastHitInfo& _hit : GetLevel()->GetWorldPhysics().RaycastAll(_from, _from + _direction * CHARGE_START))
		if (_hit.hit && _hit.actor && !Cast<BlackoutCharacter>(_hit.actor))
			_distance = std::min(_distance, std::max(0.f, (_hit.point - _from).Length() - 30.f));
	return _from + _direction * _distance;
}

void BlackoutGameMode::SetMessage(const std::string& _text, const Color& _color)
{
	SetText(messageText, _text, 0.5f);
	messageText->GetLabel()->SetFillColor(_color);
}

void BlackoutGameMode::SetText(TextWidget* _widget, const std::string& _text, const float& _align)
{
	// The label is centered on the widget by default: _align 0 puts its left side there, 1 its right side.
	if (_widget->GetLabel()->GetString() != _text) _widget->SetText(String(_text));
	const FRect _bounds = _widget->GetLabel()->GetLocalBounds();
	_widget->GetLabel()->SetOrigin(FVector2(_bounds.GetPosition().x + _bounds.GetSize().x * _align, _bounds.GetPosition().y));
}

// ---- Events

void BlackoutGameMode::Shot(const BlackoutCharacter& _shooter, const FVector2& _position)
{
	// The shots of the other side are heard (an echo where they come from).
	if (_shooter.GetTeam() != localTeam)
	{
		Echo(_position, Color(255, 150, 60), 70.f);
		return;
	}
	if (versus) return;

	// The hero's shots are heard: the enemies close enough come to look.
	for (BlackoutEnemy* _enemy : GetLevel()->GetActorManager().GetAllActorOfClass<BlackoutEnemy>())
		if (!_enemy->IsDead() && (_enemy->GetPosition() - _position).Length() < static_cast<float>(hearingRange))
			_enemy->Hear(_position);
}

void BlackoutGameMode::Explosion(const FVector2& _position)
{
	Echo(_position, Color(255, 200, 120), 170.f);
	for (BlackoutEnemy* _enemy : GetLevel()->GetActorManager().GetAllActorOfClass<BlackoutEnemy>())
		if (!_enemy->IsDead() && (_enemy->GetPosition() - _position).Length() < static_cast<float>(hearingRange))
			_enemy->Hear(_position);
}

void BlackoutGameMode::HeroHurt()
{
	hurtTime = 0.35f;
}

void BlackoutGameMode::EnemyKilled(const FVector2& _position, const int& _weapon)
{
	++kills;
	DropWeapon(_weapon, _position);		// the hero can take it (E)
	Echo(_position, Color(255, 255, 255), 90.f);
	LOG(VerbosityType::Log, "Blackout: enemy killed (" + std::to_string(kills) + ")");
}

}
