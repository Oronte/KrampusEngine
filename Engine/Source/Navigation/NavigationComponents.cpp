#include "NavigationComponents.h"
#include "Actors/Actor.h"
#include "Components/PhysicsComponent.h"
#include "Components/SpriteComponent.h"
#include "Core/Engine.h"
#include "GameFramework/Level.h"
#include "Graphics/Shapes/ShapeAssets.h"
#include "Utilities/Debug/Debug.h"
#include "Graphics/Ressources/Vertices.h"
#include <cmath>

using namespace Krampus;

namespace
{
	float Length(const FVector2& _v)	{ return std::sqrt(_v.x * _v.x + _v.y * _v.y); }

	// Points of the actor's own space into the world: scaled, turned, moved.
	std::vector<FVector2> ToWorld(const std::vector<FVector2>& _points, const FVector2& _position, const float _radians, const FVector2& _scale)
	{
		const float _cos = std::cos(_radians), _sin = std::sin(_radians);
		std::vector<FVector2> _world;
		_world.reserve(_points.size());
		for (const FVector2& _p : _points)
		{
			const float _x = _p.x * _scale.x, _y = _p.y * _scale.y;
			_world.emplace_back(_position.x + _x * _cos - _y * _sin, _position.y + _x * _sin + _y * _cos);
		}
		return _world;
	}

	bool IsAlive(Level* _level, const Actor* _actor)
	{
		if (!_level || !_actor) return false;
		for (const std::unique_ptr<Actor>& _candidate : _level->GetActorManager().GetActors())
			if (_candidate.get() == _actor) return !_candidate->IsPendingDestroy();
		return false;
	}

	bool Editing(const Component& _component)
	{
		return _component.GetWorld() && _component.GetWorld()->IsLevelPaused();
	}
}

bool Krampus::NavShapeOf(Actor& _actor, NavSource& _source)
{
	const FVector2 _position = _actor.GetPosition();
	const FVector2 _scale = _actor.GetScale();
	const float _radians = static_cast<float>(_actor.GetRotation().ToRadians());

	if (const PhysicsComponent* _body = _actor.GetComponent<PhysicsComponent>())
	{
		switch (_body->GetShapeType())
		{
		case ShapeType::Circle:
			_source.circle = true;
			_source.center = _position;
			_source.radius = _body->GetCircleRadius() * std::max(std::abs(_scale.x), std::abs(_scale.y));
			break;
		case ShapeType::Rectangle:
			_source.polygon = ToWorld(Polygon2D::Box(_body->GetBoxSize()).points, _position, _radians, _scale);
			break;
		default:
			_source.polygon = ToWorld(ShapeAssets::Get(_body->GetPolygonName()).points, _position, _radians, _scale);
			break;
		}
		_source.ComputeBounds();
		return true;
	}
	if (const SpriteComponent* _sprite = _actor.GetComponent<SpriteComponent>())
	{
		if (_sprite->GetShapeType() == ShapeType::Circle)
		{
			_source.circle = true;
			_source.center = _position;
			_source.radius = _sprite->GetRadius() * std::max(std::abs(_scale.x), std::abs(_scale.y));
		}
		else _source.polygon = ToWorld(_sprite->GetShapeObject()->GetOutline().points, _position, _radians, _scale);
		_source.ComputeBounds();
		return true;
	}
	return false;
}

// ================= NavMeshBoundsComponent =================

NavMeshBoundsComponent::NavMeshBoundsComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(NavMeshBoundsComponent);
	UseDebug();	// drawn when the navigation is shown (P in the editor)
}

void NavMeshBoundsComponent::Construct()
{
	Component::Construct();
	GetLevel()->GetNavigation().AddBounds(this);
}

void NavMeshBoundsComponent::Deconstruct()
{
	Component::Deconstruct();
	GetLevel()->GetNavigation().RemoveBounds(this);
}

void NavMeshBoundsComponent::Tick(const Float& _deltaTime)
{
	Component::Tick(_deltaTime);
	GetLevel()->GetNavigation().Update();
}

