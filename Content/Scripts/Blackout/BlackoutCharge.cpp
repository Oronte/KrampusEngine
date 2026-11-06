#include "BlackoutCharge.h"

namespace Krampus
{

// ---- BlackoutSite

BlackoutSite::BlackoutSite(Level* _level)
	: Actor(_level)
{
	name = "Site";

	zone = CreateComponent<SpriteComponent>(CircleShapeData(170.f, "Blackout/Solid"));
	zone->GetShapeObject()->GetShape()->SetFillColor(Color(230, 190, 60, 45));
	zone->GetShapeObject()->GetShape()->SetOutlineThickness(4.f);
	zone->GetShapeObject()->GetShape()->SetOutlineColor(Color(240, 200, 70, 160));
	zone->SetZOrder(static_cast<std::uint8_t>(10));		// on the floor, under the walls and the soldiers

	letter = CreateComponent<LabelComponent>(TextData("A", 120u));
	letter->SetFillColor(Color(240, 200, 70, 120));
	letter->SetZOrder(static_cast<std::uint8_t>(11));
	Refresh();
}

void BlackoutSite::BeginPlay()
{
	Actor::BeginPlay();
	Refresh();
}

bool BlackoutSite::Contains(const FVector2& _position) const
{
	return (_position - GetPosition()).Length() <= static_cast<float>(radius);
}

void BlackoutSite::Tick(const Float& _deltaTime)
{
	Actor::Tick(_deltaTime);
	Refresh();
}

void BlackoutSite::Refresh()
{
	// The circle and the letter follow the properties (also edited in the Inspector).
	if (static_cast<float>(radius) != shownRadius)
	{
		shownRadius = static_cast<float>(radius);
		zone->SetRadius(radius);
	}
	if (siteName != shownName)
	{
		shownName = siteName;
		letter->SetString(siteName);
		const FRect _bounds = letter->GetLocalBounds();
		letter->SetOrigin(FVector2(_bounds.GetPosition().x + _bounds.GetSize().x / 2.f, _bounds.GetPosition().y + _bounds.GetSize().y / 2.f));
	}
}

// ---- BlackoutBuyZone

BlackoutBuyZone::BlackoutBuyZone(Level* _level)
	: Actor(_level)
{
	name = "BuyZone";

	ring = CreateComponent<SpriteComponent>(CircleShapeData(100.f, "Blackout/Solid", TextureExtensionType::PNG, IRect(), 90));
	ring->GetShapeObject()->GetShape()->SetFillColor(Color(0, 0, 0, 0));
	ring->GetShapeObject()->GetShape()->SetOutlineThickness(-5.f);		// inside the circle: the hero stops on it
	ring->GetShapeObject()->GetShape()->SetOutlineColor(Color(0, 0, 0, 0));
	ring->SetZOrder(static_cast<std::uint8_t>(12));		// on the floor, under the soldiers (and the HUD is drawn after the world)

	// A faint green light: the circle is seen in the dark.
	glow = CreateComponent<PointLightComponent>();
	glow->color = Color(120, 220, 140);
	glow->intensity = 0.f;
	glow->castShadows = true;
	Show(false, FVector2(), 0.f);
}

void BlackoutBuyZone::Show(const bool& _visible, const FVector2& _center, const float& _radius)
{
	if (_visible)
	{
		SetPosition(_center);
		if (_radius != radius)
		{
			radius = _radius;
			ring->SetRadius(_radius);
			glow->radius = _radius * 1.15f;
		}
	}
	if (_visible == visible) return;
	visible = _visible;
	ring->GetShapeObject()->GetShape()->SetFillColor(_visible ? Color(120, 220, 140, 22) : Color(0, 0, 0, 0));
	ring->GetShapeObject()->GetShape()->SetOutlineColor(_visible ? Color(140, 230, 150, 210) : Color(0, 0, 0, 0));
	glow->intensity = _visible ? 0.35f : 0.f;
}

// ---- BlackoutCharge

BlackoutCharge::BlackoutCharge(Level* _level)
	: Actor(_level)
{
	name = "Charge";

	box = CreateComponent<SpriteComponent>(RectangleShapeData(FVector2(40.f, 34.f), "Bomb"));
	box->GetShapeObject()->GetShape()->SetFillColor(Color(0, 0, 0, 0));
	box->SetZOrder(ZOrder::Projectiles);

	light = CreateComponent<PointLightComponent>();
	light->color = Color(255, 60, 50);
	light->radius = 110.f;
	light->intensity = 0.f;
	light->castShadows = false;
}

void BlackoutCharge::Show(const bool& _visible, const FVector2& _position)
{
	visible = _visible;
	if (_visible) SetPosition(_position);
	box->GetShapeObject()->GetShape()->SetFillColor(_visible ? Color::White() : Color(0, 0, 0, 0));
	if (!_visible) light->intensity = 0.f;
}

void BlackoutCharge::SetBlink(const float& _period)
{
	period = _period;
}

void BlackoutCharge::Tick(const Float& _deltaTime)
{
	Actor::Tick(_deltaTime);
	if (!visible) return;
	clock += static_cast<float>(_deltaTime);
	light->intensity = period <= 0.f ? 0.7f : (std::fmod(clock, period) < 0.12f ? 1.6f : 0.25f);
}

}
