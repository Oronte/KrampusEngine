#include "ParticleEmitter.h"
#include <algorithm>
#include <atomic>
#include <cmath>

using namespace Krampus;

namespace
{
	constexpr float			DEG_TO_RAD		= 3.14159265f / 180.f;
	constexpr int			POOL_LIMIT		= 50000;	// an emitter never takes more than this
	constexpr float			MAX_STEP		= 0.1f;		// a frame longer than this (a hitch, a breakpoint) is cut: no huge jump
	std::atomic<int>		totalAlive		= 0;

	std::uint32_t NextSeed()
	{
		static std::uint32_t _seed = 0x9E3779B9u;
		_seed = _seed * 1664525u + 1013904223u;
		return _seed | 1u;	// xorshift needs a seed that is not zero
	}

	std::uint8_t Mix(const std::uint8_t& _a, const std::uint8_t& _b, const float& _t)
	{
		return static_cast<std::uint8_t>(_a + (_b - _a) * _t);
	}
}

void ParticleSettings::ApplyPreset(const ParticlePreset& _preset)
{
	if (_preset == ParticlePreset::Custom) return;
	const std::string _texture = texture;	// a picture chosen by the user is kept
	*this = ParticleSettings();
	texture = _texture;

	switch (_preset)
	{
	case ParticlePreset::Fire:
		break;	// the defaults are a fire

	case ParticlePreset::Smoke:
		rate = 14.f;			lifetimeMin = 2.f;		lifetimeMax = 3.5f;
		speedMin = 20.f;		speedMax = 45.f;		spread = 30.f;
		sizeStart = 20.f;		sizeEnd = 70.f;
		rotationMin = 0.f;		rotationMax = 360.f;	spinMin = -30.f;	spinMax = 30.f;
		colorStart = Color(130, 130, 130, 140);			colorEnd = Color(60, 60, 60, 0);
		gravity = FVector2(0.f, -15.f);					drag = 0.3f;
		blend = static_cast<int>(ParticleBlend::Alpha);
		break;

	case ParticlePreset::Sparks:
		rate = 0.f;				burst = 40;				duration = 1.f;
		shape = static_cast<int>(ParticleShape::Point);
		lifetimeMin = 0.4f;		lifetimeMax = 0.9f;
		speedMin = 200.f;		speedMax = 420.f;		spread = 160.f;
		sizeStart = 5.f;		sizeEnd = 1.f;			stretch = 0.03f;
		colorStart = Color(255, 230, 150, 255);			colorEnd = Color(255, 120, 20, 0);
		gravity = FVector2(0.f, 600.f);					drag = 1.f;
		break;

	case ParticlePreset::Explosion:
		looping = false;		duration = 0.1f;		rate = 0.f;			burst = 120;
		shapeRadius = 6.f;		radial = true;
		lifetimeMin = 0.5f;		lifetimeMax = 1.2f;
		speedMin = 80.f;		speedMax = 380.f;
		sizeStart = 30.f;		sizeEnd = 4.f;			sizeRandom = 0.5f;
		colorStart = Color(255, 220, 120, 255);			colorEnd = Color(200, 40, 10, 0);
		gravity = FVector2(0.f, 0.f);					drag = 2.5f;
		break;

	case ParticlePreset::Snow:
		shape = static_cast<int>(ParticleShape::Line);	shapeSize = FVector2(800.f, 0.f);
		rate = 40.f;			maxParticles = 600;
		lifetimeMin = 6.f;		lifetimeMax = 9.f;
		speedMin = 30.f;		speedMax = 60.f;		direction = 90.f;	spread = 20.f;
		sizeStart = 6.f;		sizeEnd = 6.f;			sizeRandom = 0.5f;
		spinMin = -40.f;		spinMax = 40.f;
		colorStart = Color(255, 255, 255, 230);			colorEnd = Color(255, 255, 255, 0);
		gravity = FVector2(0.f, 5.f);					drag = 0.f;
		blend = static_cast<int>(ParticleBlend::Alpha);
		break;

	case ParticlePreset::Rain:
		shape = static_cast<int>(ParticleShape::Line);	shapeSize = FVector2(800.f, 0.f);
		rate = 150.f;			maxParticles = 400;
		lifetimeMin = 1.2f;		lifetimeMax = 1.6f;
		speedMin = 600.f;		speedMax = 800.f;		direction = 100.f;	spread = 2.f;
		sizeStart = 3.f;		sizeEnd = 3.f;			sizeRandom = 0.f;	stretch = 0.03f;
		colorStart = Color(160, 180, 255, 180);			colorEnd = Color(160, 180, 255, 120);
		gravity = FVector2(0.f, 0.f);					drag = 0.f;
		blend = static_cast<int>(ParticleBlend::Alpha);
		break;

	case ParticlePreset::Magic:
		shapeRadius = 40.f;		fromEdge = true;		radial = true;
		rate = 50.f;			lifetimeMin = 1.f;		lifetimeMax = 1.6f;
		speedMin = 10.f;		speedMax = 30.f;
		sizeStart = 10.f;		sizeEnd = 0.f;
		spinMin = -90.f;		spinMax = 90.f;
		colorStart = Color(140, 90, 255, 255);			colorEnd = Color(60, 200, 255, 0);
		gravity = FVector2(0.f, -30.f);					drag = 0.f;
		break;

	default:
		break;
	}
}

