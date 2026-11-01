#include "NavigationSystem.h"
#include "NavigationComponents.h"
#include "Actors/Actor.h"
#include "Components/PhysicsComponent.h"
#include "GameFramework/Level.h"
#include "Utilities/Debug/Logger.h"
#include <algorithm>
#include <cmath>

using namespace Krampus;

void NavigationSystem::AddBounds(NavMeshBoundsComponent* _bounds)
{
	for (const std::unique_ptr<Entry>& _entry : meshes) if (_entry->bounds == _bounds) return;
	auto _entry = std::make_unique<Entry>();
	_entry->bounds = _bounds;
	meshes.push_back(std::move(_entry));
	everChecked = false;	// build it at the next Update
}

void NavigationSystem::RemoveBounds(NavMeshBoundsComponent* _bounds)
{
	std::erase_if(meshes, [&](const std::unique_ptr<Entry>& _entry) { return _entry->bounds == _bounds; });
}

void NavigationSystem::AddAgent(NavAgentComponent* _agent)
{
	if (std::find(agents.begin(), agents.end(), _agent) == agents.end()) agents.push_back(_agent);
}

void NavigationSystem::RemoveAgent(NavAgentComponent* _agent)
{
	std::erase(agents, _agent);
}

void NavigationSystem::RebuildAll()
{
	for (const std::unique_ptr<Entry>& _entry : meshes) _entry->dirty = true;
	everChecked = false;
}

void NavigationSystem::Gather(std::vector<NavSource>& _sources, std::vector<NavLinkSource>& _links) const
{
	for (const std::unique_ptr<Actor>& _actor : level->GetActorManager().GetActors())
	{
		if (!_actor->IsActive() || _actor->IsPendingDestroy()) continue;

		// A modifier says what the actor is for the navigation; else a static body is an obstacle (agents are never obstacles).
		const NavModifierComponent* _modifier = _actor->GetComponent<NavModifierComponent>();
		if (_modifier && _modifier->IsActive())
		{
			NavSource _source;
			if (_modifier->MakeSource(_source)) { _source.key = _modifier; _sources.push_back(std::move(_source)); }
		}
		else if (!_actor->GetComponent<NavAgentComponent>())
		{
			const PhysicsComponent* _body = _actor->GetComponent<PhysicsComponent>();
			if (_body && _body->IsActive() && _body->GetBodyType() == 0)
			{
				NavSource _source;
				if (NavShapeOf(*_actor, _source)) { _source.key = _body; _sources.push_back(std::move(_source)); }
			}
		}

		for (const std::unique_ptr<Component>& _component : _actor->GetComponents())
			if (const NavLinkComponent* _link = Cast<NavLinkComponent>(_component.get()); _link && _link->IsActive())
				_links.push_back(_link->MakeSource());
	}
}

void NavigationSystem::Update()
{
	if (meshes.empty()) return;
	const auto _now = std::chrono::steady_clock::now();
	const bool _anyDirty = std::any_of(meshes.begin(), meshes.end(), [](const std::unique_ptr<Entry>& _e) { return _e->dirty || !_e->mesh.IsBuilt(); });
	if (everChecked && !_anyDirty && std::chrono::duration<float>(_now - lastCheck).count() < checkInterval) return;
	lastCheck = _now;
	everChecked = true;

	std::vector<NavSource> _sources;
	std::vector<NavLinkSource> _links;
	Gather(_sources, _links);

	// What moved, appeared or went away since the last look: the region to rebuild is around their old and new places.
	float _minX = 1e30f, _minY = 1e30f, _maxX = -1e30f, _maxY = -1e30f;
	bool _changed = false;
	const auto _grow = [&](const float _x0, const float _y0, const float _x1, const float _y1)
	{
		_minX = std::min(_minX, _x0); _minY = std::min(_minY, _y0);
		_maxX = std::max(_maxX, _x1); _maxY = std::max(_maxY, _y1);
		_changed = true;
	};
	std::unordered_map<const void*, SourceState> _now_sources;
	for (const NavSource& _source : _sources)
	{
		SourceState _state{ _source.minX, _source.minY, _source.maxX, _source.maxY, _source.blocked, _source.cost, _source.polygon.size(),
			_source.polygon.empty() ? _source.center.x : _source.polygon.front().x, _source.polygon.empty() ? _source.center.y : _source.polygon.front().y };
		const auto _old = lastSources.find(_source.key);
		const bool _same = _old != lastSources.end() && _old->second.minX == _state.minX && _old->second.minY == _state.minY && _old->second.maxX == _state.maxX
			&& _old->second.maxY == _state.maxY && _old->second.blocked == _state.blocked && _old->second.cost == _state.cost && _old->second.points == _state.points
			&& _old->second.firstX == _state.firstX && _old->second.firstY == _state.firstY;
		if (!_same)
		{
			_grow(_state.minX, _state.minY, _state.maxX, _state.maxY);
			if (_old != lastSources.end()) _grow(_old->second.minX, _old->second.minY, _old->second.maxX, _old->second.maxY);
		}
		_now_sources[_source.key] = _state;
	}
	for (const auto& [_key, _state] : lastSources)
		if (!_now_sources.contains(_key)) _grow(_state.minX, _state.minY, _state.maxX, _state.maxY);
	lastSources = std::move(_now_sources);

	bool _linksChanged = _links.size() != lastLinks.size();
	for (size_t _i = 0; !_linksChanged && _i < _links.size(); ++_i)
		_linksChanged = _links[_i].start.x != lastLinks[_i].start.x || _links[_i].start.y != lastLinks[_i].start.y || _links[_i].end.x != lastLinks[_i].end.x
			|| _links[_i].end.y != lastLinks[_i].end.y || _links[_i].bidirectional != lastLinks[_i].bidirectional || _links[_i].cost != lastLinks[_i].cost;
	lastLinks = _links;

	for (const std::unique_ptr<Entry>& _entry : meshes)
	{
		const NavMeshSettings _settings = _entry->bounds->GetSettings();
		if (_entry->dirty || !_entry->mesh.IsBuilt() || !_settings.SameAs(_entry->mesh.GetSettings()))
		{
			_entry->mesh.Build(_settings, _sources, _links);
			_entry->dirty = false;
			LOG(VerbosityType::Log, "Navigation: built " + std::to_string(_entry->mesh.GetPolys().size()) + " polygons (" + std::to_string(_entry->mesh.GetWidth())
				+ " x " + std::to_string(_entry->mesh.GetHeight()) + " cells, agent radius " + std::to_string(static_cast<int>(_settings.agentRadius)) + ") in "
				+ std::to_string(_entry->mesh.GetLastBuildMs()) + " ms");
		}
		else if (_changed) _entry->mesh.Update(_minX, _minY, _maxX, _maxY, _sources, _links);
		else if (_linksChanged) _entry->mesh.Update(1.f, 1.f, 0.f, 0.f, _sources, _links);	// an empty region: only the links are made again
	}
}

