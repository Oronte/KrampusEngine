#include "LightComponents.h"
#include "Actors/Actor.h"
#include "Components/SpriteComponent.h"
#include "Utilities/Math/Polygon2D.h"
#include "Utilities/Debug/Debug.h"
#include <cmath>
#include <algorithm>

using namespace Krampus;

namespace
{
	template <typename T>
	std::vector<T*>& All()
	{
		static std::vector<T*> _all;
		return _all;
	}

	template <typename T>
	void Register(T* _component)
	{
		if (std::find(All<T>().begin(), All<T>().end(), _component) == All<T>().end()) All<T>().push_back(_component);
	}

	template <typename T>
	std::vector<T*> Of(const Level* _level)
	{
		std::vector<T*> _found;
		for (T* _component : All<T>())
			if (_component->GetLevel() == _level && _component->IsActive() && _component->GetOwner()->IsActive()) _found.push_back(_component);
		return _found;
	}
}

// ---- Point light

PointLightComponent::PointLightComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(PointLightComponent);
}

void PointLightComponent::Construct()
{
	Component::Construct();
	Register(this);
}

void PointLightComponent::Deconstruct()
{
	Component::Deconstruct();
	std::erase(All<PointLightComponent>(), this);
}

std::vector<PointLightComponent*> PointLightComponent::LightsOf(const Level* _level)
{
	return Of<PointLightComponent>(_level);
}

// ---- Global light

GlobalLightComponent::GlobalLightComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(GlobalLightComponent);
}

void GlobalLightComponent::Construct()
{
	Component::Construct();
	Register(this);
}

void GlobalLightComponent::Deconstruct()
{
	Component::Deconstruct();
	std::erase(All<GlobalLightComponent>(), this);
}

GlobalLightComponent* GlobalLightComponent::Of(const Level* _level)
{
	const std::vector<GlobalLightComponent*> _lights = ::Of<GlobalLightComponent>(_level);
	return _lights.empty() ? nullptr : _lights.front();
}

// ---- Spot light

SpotLightComponent::SpotLightComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(SpotLightComponent);
}

void SpotLightComponent::Construct()
{
	Component::Construct();
	Register(this);
}

void SpotLightComponent::Deconstruct()
{
	Component::Deconstruct();
	std::erase(All<SpotLightComponent>(), this);
}

FVector2 SpotLightComponent::GetDirection() const
{
	const float _radians = static_cast<float>(GetRotation().ToRadians());
	return FVector2(std::cos(_radians), std::sin(_radians));
}

std::vector<SpotLightComponent*> SpotLightComponent::LightsOf(const Level* _level)
{
	return Of<SpotLightComponent>(_level);
}

void SpotLightComponent::DrawDebug()
{
#ifdef DEBUG
	// The two edges of the cone and its far arc, in yellow, with a line along the direction.
	const FVector2 _from = GetPosition();
	const float _middle = static_cast<float>(GetRotation().ToRadians());
	const float _half = std::clamp(angle, 1.f, 360.f) * 0.5f * 3.14159265f / 180.f;
	const float _reach = std::max(1.f, radius);
	const auto _at = [&](const float _offset) { return _from + FVector2(std::cos(_middle + _offset), std::sin(_middle + _offset)) * _reach; };

	constexpr int ARC = 16;
	FVector2 _previous = _at(-_half);
	Debug::DrawDebugSegment(GetLevel(), _from, _previous, Color::Yellow());
	for (int _i = 1; _i <= ARC; ++_i)
	{
		const FVector2 _next = _at(-_half + 2.f * _half * static_cast<float>(_i) / ARC);
		Debug::DrawDebugSegment(GetLevel(), _previous, _next, Color::Yellow());
		_previous = _next;
	}
	Debug::DrawDebugSegment(GetLevel(), _from, _previous, Color::Yellow());
	Debug::DrawDebugSegment(GetLevel(), _from, _at(0.f), Color(255, 255, 0, 120));
	Debug::DrawDebugPoint(GetLevel(), _from, 6.f, Color::Yellow());
#endif
}

// ---- Shadow caster

ShadowCasterComponent::ShadowCasterComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(ShadowCasterComponent);
}

void ShadowCasterComponent::Construct()
{
	Component::Construct();
	Register(this);
}

void ShadowCasterComponent::Deconstruct()
{
	Component::Deconstruct();
	std::erase(All<ShadowCasterComponent>(), this);
}

std::vector<FVector2> ShadowCasterComponent::GetWorldOutline() const
{
	const SpriteComponent* _sprite = GetOwner()->GetComponent<SpriteComponent>();
	if (!_sprite || !_sprite->GetShapeObject()) return {};
	return Polygon2D::Transform(_sprite->GetShapeObject()->GetOutline().points, _sprite->GetPosition(), _sprite->GetRotation(), _sprite->GetScale());
}

std::vector<ShadowCasterComponent*> ShadowCasterComponent::CastersOf(const Level* _level)
{
	return Of<ShadowCasterComponent>(_level);
}
