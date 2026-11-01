#include "LightComponents.h"
#include "Actors/Actor.h"
#include "Components/SpriteComponent.h"
#include "Utilities/Math/Polygon2D.h"
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
