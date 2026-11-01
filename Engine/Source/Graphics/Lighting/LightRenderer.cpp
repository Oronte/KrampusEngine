#include "LightRenderer.h"
#include "Components/LightComponents.h"
#include "Actors/Actor.h"
#include <algorithm>
#include <cmath>

using namespace Krampus;

namespace
{
	constexpr int		SEGMENTS	= 40;
	constexpr float		PI			= 3.14159265f;

	sf::Color Scaled(const Color& _color, const float _intensity)
	{
		const auto _channel = [&](const float _value) { return static_cast<std::uint8_t>(std::clamp(_value * _intensity, 0.f, 255.f)); };
		return sf::Color(_channel(_color.r), _channel(_color.g), _channel(_color.b));
	}

	sf::Color Times(const sf::Color& _color, const float _factor)
	{
		return sf::Color(static_cast<std::uint8_t>(_color.r * _factor), static_cast<std::uint8_t>(_color.g * _factor), static_cast<std::uint8_t>(_color.b * _factor));
	}

	// The light gets weaker as (1 - distance / radius)^2: rings at a quarter, half and three quarters of the radius.
	void BuildGradient(std::vector<sf::Vertex>& _out, const sf::Vector2f& _center, const float _radius, const sf::Color& _color)
	{
		constexpr float RINGS[] = { 0.f, 0.25f, 0.5f, 0.75f, 1.f };
		const auto _point = [&](const float _ring, const int _segment)
		{
			const float _angle = 2.f * PI * static_cast<float>(_segment) / SEGMENTS;
			const float _falloff = (1.f - _ring) * (1.f - _ring);
			return sf::Vertex{ _center + sf::Vector2f(std::cos(_angle), std::sin(_angle)) * (_radius * _ring), Times(_color, _falloff) };
		};
		_out.clear();
		for (int _s = 0; _s < SEGMENTS; ++_s)
		{
			_out.push_back(sf::Vertex{ _center, _color });
			_out.push_back(_point(RINGS[1], _s));
			_out.push_back(_point(RINGS[1], _s + 1));
			for (int _r = 1; _r + 1 < 5; ++_r)
			{
				const sf::Vertex _a = _point(RINGS[_r], _s), _b = _point(RINGS[_r], _s + 1), _c = _point(RINGS[_r + 1], _s + 1), _d = _point(RINGS[_r + 1], _s);
				_out.insert(_out.end(), { _a, _b, _c, _a, _c, _d });
			}
		}
	}

	// The shadow of a polygon: the edges that face away from the light are pushed away from it, so the polygon itself stays lit.
	void BuildShadow(std::vector<sf::Vertex>& _out, const std::vector<FVector2>& _polygon, const FVector2& _light, const float _reach)
	{
		const size_t _count = _polygon.size();
		if (_count < 3) return;
		float _area = 0.f;
		for (size_t _i = 0; _i < _count; ++_i)
		{
			const FVector2& _a = _polygon[_i];
			const FVector2& _b = _polygon[(_i + 1) % _count];
			_area += _a.x * _b.y - _b.x * _a.y;
		}
		const auto _away = [&](const FVector2& _point)
		{
			FVector2 _direction = _point - _light;
			const float _length = std::sqrt(_direction.x * _direction.x + _direction.y * _direction.y);
			if (_length > 0.001f) _direction = _direction * (_reach / _length);
			return sf::Vector2f(_point.x + _direction.x, _point.y + _direction.y);
		};
		const sf::Color _black(0, 0, 0);
		for (size_t _i = 0; _i < _count; ++_i)
		{
			const FVector2& _a = _polygon[_i];
			const FVector2& _b = _polygon[(_i + 1) % _count];
			const FVector2 _edge = _b - _a;
			const FVector2 _outward = _area > 0.f ? FVector2(_edge.y, -_edge.x) : FVector2(-_edge.y, _edge.x);
			const FVector2 _toEdge = _a - _light;
			if (_outward.x * _toEdge.x + _outward.y * _toEdge.y <= 0.f) continue;	// faces the light

			const sf::Vertex _p0{ sf::Vector2f(_a.x, _a.y), _black }, _p1{ sf::Vector2f(_b.x, _b.y), _black };
			const sf::Vertex _p2{ _away(_b), _black }, _p3{ _away(_a), _black };
			_out.insert(_out.end(), { _p0, _p1, _p2, _p0, _p2, _p3 });
		}
	}

