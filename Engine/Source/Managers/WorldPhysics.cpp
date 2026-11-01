#include "WorldPhysics.h"

#include "LevelManager.h"
#include "Core/Engine.h"
#include "GameFramework/Level.h"
#include "Components/PhysicsComponent.h"
#include <algorithm>
#include <cmath>

Krampus::WorldPhysics::WorldPhysics(Engine *_engine)
	: KrampusObject(_engine)
{
	b2WorldDef _physicsWorld = b2DefaultWorldDef();
	_physicsWorld.gravity *= -1.0f;
	physicsWorldID = b2CreateWorld(&_physicsWorld);
}

Krampus::WorldPhysics::~WorldPhysics()
{
	b2DestroyWorld(physicsWorldID);
}

void Krampus::WorldPhysics::Update()
{
	Level* _level = GetWorld()->GetLevelManager()->GetCurrentLevel();
	if (!_level) return;

	// box2d is stable with small steps: a long frame is cut into equal steps of at most MaxStepTime (no body through a wall, no
	// explosion after a hitch), and each frame uses all of its time (no stutter at a high frame rate, as a fixed step would give).
	const float _deltaTime = _level->GetTimerManager().GetDeltaTime();
	const int _steps = std::clamp(static_cast<int>(std::ceil(_deltaTime / MaxStepTime)), 1, MaxSteps);
	for (int _i = 0; _i < _steps; ++_i)
		b2World_Step(physicsWorldID, _deltaTime / static_cast<float>(_steps), subStepsCount);

	onResolveCollision.Broadcast();
}


void Krampus::WorldPhysics::Explode(ExplosionInfo _info)
{
	b2ExplosionDef _explosionDef = b2DefaultExplosionDef();
	const FVector2& _pos = _info.position / PX_TO_M;
	_explosionDef.position = b2Vec2(_pos.x, _pos.y);
	_explosionDef.radius = _info.radius / PX_TO_M;
	_explosionDef.falloff = _info.falloff;
	_explosionDef.impulsePerLength = _info.impulsePerLength;
	_explosionDef.maskBits = CAST(uint64_t, _info.channel);

	b2World_Explode(physicsWorldID, &_explosionDef);
}

// ---- Queries
namespace
{
	using namespace Krampus;

	// The query hits a shape whose category is in _channels (the shape's own mask is not asked: a ray is not a body).
	b2QueryFilter QueryFilter(const CollisionChannel& _channels)
	{
		b2QueryFilter _filter = b2DefaultQueryFilter();
		_filter.categoryBits = ~0ull;
		_filter.maskBits = static_cast<uint64_t>(_channels);
		return _filter;
	}

	PhysicsComponent* ComponentOf(const b2ShapeId _shape)
	{
		return static_cast<PhysicsComponent*>(b2Body_GetUserData(b2Shape_GetBody(_shape)));
	}

	struct RayContext
	{
		const Actor*					ignore	= nullptr;
		FVector2						from;
		float							length	= 0.f;
		bool							all		= false;
		std::vector<RaycastHitInfo>		hits;
	};

	float OnRayHit(b2ShapeId _shape, b2Vec2 _point, b2Vec2 _normal, float _fraction, void* _context)
	{
		RayContext& _ray = *static_cast<RayContext*>(_context);
		PhysicsComponent* _component = ComponentOf(_shape);
		if (_ray.ignore && _component && _component->GetOwner() == _ray.ignore) return -1.f;	// through it

		RaycastHitInfo _hit;
		_hit.hit = true;
		_hit.point = FVector2(_point.x, _point.y) * PX_TO_M;
		_hit.normal = FVector2(_normal.x, _normal.y);
		_hit.distance = _fraction * _ray.length;
		_hit.component = _component;
		_hit.actor = _component ? _component->GetOwner() : nullptr;
		if (_ray.all) { _ray.hits.push_back(_hit); return 1.f; }	// keep going
		_ray.hits.assign(1, _hit);
		return _fraction;	// only nearer hits from now on
	}

	bool OnOverlap(b2ShapeId _shape, void* _context)
	{
		auto& _found = *static_cast<std::vector<PhysicsComponent*>*>(_context);
		if (PhysicsComponent* _component = ComponentOf(_shape))
			if (std::find(_found.begin(), _found.end(), _component) == _found.end()) _found.push_back(_component);	// one per body
		return true;
	}

	std::vector<RaycastHitInfo> CastRay(b2WorldId _world, const FVector2& _from, const FVector2& _to, const CollisionChannel& _channels, const Actor* _ignore, const bool _all)
	{
		RayContext _ray;
		_ray.ignore = _ignore;
		_ray.from = _from;
		_ray.all = _all;
		const FVector2 _delta = _to - _from;
		_ray.length = std::sqrt(_delta.x * _delta.x + _delta.y * _delta.y);
		if (_ray.length <= 0.f) return {};
		const FVector2 _origin = _from / PX_TO_M, _translation = _delta / PX_TO_M;
		b2World_CastRay(_world, b2Vec2(_origin.x, _origin.y), b2Vec2(_translation.x, _translation.y), QueryFilter(_channels), &OnRayHit, &_ray);
		if (_all) std::sort(_ray.hits.begin(), _ray.hits.end(), [](const RaycastHitInfo& _a, const RaycastHitInfo& _b) { return _a.distance < _b.distance; });
		return _ray.hits;
	}
}

Krampus::RaycastHitInfo Krampus::WorldPhysics::Raycast(const FVector2& _from, const FVector2& _to, const CollisionChannel& _channels, const Actor* _ignore) const
{
	std::vector<RaycastHitInfo> _hits = CastRay(physicsWorldID, _from, _to, _channels, _ignore, false);
	return _hits.empty() ? RaycastHitInfo() : _hits.front();
}

std::vector<Krampus::RaycastHitInfo> Krampus::WorldPhysics::RaycastAll(const FVector2& _from, const FVector2& _to, const CollisionChannel& _channels, const Actor* _ignore) const
{
	return CastRay(physicsWorldID, _from, _to, _channels, _ignore, true);
}

std::vector<Krampus::PhysicsComponent*> Krampus::WorldPhysics::OverlapBox(const FVector2& _center, const FVector2& _size, const CollisionChannel& _channels) const
{
	const FVector2 _half = _size * 0.5f / PX_TO_M, _c = _center / PX_TO_M;
	const b2Vec2 _points[4] = { { _c.x - _half.x, _c.y - _half.y }, { _c.x + _half.x, _c.y - _half.y }, { _c.x + _half.x, _c.y + _half.y }, { _c.x - _half.x, _c.y + _half.y } };
	const b2ShapeProxy _proxy = b2MakeProxy(_points, 4, 0.f);	// the shapes really touching the box, not only their bounding boxes
	std::vector<PhysicsComponent*> _found;
	b2World_OverlapShape(physicsWorldID, &_proxy, QueryFilter(_channels), &OnOverlap, &_found);
	return _found;
}

std::vector<Krampus::PhysicsComponent*> Krampus::WorldPhysics::OverlapCircle(const FVector2& _center, const Float& _radius, const CollisionChannel& _channels) const
{
	const FVector2 _c = _center / PX_TO_M;
	const b2Vec2 _point(_c.x, _c.y);
	const b2ShapeProxy _proxy = b2MakeProxy(&_point, 1, static_cast<float>(_radius) / PX_TO_M);
	std::vector<PhysicsComponent*> _found;
	b2World_OverlapShape(physicsWorldID, &_proxy, QueryFilter(_channels), &OnOverlap, &_found);
	return _found;
}
