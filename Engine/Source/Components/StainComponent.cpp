#include "StainComponent.h"
#include "SpriteComponent.h"
#include "Actors/Actor.h"
#include "GameFramework/Level.h"
#include "Graphics/Window.h"
#include "Utilities/Math/Polygon2D.h"
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <algorithm>
#include <cmath>

using namespace Krampus;

namespace
{
	constexpr unsigned	CELL			= 128;		// one style of mark in the stamp picture
	constexpr unsigned	STYLE_COUNT		= 3;
	constexpr unsigned	MAX_TARGET		= 1024;		// biggest side of a marks picture
	constexpr size_t	MAX_PENDING		= 4096;		// marks waiting for a draw (a hidden sprite does not take memory forever)
	constexpr float		TWO_PI			= 6.2831853f;

	// Premultiplied colors: the marks picture keeps color * alpha, so marks over marks mix right.
	const sf::BlendMode PREMULTIPLIED(sf::BlendMode::Factor::One, sf::BlendMode::Factor::OneMinusSrcAlpha);
	// Fading takes the same amount off every channel.
	const sf::BlendMode FADE(sf::BlendMode::Factor::One, sf::BlendMode::Factor::One, sf::BlendMode::Equation::ReverseSubtract);

	std::vector<StainComponent*>& All()
	{
		static std::vector<StainComponent*> _all;
		return _all;
	}

	float Smooth(const float& _x)
	{
		const float _t = std::clamp(_x, 0.f, 1.f);
		return _t * _t * (3.f - 2.f * _t);
	}

	// A radius that changes around the circle, the same each time (seeded): irregular edges.
	float Wobble(const float& _angle, const std::uint32_t& _seed, const float& _amount)
	{
		float _sum = 0.f;
		for (int _k = 1; _k <= 5; ++_k)
		{
			const float _phase = static_cast<float>((_seed * (2654435761u + _k * 40503u)) % 6283u) / 1000.f;
			_sum += std::sin(_angle * (_k + 2) + _phase) / static_cast<float>(_k);
		}
		return 1.f + _sum * _amount * 0.45f;
	}

	// The three styles side by side, white with their shape in the alpha (the color of the mark tints them).
	const sf::Texture& StampAtlas()
	{
		static std::unique_ptr<sf::Texture> _atlas;
		if (_atlas) return *_atlas;

		sf::Image _image({ CELL * STYLE_COUNT, CELL }, sf::Color::Transparent);
		const auto _put = [&](const unsigned _style, const unsigned _x, const unsigned _y, const float _alpha)
		{
			_image.setPixel({ _style * CELL + _x, _y }, sf::Color(255, 255, 255, static_cast<std::uint8_t>(std::clamp(_alpha, 0.f, 1.f) * 255.f)));
		};

		// Drops around the splat: center (in -1..1) and radius.
		struct Drop { float x, y, r; };
		std::vector<Drop> _drops;
		std::uint32_t _seed = 12345u;
		const auto _next = [&]() { _seed ^= _seed << 13; _seed ^= _seed >> 17; _seed ^= _seed << 5; return (_seed >> 8) / 16777216.f; };
		for (int _i = 0; _i < 9; ++_i)
		{
			const float _angle = _next() * TWO_PI, _distance = 0.62f + _next() * 0.3f;
			_drops.push_back({ std::cos(_angle) * _distance, std::sin(_angle) * _distance, 0.035f + _next() * 0.07f });
		}

		for (unsigned _y = 0; _y < CELL; ++_y)
			for (unsigned _x = 0; _x < CELL; ++_x)
			{
				const float _dx = (_x + 0.5f) / CELL * 2.f - 1.f, _dy = (_y + 0.5f) / CELL * 2.f - 1.f;
				const float _d = std::sqrt(_dx * _dx + _dy * _dy);
				const float _angle = std::atan2(_dy, _dx);

				// Soft
				_put(0, _x, _y, Smooth(1.f - _d));

				// Splat: a blob with a wavy edge, and drops
				float _splat = Smooth((0.55f * Wobble(_angle, 7u, 0.5f) - _d) * 25.f);
				for (const Drop& _drop : _drops)
				{
					const float _ex = _dx - _drop.x, _ey = _dy - _drop.y;
					_splat = std::max(_splat, Smooth((_drop.r - std::sqrt(_ex * _ex + _ey * _ey)) * 40.f));
				}
				_put(1, _x, _y, _splat);

				// Scorch: darker in the middle, a ragged edge
				const float _edge = 0.8f * Wobble(_angle, 99u, 0.7f);
				const float _scorch = Smooth((_edge - _d) / _edge * 1.6f) * (0.7f + 0.3f * Wobble(_angle * 3.f + _d * 9.f, 31u, 0.6f));
				_put(2, _x, _y, _scorch);
			}

		_atlas = std::make_unique<sf::Texture>();
		if (!_atlas->loadFromImage(_image)) LOG_WARNING("StainComponent: the mark pictures could not be made");
		_atlas->setSmooth(true);
		return *_atlas;
	}