	// A light that reaches nothing on the screen is skipped (the view is taken as not rotated: a bigger box, never too small).
	bool Visible(const sf::View& _view, const sf::Vector2f& _center, const float _radius)
	{
		const sf::Vector2f _half = _view.getSize() * 0.5f;
		const float _margin = std::max(std::abs(_half.x), std::abs(_half.y)) * 0.42f;	// a turned view sees a little more
		return std::abs(_center.x - _view.getCenter().x) <= std::abs(_half.x) + _margin + _radius
			&& std::abs(_center.y - _view.getCenter().y) <= std::abs(_half.y) + _margin + _radius;
	}
}

bool LightRenderer::IsLit(const Level* _level)
{
	return GlobalLightComponent::Of(_level) || !PointLightComponent::LightsOf(_level).empty();
}

void LightRenderer::Render(const Level* _level, sf::RenderTarget& _target)
{
	const std::vector<PointLightComponent*> _lights = PointLightComponent::LightsOf(_level);
	const GlobalLightComponent* _global = GlobalLightComponent::Of(_level);
	if (_lights.empty() && !_global) return;

	const sf::Vector2u _size = _target.getSize();
	if (_size.x == 0 || _size.y == 0) return;
	if (lightMap.getSize() != _size && !lightMap.resize(_size)) return;

	const sf::View _view = _target.getView();
	const sf::View _screen(sf::FloatRect({ 0.f, 0.f }, sf::Vector2f(_size)));
	lightMap.clear(_global ? Scaled(_global->color, _global->intensity) : sf::Color::Black);
	lightMap.setView(_view);

	// The outlines once for every light.
	std::vector<std::vector<FVector2>> _casters;
	for (const ShadowCasterComponent* _caster : ShadowCasterComponent::CastersOf(_level))
		if (std::vector<FVector2> _outline = _caster->GetWorldOutline(); _outline.size() >= 3) _casters.push_back(std::move(_outline));

	for (const PointLightComponent* _light : _lights)
	{
		const FVector2 _position = _light->GetPosition();
		const sf::Vector2f _center(_position.x, _position.y);
		const float _radius = std::max(1.f, _light->radius);
		if (!Visible(_view, _center, _radius)) continue;
		BuildGradient(gradient, _center, _radius, Scaled(_light->color, _light->intensity));

		shadows.clear();
		if (_light->castShadows)
			for (const std::vector<FVector2>& _outline : _casters)
			{
				// Only the casters the light reaches (their box against its circle).
				float _left = _outline[0].x, _right = _left, _top = _outline[0].y, _bottom = _top;
				for (const FVector2& _p : _outline) { _left = std::min(_left, _p.x); _right = std::max(_right, _p.x); _top = std::min(_top, _p.y); _bottom = std::max(_bottom, _p.y); }
				const float _dx = std::max({ _left - _position.x, 0.f, _position.x - _right }), _dy = std::max({ _top - _position.y, 0.f, _position.y - _bottom });
				if (_dx * _dx + _dy * _dy <= _radius * _radius) BuildShadow(shadows, _outline, _position, _radius * 2.f);
			}

		if (shadows.empty())
		{
			lightMap.draw(gradient.data(), gradient.size(), sf::PrimitiveType::Triangles, sf::BlendAdd);
			continue;
		}
		if (oneLight.getSize() != _size && !oneLight.resize(_size)) continue;
		oneLight.clear(sf::Color::Black);
		oneLight.setView(_view);
		oneLight.draw(gradient.data(), gradient.size(), sf::PrimitiveType::Triangles);
		oneLight.draw(shadows.data(), shadows.size(), sf::PrimitiveType::Triangles);
		oneLight.display();
		lightMap.setView(_screen);
		lightMap.draw(sf::Sprite(oneLight.getTexture()), sf::BlendAdd);
		lightMap.setView(_view);
	}
	lightMap.display();

	// Only the part of the window this view draws in (split screen: each camera lights its own part).
	const sf::IntRect _viewport = _target.getViewport(_view);
	sf::Sprite _sprite(lightMap.getTexture());
	_sprite.setTextureRect(_viewport);
	_sprite.setPosition(sf::Vector2f(_viewport.position));
	_target.setView(_screen);
	_target.draw(_sprite, sf::BlendMultiply);
	_target.setView(_view);
}