ParticleEmitter::ParticleEmitter()
	: random(NextSeed())
{
}

ParticleEmitter::~ParticleEmitter()
{
	totalAlive -= static_cast<int>(alive);
}

int ParticleEmitter::GetTotalAlive()
{
	return totalAlive;
}

float ParticleEmitter::Random01()
{
	// xorshift32: a few instructions, enough for effects.
	random ^= random << 13;
	random ^= random >> 17;
	random ^= random << 5;
	return static_cast<float>(random >> 8) * (1.f / 16777216.f);
}

float ParticleEmitter::Range(const float& _min, const float& _max)
{
	return _min + (_max - _min) * Random01();
}

void ParticleEmitter::Play()
{
	playing = true;
	cycleTime = 0.f;
	emitCarry = 0.f;
	burstPending = true;
	wasAlive = true;
}

void ParticleEmitter::Stop()
{
	playing = false;
	burstPending = false;
}

void ParticleEmitter::Clear()
{
	totalAlive -= static_cast<int>(alive);
	alive = 0;
	bounds = sf::FloatRect();
}

void ParticleEmitter::Burst(const int& _count)
{
	ResizePool();
	const FVector2 _position = hasLast ? lastPosition : FVector2(0.f, 0.f);
	for (int _i = 0; _i < _count; ++_i)
		Spawn(_position, lastRotation, lastScale);
	if (_count > 0) wasAlive = true;
}

void ParticleEmitter::ResizePool()
{
	const std::size_t _size = static_cast<std::size_t>(std::clamp(settings.maxParticles, 1, POOL_LIMIT));
	if (particles.size() == _size) return;
	if (alive > _size)
	{
		totalAlive -= static_cast<int>(alive - _size);
		alive = _size;
	}
	particles.resize(_size);	// only when maxParticles changes, never while playing
}