	// Draws the marks picture over the sprite, only where the sprite is: its shape (the triangles drawn) times the alpha of its picture.
	const char* VERTEX_SHADER = R"(
		uniform vec4 stainRect;
		varying vec2 stainUV;
		void main()
		{
			gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
			gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
			gl_FrontColor = gl_Color;
			stainUV = (gl_Vertex.xy - stainRect.xy) / stainRect.zw;
		})";

	const char* FRAGMENT_SHADER = R"(
		uniform sampler2D texture;
		uniform sampler2D stains;
		uniform float hasTexture;
		varying vec2 stainUV;
		void main()
		{
			float mask = gl_Color.a;
			if (hasTexture > 0.5) mask *= texture2D(texture, gl_TexCoord[0].xy).a;
			vec4 stain = texture2D(stains, vec2(stainUV.x, 1.0 - stainUV.y));
			gl_FragColor = stain * mask;
		})";

	sf::Shader* StainShader()
	{
		static std::unique_ptr<sf::Shader> _shader;
		static bool _tried = false;
		if (_tried) return _shader.get();
		_tried = true;
		if (!sf::Shader::isAvailable())
		{
			LOG_WARNING("StainComponent: this graphics card has no shaders, marks are not shown");
			return nullptr;
		}
		_shader = std::make_unique<sf::Shader>();
		if (!_shader->loadFromMemory(VERTEX_SHADER, FRAGMENT_SHADER))
		{
			LOG_WARNING("StainComponent: the marks shader could not be compiled, marks are not shown");
			_shader.reset();
		}
		return _shader.get();
	}
}

StainComponent::StainComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(StainComponent);
}

void StainComponent::Construct()
{
	Component::Construct();
	if (std::find(All().begin(), All().end(), this) == All().end()) All().push_back(this);
	if (SpriteComponent* _sprite = GetOwner()->GetComponent<SpriteComponent>()) _sprite->SetStainLayer(this);
}

void StainComponent::Deconstruct()
{
	Component::Deconstruct();
	std::erase(All(), this);
	if (SpriteComponent* _sprite = GetOwner()->GetComponent<SpriteComponent>())
		if (_sprite->GetStainLayer() == this) _sprite->SetStainLayer(nullptr);
}

void StainComponent::BeginPlay()
{
	Component::BeginPlay();
	Clear();	// the game starts clean
}

const sf::Shape* StainComponent::GetSpriteShape() const
{
	const SpriteComponent* _sprite = GetOwner()->GetComponent<SpriteComponent>();
	return _sprite && _sprite->GetShapeObject() && _sprite->GetShapeObject()->GetShape() ? _sprite->GetShapeObject()->GetShape()->Get() : nullptr;
}

float StainComponent::Random01()
{
	random ^= random << 13;
	random ^= random >> 17;
	random ^= random << 5;
	return static_cast<float>(random >> 8) * (1.f / 16777216.f);
}

void StainComponent::Stain(const FVector2& _worldPosition, const float& _radius, const Color& _color, const StainStyle& _style)
{
	if (strength <= 0.f || pending.size() >= MAX_PENDING) return;
	SpriteComponent* _sprite = GetOwner()->GetComponent<SpriteComponent>();
	const sf::Shape* _shape = GetSpriteShape();
	if (!_sprite || !_shape) return;
	if (_sprite->GetStainLayer() != this) _sprite->SetStainLayer(this);	// the sprite may have been added after this component

	// Kept in the sprite's own coordinates: the mark stays on it when it moves, turns or is scaled.
	const sf::Vector2f _scale = _shape->getScale();
	Stamp _stamp;
	_stamp.position = _shape->getInverseTransform().transformPoint(sf::Vector2f(_worldPosition.x, _worldPosition.y));
	_stamp.radius = _radius / std::max(0.0001f, std::sqrt(std::abs(_scale.x * _scale.y)));
	_stamp.color = sf::Color(_color.r, _color.g, _color.b, static_cast<std::uint8_t>(std::clamp(_color.a * strength, 0.f, 255.f)));
	_stamp.style = std::clamp(static_cast<int>(_style), 0, static_cast<int>(STYLE_COUNT) - 1);
	_stamp.rotation = Random01() * TWO_PI;
	pending.push_back(_stamp);
}