NavMeshSettings NavMeshBoundsComponent::GetSettings() const
{
	NavMeshSettings _settings;
	const FVector2 _size(std::max(16.f, std::abs(size.x)), std::max(16.f, std::abs(size.y)));
	_settings.min = FVector2(GetOwner()->GetPosition().x - _size.x * 0.5f, GetOwner()->GetPosition().y - _size.y * 0.5f);
	_settings.size = _size;
	_settings.cellSize = std::clamp(cellSize, 2.f, 128.f);
	_settings.agentRadius = std::max(0.f, agentRadius);
	_settings.tileSize = std::clamp(tileSize, 8, 128);
	_settings.erodeEdges = erodeEdges;
	return _settings;
}

void NavMeshBoundsComponent::Rebuild()
{
	GetLevel()->GetNavigation().RebuildAll();
	GetLevel()->GetNavigation().Update();
}

const NavMesh* NavMeshBoundsComponent::GetMesh() const
{
	return GetLevel()->GetNavigation().GetMesh(this);
}

std::string NavMeshBoundsComponent::ToString() const
{
	const NavMesh* _mesh = GetMesh();
	if (!_mesh) return name + " -> not built";
	return name + " -> " + std::to_string(_mesh->GetPolys().size()) + " polygons, " + std::to_string(_mesh->GetWalkableCells()) + " walkable cells, last build "
		+ std::to_string(_mesh->GetLastBuildMs()) + " ms";
}

void NavMeshBoundsComponent::DrawDebug()
{
#ifdef DEBUG
	NavigationSystem& _navigation = GetLevel()->GetNavigation();
	const bool _editing = Editing(*this);
	if (_editing) _navigation.Update();	// the editor does not tick the level: the mesh follows the changes here

	if (_editing || NavigationSystem::showNavigation)
	{
		const NavMeshSettings _s = GetSettings();
		Debug::DrawDebugRect(GetLevel(), GetOwner()->GetPosition(), _s.size, Angle(), Color(70, 200, 120, 255));
	}
	if (!NavigationSystem::showNavigation) return;
	const NavMesh* _mesh = GetMesh();
	if (!_mesh) return;

	// The polygons in one batch (filled, colored by area cost), their borders in another, the links in a third.
	auto _fill = std::make_shared<Vertices>(PrimitiveType::Triangles);
	auto _lines = std::make_shared<Vertices>(PrimitiveType::Lines);
	auto _links = std::make_shared<Vertices>(PrimitiveType::Lines);
	for (const NavPoly& _poly : _mesh->GetPolys())
	{
		const float _cost = _mesh->AreaCost(_poly.area);
		const Color _color = _cost > 1.001f ? Color(235, 150, 40, static_cast<std::uint8_t>(std::clamp(50.f + _cost * 10.f, 60.f, 140.f)))
			: _cost < 0.999f ? Color(60, 150, 255, 80) : Color(40, 200, 90, 55);
		const FVector2 _a = _mesh->PolyMin(_poly), _c = _mesh->PolyMax(_poly);
		const FVector2 _b(_c.x, _a.y), _d(_a.x, _c.y);
		for (const FVector2& _p : { _a, _b, _c, _a, _c, _d }) _fill->Append(_p, _color);
		const Color _edge(30, 140, 70, 150);
		for (const FVector2& _p : { _a, _b, _b, _c, _c, _d, _d, _a }) _lines->Append(_p, _edge);
	}
	for (const NavEdge& _edge : _mesh->GetEdges())
		if (_edge.link >= 0)
		{
			_links->Append(_edge.a, Color(200, 90, 255, 255));
			_links->Append(_edge.b, Color(255, 200, 255, 255));
		}
	GetLevel()->GetCameraManager().PushDebug(_fill);
	GetLevel()->GetCameraManager().PushDebug(_lines);
	GetLevel()->GetCameraManager().PushDebug(_links);
#endif
}

// ================= NavModifierComponent =================

NavModifierComponent::NavModifierComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(NavModifierComponent);
	UseDebug();
}