void ParticleEmitter::Spawn(const FVector2& _position, const float& _rotation, const FVector2& _scale)
{
	if (alive >= particles.size()) return;	// pool full: the effect is as big as it may be

	const ParticleSettings& _s = settings;
	const float _turn = _rotation * DEG_TO_RAD;
	const float _cos = std::cos(_turn), _sin = std::sin(_turn);

	// Where in the shape, in the emitter's space.
	float _x = 0.f, _y = 0.f;
	switch (static_cast<ParticleShape>(_s.shape))
	{
	case ParticleShape::Circle:
	{
		const float _angle = Random01() * 6.2831853f;
		const float _radius = _s.fromEdge ? _s.shapeRadius : _s.shapeRadius * std::sqrt(Random01());	// sqrt: even inside the disc
		_x = std::cos(_angle) * _radius;
		_y = std::sin(_angle) * _radius;
		break;
	}
	case ParticleShape::Rectangle:
	{
		const float _w = _s.shapeSize.x, _h = _s.shapeSize.y;
		if (_s.fromEdge && _w + _h > 0.f)
		{
			float _along = Random01() * 2.f * (_w + _h);	// a point on the outline, every side by its length
			if (_along < _w)				{ _x = _along - _w * 0.5f;				_y = -_h * 0.5f; }
			else if ((_along -= _w) < _h)	{ _x = _w * 0.5f;						_y = _along - _h * 0.5f; }
			else if ((_along -= _h) < _w)	{ _x = _w * 0.5f - _along;				_y = _h * 0.5f; }
			else							{ _x = -_w * 0.5f;						_y = _h * 0.5f - (_along - _w); }
		}
		else
		{
			_x = (Random01() - 0.5f) * _w;
			_y = (Random01() - 0.5f) * _h;
		}
		break;
	}
	case ParticleShape::Line:
		_x = (Random01() - 0.5f) * _s.shapeSize.x;
		break;
	default:
		break;
	}
	_x *= _scale.x;
	_y *= _scale.y;
	const float _offsetX = _x * _cos - _y * _sin;
	const float _offsetY = _x * _sin + _y * _cos;

	// Which way it goes.
	float _dirX, _dirY;
	const float _length = std::sqrt(_offsetX * _offsetX + _offsetY * _offsetY);
	if (_s.radial && _length > 0.0001f)
	{
		_dirX = _offsetX / _length;
		_dirY = _offsetY / _length;
	}
	else
	{
		const float _angle = _s.radial ? Random01() * 6.2831853f : (_s.direction + _rotation + (Random01() - 0.5f) * _s.spread) * DEG_TO_RAD;
		_dirX = std::cos(_angle);
		_dirY = std::sin(_angle);
	}
	const float _speed = Range(_s.speedMin, _s.speedMax);

	Particle& _p = particles[alive++];
	++totalAlive;
	_p.position		= sf::Vector2f((_s.worldSpace ? _position.x : 0.f) + _offsetX, (_s.worldSpace ? _position.y : 0.f) + _offsetY);
	_p.velocity		= sf::Vector2f(_dirX * _speed, _dirY * _speed);
	_p.age			= 0.f;
	_p.inverseLife	= 1.f / std::max(0.01f, Range(_s.lifetimeMin, _s.lifetimeMax));
	_p.scale		= 1.f - std::clamp(_s.sizeRandom, 0.f, 1.f) * Random01();
	_p.rotation		= Range(_s.rotationMin, _s.rotationMax);
	_p.spin			= Range(_s.spinMin, _s.spinMax);
}

bool ParticleEmitter::Update(float _deltaTime, const FVector2& _position, const float& _rotation, const FVector2& _scale)
{
	ResizePool();
	if (!hasLast) { lastPosition = _position; lastRotation = _rotation; lastScale = _scale; hasLast = true; }
	_deltaTime = std::min(_deltaTime, MAX_STEP);
	if (_deltaTime <= 0.f)
	{
		lastPosition = _position;
		return false;
	}
	const ParticleSettings& _s = settings;

	// ---- Emit. Particles of one frame are spread on the way the emitter went: a fast emitter leaves a trail, not clumps.
	if (playing)
	{
		int _count = 0;
		if (burstPending)
		{
			_count += std::max(0, _s.burst);
			burstPending = false;
		}
		emitCarry += std::max(0.f, _s.rate) * _deltaTime;
		const int _fromRate = static_cast<int>(emitCarry);
		emitCarry -= static_cast<float>(_fromRate);
		_count += _fromRate;

		for (int _i = 0; _i < _count; ++_i)
		{
			const float _t = (_i + 1.f) / static_cast<float>(_count);
			const FVector2 _at(lastPosition.x + (_position.x - lastPosition.x) * _t, lastPosition.y + (_position.y - lastPosition.y) * _t);
			Spawn(_s.worldSpace ? _at : _position, _rotation, _scale);
		}

		cycleTime += _deltaTime;
		if (cycleTime >= std::max(0.01f, _s.duration))
		{
			if (_s.looping)
			{
				cycleTime = std::fmod(cycleTime, std::max(0.01f, _s.duration));
				burstPending = _s.burst > 0;
			}
			else playing = false;
		}
	}
	lastPosition = _position;
	lastRotation = _rotation;
	lastScale = _scale;

	// ---- Live. One pass: age, move, and swap the dead ones out (the order of particles does not matter).
	const float _keep = std::max(0.f, 1.f - _s.drag * _deltaTime);
	const sf::Vector2f _gravity(_s.gravity.x * _deltaTime, _s.gravity.y * _deltaTime);
	const float _biggest = std::max(_s.sizeStart, _s.sizeEnd) * 0.5f;
	float _minX = 1e30f, _minY = 1e30f, _maxX = -1e30f, _maxY = -1e30f, _maxSpeed = 0.f;

	std::size_t _i = 0;
	while (_i < alive)
	{
		Particle& _p = particles[_i];
		_p.age += _deltaTime;
		if (_p.age * _p.inverseLife >= 1.f)
		{
			_p = particles[--alive];
			--totalAlive;
			continue;
		}
		_p.velocity = (_p.velocity + _gravity) * _keep;
		_p.position += _p.velocity * _deltaTime;
		_p.rotation += _p.spin * _deltaTime;

		_minX = std::min(_minX, _p.position.x);		_maxX = std::max(_maxX, _p.position.x);
		_minY = std::min(_minY, _p.position.y);		_maxY = std::max(_maxY, _p.position.y);
		if (_s.stretch > 0.f) _maxSpeed = std::max(_maxSpeed, std::abs(_p.velocity.x) + std::abs(_p.velocity.y));
		++_i;
	}

	if (alive == 0) bounds = sf::FloatRect();
	else
	{
		const float _margin = _biggest + _maxSpeed * _s.stretch;
		bounds = sf::FloatRect({ _minX - _margin, _minY - _margin }, { _maxX - _minX + _margin * 2.f, _maxY - _minY + _margin * 2.f });
	}

	if (wasAlive && !playing && alive == 0)
	{
		wasAlive = false;
		return true;
	}
	return false;
}