const NavMesh* NavigationSystem::GetMesh(const NavMeshBoundsComponent* _bounds) const
{
	for (const std::unique_ptr<Entry>& _entry : meshes)
		if (_entry->bounds == _bounds) return _entry->mesh.IsBuilt() ? &_entry->mesh : nullptr;
	return nullptr;
}

const NavMesh* NavigationSystem::MeshFor(const FVector2& _point, const float& _agentRadius) const
{
	const NavMesh* _best = nullptr;
	float _bestRadius = 0.f;
	bool _bestFits = false;
	for (const std::unique_ptr<Entry>& _entry : meshes)
	{
		const NavMesh& _mesh = _entry->mesh;
		if (!_mesh.IsBuilt()) continue;
		const NavMeshSettings& _s = _mesh.GetSettings();
		const float _margin = _s.cellSize * 4.f + _s.agentRadius * 2.f;	// a point just outside still projects onto it
		if (_point.x < _s.min.x - _margin || _point.y < _s.min.y - _margin || _point.x > _s.min.x + _s.size.x + _margin || _point.y > _s.min.y + _s.size.y + _margin) continue;
		const bool _fits = _s.agentRadius + 0.01f >= _agentRadius;
		// The smallest radius that fits the agent; when none does, the biggest one.
		const bool _better = !_best || (_fits && !_bestFits) || (_fits && _bestFits && _s.agentRadius < _bestRadius) || (!_fits && !_bestFits && _s.agentRadius > _bestRadius);
		if (_better) { _best = &_mesh; _bestRadius = _s.agentRadius; _bestFits = _fits; }
	}
	return _best;
}

NavPath NavigationSystem::FindPath(const FVector2& _start, const FVector2& _goal, const float& _agentRadius) const
{
	const NavMesh* _mesh = MeshFor(_start, _agentRadius);
	return _mesh ? _mesh->FindPath(_start, _goal) : NavPath();
}

bool NavigationSystem::Project(const FVector2& _point, const float& _extent, FVector2& _out, const float& _agentRadius) const
{
	const NavMesh* _mesh = MeshFor(_point, _agentRadius);
	return _mesh && _mesh->Project(_point, _extent, _out);
}

bool NavigationSystem::Raycast(const FVector2& _from, const FVector2& _to, FVector2* _hit, const float& _agentRadius) const
{
	const NavMesh* _mesh = MeshFor(_from, _agentRadius);
	if (!_mesh) { if (_hit) *_hit = _from; return false; }
	return _mesh->Raycast(_from, _to, _hit);
}

bool NavigationSystem::IsReachable(const FVector2& _from, const FVector2& _to, const float& _agentRadius) const
{
	const NavMesh* _mesh = MeshFor(_from, _agentRadius);
	return _mesh && _mesh->IsReachable(_from, _to);
}

bool NavigationSystem::RandomPoint(const FVector2& _center, const float& _radius, FVector2& _out, const float& _agentRadius)
{
	const NavMesh* _mesh = MeshFor(_center, _agentRadius);
	return _mesh && _mesh->RandomPoint(_center, _radius, _out, random);
}