bool NavModifierComponent::MakeSource(NavSource& _source) const
{
	_source.blocked = static_cast<NavArea>(area) == NavArea::Blocked;
	_source.cost = std::max(0.05f, cost);
	if (useOwnerShape) return NavShapeOf(*GetOwner(), _source);

	const FVector2 _position = GetOwner()->GetPosition();
	if (shape == 1)
	{
		_source.circle = true;
		_source.center = _position;
		_source.radius = std::max(1.f, radius);
	}
	else _source.polygon = ToWorld(Polygon2D::Box(size).points, _position, static_cast<float>(GetOwner()->GetRotation().ToRadians()), FVector2(1.f, 1.f));
	_source.ComputeBounds();
	return true;
}

void NavModifierComponent::DrawDebug()
{
#ifdef DEBUG
	if (!NavigationSystem::showNavigation) return;
	NavSource _source;
	if (!MakeSource(_source)) return;
	const Color _color = _source.blocked ? Color(230, 60, 60, 255) : Color(235, 150, 40, 255);
	if (_source.circle) Debug::DrawDebugCircle(GetLevel(), _source.center, _source.radius, 24, _color);
	else Debug::DrawDebugPolygon(GetLevel(), _source.polygon, _color);
#endif
}

// ================= NavLinkComponent =================

NavLinkComponent::NavLinkComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(NavLinkComponent);
	UseDebug();
}

NavLinkSource NavLinkComponent::MakeSource() const
{
	NavLinkSource _link;
	_link.key = this;
	_link.start = GetOwner()->GetPosition() + startOffset;
	_link.end = GetOwner()->GetPosition() + endOffset;
	_link.bidirectional = bidirectional;
	_link.cost = std::max(0.05f, cost);
	return _link;
}

void NavLinkComponent::DrawDebug()
{
#ifdef DEBUG
	if (!NavigationSystem::showNavigation && !Editing(*this)) return;
	const NavLinkSource _link = MakeSource();
	const Color _color(200, 90, 255, 255);
	Debug::DrawDebugSegment(GetLevel(), _link.start, _link.end, _color);
	Debug::DrawDebugCircle(GetLevel(), _link.start, 8.f, 12, _color);
	Debug::DrawDebugCircle(GetLevel(), _link.end, _link.bidirectional ? 8.f : 4.f, 12, _color);
#endif
}

// ================= NavAgentComponent =================

NavAgentComponent::NavAgentComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(NavAgentComponent);
	UseDebug();
}

void NavAgentComponent::Construct()
{
	Component::Construct();
	system = &GetLevel()->GetNavigation();
	system->AddAgent(this);
}

void NavAgentComponent::Deconstruct()
{
	Component::Deconstruct();
	if (system) system->RemoveAgent(this);
	system = nullptr;
}

bool NavAgentComponent::MakePath()
{
	if (!system) return false;
	const FVector2 _position = GetOwner()->GetPosition();
	path = system->FindPath(_position, goal, radius);
	next = 1;
	sinceRepath = 0.f;
	onLink = false;
	const NavMesh* _mesh = system->MeshFor(_position, radius);
	meshVersion = _mesh ? _mesh->GetVersion() : -1;
	if (target) targetPathedAt = target->GetPosition();
	if (!path.found)
	{
		Fail();
		return false;
	}
	status = NavAgentStatus::Moving;
	return true;
}

bool NavAgentComponent::MoveTo(const FVector2& _goal)
{
	target = nullptr;
	goal = _goal;
	return MakePath();
}

bool NavAgentComponent::MoveToActor(Actor* _target)
{
	if (!_target) { Fail(); return false; }
	target = _target;
	goal = _target->GetPosition();
	return MakePath();
}

bool NavAgentComponent::Wander(const float& _radius)
{
	if (!system) return false;
	FVector2 _point;
	if (!system->RandomPoint(GetOwner()->GetPosition(), std::max(1.f, _radius), _point, radius)) { Fail(); return false; }
	return MoveTo(_point);
}

void NavAgentComponent::Stop()
{
	status = NavAgentStatus::Idle;
	target = nullptr;
	path = NavPath();
	velocity = FVector2(0.f, 0.f);
	if (PhysicsComponent* _body = GetOwner()->GetComponent<PhysicsComponent>(); _body && _body->GetBodyType() == 2) _body->SetLinearVelocity(FVector2(0.f, 0.f));
}