void StainComponent::Clear()
{
	pending.clear();
	hasStains = false;
	fadeCarry = 0.f;
	if (target)
	{
		target->clear(sf::Color::Transparent);
		target->display();
	}
}

bool StainComponent::Contains(const FVector2& _worldPosition) const
{
	const sf::Shape* _shape = GetSpriteShape();
	if (!_shape) return false;
	const sf::Vector2f _point(_worldPosition.x, _worldPosition.y);
	if (!_shape->getGlobalBounds().contains(_point)) return false;

	const sf::Vector2f _local = _shape->getInverseTransform().transformPoint(_point);
	const sf::FloatRect _bounds = _shape->getLocalBounds();
	if (!_bounds.contains(_local)) return false;

	const SpriteComponent* _sprite = GetOwner()->GetComponent<SpriteComponent>();
	switch (_sprite->GetShapeType())
	{
	case ShapeType::Circle:
	{
		const sf::Vector2f _offset = _local - _bounds.getCenter();
		const float _radius = _bounds.size.x * 0.5f;
		return _offset.x * _offset.x + _offset.y * _offset.y <= _radius * _radius;
	}
	case ShapeType::Polygon:
		return _sprite->GetShapeObject()->GetOutline().Contains(FVector2(_local.x, _local.y));
	default:
		return true;
	}
}

void StainComponent::GatherCandidates(Level* _level, const Actor* _ignore, std::vector<Candidate>& _out)
{
	_out.clear();
	for (StainComponent* _stain : All())
	{
		if (_stain->GetLevel() != _level || _stain->GetOwner() == _ignore || !_stain->IsActive() || !_stain->GetOwner()->IsActive() || _stain->strength <= 0.f) continue;
		if (const sf::Shape* _shape = _stain->GetSpriteShape())
			_out.push_back({ _stain, _shape->getGlobalBounds() });
	}
}

int StainComponent::StainAt(Level* _level, const FVector2& _worldPosition, const float& _radius, const Color& _color, const StainStyle& _style, const Actor* _ignore)
{
	int _count = 0;
	for (StainComponent* _stain : All())
	{
		if (_stain->GetLevel() != _level || _stain->GetOwner() == _ignore || !_stain->IsActive() || !_stain->GetOwner()->IsActive()) continue;
		const sf::Shape* _shape = _stain->GetSpriteShape();
		if (!_shape) continue;
		sf::FloatRect _bounds = _shape->getGlobalBounds();
		_bounds.position -= sf::Vector2f(_radius, _radius);
		_bounds.size += sf::Vector2f(_radius * 2.f, _radius * 2.f);
		if (!_bounds.contains(sf::Vector2f(_worldPosition.x, _worldPosition.y))) continue;
		_stain->Stain(_worldPosition, _radius, _color, _style);	// cut to the sprite's shape when drawn
		++_count;
	}
	return _count;
}

StainComponent* StainComponent::FindAt(Level* _level, const FVector2& _worldPosition, const Actor* _ignore)
{
	for (StainComponent* _stain : All())
		if (_stain->GetLevel() == _level && _stain->GetOwner() != _ignore && _stain->IsActive() && _stain->GetOwner()->IsActive() && _stain->Contains(_worldPosition))
			return _stain;
	return nullptr;
}

bool StainComponent::EnsureTarget(const sf::Shape& _shape)
{
	const sf::FloatRect _bounds = _shape.getLocalBounds();
	if (_bounds.size.x <= 0.f || _bounds.size.y <= 0.f) return false;
	if (target && _bounds == targetBounds) return true;

	// A new size of sprite: a new picture (the old marks would not fit it anymore).
	const float _resolution = std::clamp(resolution, 0.05f, 4.f);
	const sf::Vector2u _size(
		std::clamp(static_cast<unsigned>(std::ceil(_bounds.size.x * _resolution)), 4u, MAX_TARGET),
		std::clamp(static_cast<unsigned>(std::ceil(_bounds.size.y * _resolution)), 4u, MAX_TARGET));
	target = std::make_unique<sf::RenderTexture>();
	if (!target->resize(_size))
	{
		LOG_WARNING("StainComponent: no picture for the marks of \"" + GetOwner()->GetName().StdString() + "\"");
		target.reset();
		return false;
	}
	target->setSmooth(true);
	target->clear(sf::Color::Transparent);
	target->display();
	targetBounds = _bounds;
	hasStains = false;
	return true;
}

