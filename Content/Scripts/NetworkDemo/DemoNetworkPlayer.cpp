#include "DemoNetworkPlayer.h"
#include "Core/Engine.h"
#include "Network/NetworkManager.h"
#include "Components/CameraComponent.h"

namespace Krampus
{

FRect DemoArena(Level* _level)
{
	FVector2 _center(960.f, 540.f);
	FVector2 _size(1920.f, 1080.f);
	if (CameraComponent* _camera = _level ? _level->GetDefaultCamera() : nullptr)
		if (const sf::View* _view = _camera->GetView())
		{
			_center = FVector2(_view->getCenter().x, _view->getCenter().y);
			_size = FVector2(_view->getSize().x, _view->getSize().y);
		}

	// Room for the texts of the HUD at the top and at the bottom.
	const FVector2 _arena(std::max(200.f, _size.x - 160.f), std::max(200.f, _size.y - 420.f));
	return FRect(FVector2(_center.x - _arena.x / 2.f, _center.y - _arena.y / 2.f + 40.f), _arena);
}

DemoNetworkPlayer::DemoNetworkPlayer(Level* _level)
	: Actor(_level)
{
	name = "DemoNetworkPlayer";

	body = CreateComponent<SpriteComponent>(CircleShapeData(size));
	body->GetShapeObject()->GetShape()->ClearTexture();		// a plain disc in the player's color
	body->GetShapeObject()->GetShape()->SetOutlineThickness(3.f);

	nameLabel = CreateComponent<LabelComponent>(TextData("Player", 22u));
	nameLabel->SetOutlineColor(Color::Black());
	nameLabel->SetOutlineThickness(2.f);

	// The owner sends its position (20 times a second, UDP) and its size; the others glide to it.
	sync = CreateComponent<NetworkSyncComponent>();
	sync->syncRotation = false;
	sync->properties = "DemoNetworkPlayer.size";
}

Color DemoNetworkPlayer::ColorOf(const int _playerId)
{
	static const Color COLORS[] =
	{
		Color(255, 96, 96), Color(90, 170, 255), Color(120, 220, 120), Color(255, 200, 60),
		Color(200, 130, 255), Color(60, 220, 220), Color(255, 140, 200), Color(190, 190, 190)
	};
	return COLORS[static_cast<size_t>(std::abs(_playerId)) % std::size(COLORS)];
}

void DemoNetworkPlayer::Flash()
{
	flashTime = 0.6f;
	LOG(VerbosityType::Log, "Demo: " + GetName().StdString() + " flashes");
}

void DemoNetworkPlayer::Tick(const Float& _deltaTime)
{
	Actor::Tick(_deltaTime);

	UpdateControls(_deltaTime);
	UpdateLook();
	flashTime = std::max(0.f, flashTime - static_cast<float>(_deltaTime));
}

void DemoNetworkPlayer::UpdateControls(const float& _deltaTime)
{
	// Only the owner controls it, and only from the window that has the focus (two windows on one PC share the keyboard).
	if (!sync->IsLocalOwner() || !GetWorld()->GetWindowRef().HasFocus()) return;

	using Key = sf::Keyboard::Key;
	const auto _down = [](const std::initializer_list<Key> _keys)
	{
		for (const Key _key : _keys)
			if (sf::Keyboard::isKeyPressed(_key)) return true;
		return false;
	};

	FVector2 _direction;
	if (_down({ Key::Z, Key::W, Key::Up }))		_direction.y -= 1.f;
	if (_down({ Key::S, Key::Down }))			_direction.y += 1.f;
	if (_down({ Key::Q, Key::A, Key::Left }))	_direction.x -= 1.f;
	if (_down({ Key::D, Key::Right }))			_direction.x += 1.f;

	if (_direction.x != 0.f || _direction.y != 0.f)
	{
		const float _length = std::sqrt(_direction.x * _direction.x + _direction.y * _direction.y);
		const float _distance = static_cast<float>(speed) * _deltaTime;
		const FVector2 _step(_direction.x / _length * _distance, _direction.y / _length * _distance);

		const float _radius = static_cast<float>(size);
		const FRect _arena = DemoArena(GetLevel());
		const FVector2 _min = _arena.GetPosition() + FVector2(_radius, _radius);
		const FVector2 _max = _arena.GetPosition() + _arena.GetSize() - FVector2(_radius, _radius);
		const FVector2 _position = GetPosition() + _step;
		SetPosition(FVector2(std::clamp(_position.x, _min.x, std::max(_min.x, _max.x)), std::clamp(_position.y, _min.y, std::max(_min.y, _max.y))));
	}

	// Space: an RPC, every machine runs Flash on this player.
	const bool _space = _down({ Key::Space });
	if (_space && !spaceHeld) sync->CallRPC("Flash", NetTarget::All);
	spaceHeld = _space;

	// F / G: a property shared by the NetworkSyncComponent.
	const bool _grow = _down({ Key::F });
	const bool _shrink = _down({ Key::G });
	if ((_grow || _shrink) && !resizeHeld)
		size = std::clamp(static_cast<float>(size) + (_grow ? 8.f : -8.f), 12.f, 120.f);
	resizeHeld = _grow || _shrink;
}

void DemoNetworkPlayer::UpdateLook()
{
	NetworkManager* _net = GetWorld()->GetNetwork();
	const int _owner = sync->GetOwnerId();

	// The size (and the bounce of a flash).
	const float _bounce = flashTime > 0.f ? 1.f + 0.35f * std::sin(flashTime / 0.6f * 3.14159f) : 1.f;
	const float _radius = static_cast<float>(size) * _bounce;
	if (_radius != shownSize)
	{
		shownSize = _radius;
		body->SetRadius(_radius);
	}

	// The color of the owner, white during a flash.
	const Color _base = ColorOf(_owner);
	const float _white = std::clamp(flashTime / 0.6f, 0.f, 1.f);
	const auto _mix = [_white](const std::uint8_t _from) { return static_cast<std::uint8_t>(_from + (255 - _from) * _white); };
	body->GetShapeObject()->GetShape()->SetFillColor(Color(_mix(_base.r), _mix(_base.g), _mix(_base.b)));
	body->GetShapeObject()->GetShape()->SetOutlineColor(sync->IsLocalOwner() && _net->IsOnline() ? Color::White() : Color(30, 30, 40));

	// The name of the owner above the disc.
	const std::string _name = _net->GetPlayerName(_owner) + (sync->IsLocalOwner() && _net->IsOnline() ? " (you)" : "");
	if (_name != shownName)
	{
		shownName = _name;
		nameLabel->SetString(String(_name));
	}
	const FRect _bounds = nameLabel->GetLocalBounds();
	nameLabel->SetOrigin(FVector2(_bounds.GetPosition().x + _bounds.GetSize().x / 2.f, _bounds.GetPosition().y + _bounds.GetSize().y + _radius + 10.f));
}

}