std::size_t ParticleEmitter::BuildVertices(std::vector<sf::Vertex>& _out, const sf::Vector2f& _offset, const sf::Vector2f& _textureSize) const
{
	const std::size_t _count = alive * 6;
	if (_out.size() < _count) _out.resize(_count);	// grows to the biggest the effect has been, then stays
	if (_count == 0) return 0;

	const ParticleSettings& _s = settings;
	const Color& _c0 = _s.colorStart;
	const Color& _c1 = _s.colorEnd;
	const sf::Vector2f _uv0(0.f, 0.f), _uv1(_textureSize.x, 0.f), _uv2(_textureSize.x, _textureSize.y), _uv3(0.f, _textureSize.y);

	sf::Vertex* _v = _out.data();
	for (std::size_t _i = 0; _i < alive; ++_i)
	{
		const Particle& _p = particles[_i];
		const float _t = _p.age * _p.inverseLife;
		const float _half = (_s.sizeStart + (_s.sizeEnd - _s.sizeStart) * _t) * _p.scale * 0.5f;
		const sf::Color _color(Mix(_c0.r, _c1.r, _t), Mix(_c0.g, _c1.g, _t), Mix(_c0.b, _c1.b, _t), Mix(_c0.a, _c1.a, _t));

		// Two axes of the quad: along the speed when stretched, else turned by the particle's rotation.
		float _ax, _ay, _bx, _by;
		if (_s.stretch > 0.f)
		{
			const float _speed = std::sqrt(_p.velocity.x * _p.velocity.x + _p.velocity.y * _p.velocity.y);
			const float _dx = _speed > 0.0001f ? _p.velocity.x / _speed : 1.f;
			const float _dy = _speed > 0.0001f ? _p.velocity.y / _speed : 0.f;
			const float _long = _half + _speed * _s.stretch * 0.5f;
			_ax = _dx * _long;		_ay = _dy * _long;
			_bx = -_dy * _half;		_by = _dx * _half;
		}
		else if (_p.rotation != 0.f)
		{
			const float _cos = std::cos(_p.rotation * DEG_TO_RAD), _sin = std::sin(_p.rotation * DEG_TO_RAD);
			_ax = _cos * _half;		_ay = _sin * _half;
			_bx = -_sin * _half;	_by = _cos * _half;
		}
		else
		{
			_ax = _half;	_ay = 0.f;
			_bx = 0.f;		_by = _half;
		}

		const sf::Vector2f _center = _p.position + _offset;
		const sf::Vector2f _corner0(_center.x - _ax - _bx, _center.y - _ay - _by);
		const sf::Vector2f _corner1(_center.x + _ax - _bx, _center.y + _ay - _by);
		const sf::Vector2f _corner2(_center.x + _ax + _bx, _center.y + _ay + _by);
		const sf::Vector2f _corner3(_center.x - _ax + _bx, _center.y - _ay + _by);

		_v[0] = sf::Vertex{ _corner0, _color, _uv0 };
		_v[1] = sf::Vertex{ _corner1, _color, _uv1 };
		_v[2] = sf::Vertex{ _corner2, _color, _uv2 };
		_v[3] = sf::Vertex{ _corner0, _color, _uv0 };
		_v[4] = sf::Vertex{ _corner2, _color, _uv2 };
		_v[5] = sf::Vertex{ _corner3, _color, _uv3 };
		_v += 6;
	}
	return _count;
}