void StainComponent::PaintPending()
{
	if (pending.empty() || !target) return;

	const sf::Vector2f _size(target->getSize());
	const sf::Vector2f _toPixels(_size.x / targetBounds.size.x, _size.y / targetBounds.size.y);
	stampVertices.resize(pending.size() * 6);

	sf::Vertex* _v = stampVertices.data();
	for (const Stamp& _stamp : pending)
	{
		const sf::Vector2f _center((_stamp.position.x - targetBounds.position.x) * _toPixels.x, (_stamp.position.y - targetBounds.position.y) * _toPixels.y);
		const float _cos = std::cos(_stamp.rotation), _sin = std::sin(_stamp.rotation);
		const sf::Vector2f _a(_cos * _stamp.radius * _toPixels.x, _sin * _stamp.radius * _toPixels.y);
		const sf::Vector2f _b(-_sin * _stamp.radius * _toPixels.x, _cos * _stamp.radius * _toPixels.y);
		const float _u = static_cast<float>(_stamp.style * CELL);
		const sf::Vertex _c0{ _center - _a - _b, _stamp.color, { _u, 0.f } };
		const sf::Vertex _c1{ _center + _a - _b, _stamp.color, { _u + CELL, 0.f } };
		const sf::Vertex _c2{ _center + _a + _b, _stamp.color, { _u + CELL, static_cast<float>(CELL) } };
		const sf::Vertex _c3{ _center - _a + _b, _stamp.color, { _u, static_cast<float>(CELL) } };
		_v[0] = _c0; _v[1] = _c1; _v[2] = _c2;
		_v[3] = _c0; _v[4] = _c2; _v[5] = _c3;
		_v += 6;
	}

	sf::RenderStates _states;
	_states.texture = &StampAtlas();
	_states.blendMode = sf::BlendAlpha;	// stores color * alpha: the picture is premultiplied
	target->draw(stampVertices.data(), stampVertices.size(), sf::PrimitiveType::Triangles, _states);	// every waiting mark at once
	target->display();
	pending.clear();
	hasStains = true;
	fadeCarry = 0.f;
	fadeLeft = 255.f;
}

void StainComponent::Fade(const float& _deltaTime)
{
	if (fadeSpeed <= 0.f || !hasStains || !target) return;

	// Whole steps of 1/255 only: smaller ones would be lost in the 8 bits of the picture.
	fadeCarry += fadeSpeed * std::min(_deltaTime, 0.1f) * 255.f;
	const float _steps = std::floor(fadeCarry);
	if (_steps < 1.f) return;
	fadeCarry -= _steps;

	const std::uint8_t _amount = static_cast<std::uint8_t>(std::min(_steps, 255.f));
	const sf::Vector2f _size(target->getSize());
	const sf::Color _off(_amount, _amount, _amount, _amount);
	const sf::Vertex _quad[6] = {
		{ { 0.f, 0.f }, _off }, { { _size.x, 0.f }, _off }, { _size, _off },
		{ { 0.f, 0.f }, _off }, { _size, _off }, { { 0.f, _size.y }, _off } };
	sf::RenderStates _states;
	_states.blendMode = FADE;
	target->draw(_quad, 6, sf::PrimitiveType::Triangles, _states);
	target->display();

	fadeLeft -= _amount;
	if (fadeLeft <= 0.f) Clear();	// all gone: nothing more to draw
}

void StainComponent::DrawOver(Window& _window, const sf::Shape& _shape)
{
	const float _deltaTime = fadeClock.restart().asSeconds();
	if (!isActive || !HasStains()) return;
	if (!EnsureTarget(_shape)) return;
	PaintPending();
	Fade(_deltaTime);

	sf::Shader* _shader = StainShader();
	if (!_shader) return;
	_shader->setUniform("stains", target->getTexture());
	_shader->setUniform("stainRect", sf::Glsl::Vec4(targetBounds.position.x, targetBounds.position.y, targetBounds.size.x, targetBounds.size.y));
	_shader->setUniform("hasTexture", _shape.getTexture() ? 1.f : 0.f);
	_shader->setUniform("texture", sf::Shader::CurrentTexture);

	sf::RenderStates _states;
	_states.shader = _shader;
	_states.blendMode = PREMULTIPLIED;
	_window.GetRenderWindow().draw(_shape, _states);	// the sprite's own triangles: the marks are cut to its shape
}

std::string StainComponent::ToString() const
{
	return name + (HasStains() ? " -> marked" : " -> clean");
}