void NavAgentComponent::Arrive()
{
	const bool _partial = path.partial;
	target = nullptr;
	velocity = FVector2(0.f, 0.f);
	if (PhysicsComponent* _body = GetOwner()->GetComponent<PhysicsComponent>(); _body && _body->GetBodyType() == 2) _body->SetLinearVelocity(FVector2(0.f, 0.f));
	if (_partial) { Fail(); return; }	// as close as it could, but not there
	status = NavAgentStatus::Arrived;
	onArrived.Broadcast();
	onArrivedCalls.Invoke(this);
}

void NavAgentComponent::Fail()
{
	status = NavAgentStatus::Failed;
	target = nullptr;
	velocity = FVector2(0.f, 0.f);
	onFailed.Broadcast();
	onFailedCalls.Invoke(this);
}

float NavAgentComponent::GetRemainingDistance() const
{
	if (status != NavAgentStatus::Moving || next >= static_cast<int>(path.points.size())) return 0.f;
	float _distance = Length(path.points[next].position - GetOwner()->GetPosition());
	for (size_t _i = next + 1; _i < path.points.size(); ++_i) _distance += Length(path.points[_i].position - path.points[_i - 1].position);
	return _distance;
}

FVector2 NavAgentComponent::Avoid(const FVector2& _desired) const
{
	// Separation: each agent too close pushes away, more the closer it is (the others do the same: they part both ways).
	const FVector2 _position = GetOwner()->GetPosition();
	FVector2 _push(0.f, 0.f);
	for (const NavAgentComponent* _other : system->GetAgents())
	{
		if (_other == this || !_other->IsActive() || !_other->GetOwner()->IsActive()) continue;
		const FVector2 _offset = _position - _other->GetOwner()->GetPosition();
		const float _distance = Length(_offset);
		const float _room = radius + _other->radius + 8.f;
		if (_distance >= _room || _distance < 0.001f) continue;
		_push = _push + _offset * ((_room - _distance) / (_room * _distance));
	}
	FVector2 _result = _desired + _push * (speed * 1.5f);
	const float _length = Length(_result);
	if (_length > speed) _result = _result * (speed / _length);
	return _result;
}

void NavAgentComponent::Tick(const Float& _deltaTime)
{
	Component::Tick(_deltaTime);
	if (status != NavAgentStatus::Moving || !system) return;
	const float _dt = static_cast<float>(_deltaTime);
	Actor* _owner = GetOwner();
	FVector2 _position = _owner->GetPosition();

	// Following: a new path when the target moved enough (not every frame).
	sinceRepath += _dt;
	if (target)
	{
		if (!IsAlive(GetLevel(), target)) { Stop(); return; }
		goal = target->GetPosition();
		if (sinceRepath >= repathInterval && Length(goal - targetPathedAt) > std::max(stoppingDistance, 8.f) && !onLink) { if (!MakePath()) return; }
	}
	// The mesh changed under the path (an obstacle moved): a new one.
	const NavMesh* _mesh = system->MeshFor(_position, radius);
	if (_mesh && _mesh->GetVersion() != meshVersion && !onLink && sinceRepath > 0.1f) { if (!MakePath()) return; }

	if (next >= static_cast<int>(path.points.size())) { Arrive(); return; }
	const float _remaining = GetRemainingDistance();
	if (_remaining <= stoppingDistance) { Arrive(); return; }

	const NavPathPoint& _from = path.points[next - 1];
	const FVector2 _corner = path.points[next].position;
	onLink = _from.linkStart;
	if (onLink && linkMode == 1)	// teleport
	{
		_owner->SetPosition(_corner);
		++next;
		onLink = false;
		return;
	}

	// Speed toward the next corner, slowing down to stop at the end.
	const FVector2 _toCorner = _corner - _position;
	const float _cornerDistance = std::max(0.001f, Length(_toCorner));
	float _maxSpeed = speed;
	if (acceleration > 0.f) _maxSpeed = std::min(speed, std::sqrt(2.f * acceleration * std::max(0.f, _remaining - stoppingDistance * 0.5f)) + 1.f);
	FVector2 _desired = _toCorner * (_maxSpeed / _cornerDistance);
	if (avoidance && !onLink) _desired = Avoid(_desired);

	if (acceleration > 0.f)
	{
		const FVector2 _change = _desired - velocity;
		const float _step = acceleration * _dt, _changeLength = Length(_change);
		velocity = _changeLength <= _step ? _desired : velocity + _change * (_step / _changeLength);
	}
	else velocity = _desired;

	// Move: through the body when it is simulated, else the actor itself (kept on the mesh).
	PhysicsComponent* _body = _owner->GetComponent<PhysicsComponent>();
	if (_body && _body->IsActive() && _body->GetBodyType() == 2) _body->SetLinearVelocity(velocity);
	else
	{
		FVector2 _next = _position + velocity * _dt;
		if (!onLink && _mesh && !_mesh->IsWalkable(_next))
		{
			FVector2 _projected;
			if (_mesh->Project(_next, radius + _mesh->GetSettings().cellSize * 2.f, _projected)) _next = _projected;
		}
		_owner->SetPosition(_next);
		_position = _next;
	}

	if (Length(_corner - _position) <= std::max(4.f, Length(velocity) * _dt * 1.5f)) { ++next; onLink = false; }

	if (rotateToMovement && Length(velocity) > 1.f)
	{
		const float _wanted = std::atan2(velocity.y, velocity.x) * 180.f / 3.14159265f;
		const float _current = static_cast<float>(_owner->GetRotation().ToDegrees());
		const float _difference = std::remainder(_wanted - _current, 360.f);
		const float _turn = std::clamp(_difference, -rotationSpeed * _dt, rotationSpeed * _dt);
		_owner->SetRotation(Angle::FromDegrees(_current + _turn));
	}
}

void NavAgentComponent::DrawDebug()
{
#ifdef DEBUG
	if (!NavigationSystem::showNavigation) return;
	const FVector2 _position = GetOwner()->GetPosition();
	Debug::DrawDebugCircle(GetLevel(), _position, radius, 20, Color(80, 200, 255, 255));
	if (status != NavAgentStatus::Moving) return;
	FVector2 _last = _position;
	for (size_t _i = next; _i < path.points.size(); ++_i)
	{
		const bool _link = path.points[_i - 1].linkStart;
		Debug::DrawDebugSegment(GetLevel(), _last, path.points[_i].position, _link ? Color(200, 90, 255, 255) : Color(255, 220, 60, 255));
		_last = path.points[_i].position;
	}
	Debug::DrawDebugCircle(GetLevel(), _last, std::max(4.f, stoppingDistance), 12, path.partial ? Color(255, 120, 40, 255) : Color(255, 220, 60, 255));
#endif
}

// ================= NavTestComponent =================

NavTestComponent::NavTestComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(NavTestComponent);
	UseDebug();
}

NavPath NavTestComponent::Compute() const
{
	const FVector2 _start = GetOwner()->GetPosition();
	return GetLevel()->GetNavigation().FindPath(_start, _start + goalOffset, agentRadius);
}

void NavTestComponent::DrawDebug()
{
#ifdef DEBUG
	if (!Editing(*this) && !NavigationSystem::showNavigation) return;
	const FVector2 _start = GetOwner()->GetPosition(), _goal = _start + goalOffset;
	const NavPath _path = Compute();
	const Color _color = !_path.found ? Color(230, 60, 60, 255) : _path.partial ? Color(255, 140, 40, 255) : Color(90, 230, 120, 255);
	Debug::DrawDebugCircle(GetLevel(), _start, 10.f, 16, _color);
	Debug::DrawDebugCircle(GetLevel(), _goal, 10.f, 16, _color);
	if (!_path.found) { Debug::DrawDebugSegment(GetLevel(), _start, _goal, Color(230, 60, 60, 120)); return; }
	for (size_t _i = 1; _i < _path.points.size(); ++_i)
		Debug::DrawDebugSegment(GetLevel(), _path.points[_i - 1].position, _path.points[_i].position,
			_path.points[_i - 1].linkStart ? Color(200, 90, 255, 255) : _color);
#endif
}
