#pragma once
#include "NavMesh.h"
#include <chrono>
#include <memory>
#include <random>
#include <unordered_map>

namespace Krampus
{
	class Level;
	class NavMeshBoundsComponent;
	class NavAgentComponent;

	/// <summary>
	/// The navigation of a level (Level::GetNavigation): one navigation mesh per NavMesh Bounds Volume, kept up to date by itself.
	///   - obstacles: the static physics bodies (PhysicsComponent with bodyType Static) and the NavModifierComponents set to Blocked;
	///   - areas with a cost: NavModifierComponents set to Cost (mud, water: avoided; roads: preferred);
	///   - links: NavLinkComponents (jumps, ladders, teleporters);
	///   - each volume has its agent radius: an agent uses the volume made for its size (several sizes = several volumes).
	/// Every few frames it looks at what changed and rebuilds only the tiles that it touches. Nothing is built when the level has no volume.
	/// </summary>
	class NavigationSystem
	{
	public:
		/// <summary>Navigation meshes, modifiers and links are drawn (editor: key P or Tools > Show Navigation).</summary>
		inline static bool	showNavigation	= false;

		/// <summary>Seconds between two looks at what changed (moved obstacles).</summary>
		float				checkInterval	= 0.2f;

		explicit NavigationSystem(Level* _level) : level(_level), random(std::random_device{}()) { }

		void AddBounds(NavMeshBoundsComponent* _bounds);
		void RemoveBounds(NavMeshBoundsComponent* _bounds);
		void AddAgent(NavAgentComponent* _agent);
		void RemoveAgent(NavAgentComponent* _agent);
		const std::vector<NavAgentComponent*>& GetAgents() const { return agents; }

		/// <summary>Rebuilds what changed (called by the volumes every frame; does the work every checkInterval seconds).</summary>
		void Update();

		/// <summary>Everything is built again at the next Update.</summary>
		void RebuildAll();

		// ---- Queries (the mesh of the volume that has the point and suits the agent radius)

		/// <returns>The mesh for an agent of this radius at this place: the smallest radius that is at least as big, else the biggest.</returns>
		const NavMesh* MeshFor(const FVector2& _point, const float& _agentRadius) const;

		NavPath FindPath(const FVector2& _start, const FVector2& _goal, const float& _agentRadius = 0.f) const;
		bool Project(const FVector2& _point, const float& _extent, FVector2& _out, const float& _agentRadius = 0.f) const;
		bool Raycast(const FVector2& _from, const FVector2& _to, FVector2* _hit = nullptr, const float& _agentRadius = 0.f) const;
		bool IsReachable(const FVector2& _from, const FVector2& _to, const float& _agentRadius = 0.f) const;
		bool RandomPoint(const FVector2& _center, const float& _radius, FVector2& _out, const float& _agentRadius = 0.f);

		/// <returns>The mesh of a volume (null until it is built).</returns>
		const NavMesh* GetMesh(const NavMeshBoundsComponent* _bounds) const;

	private:
		struct SourceState
		{
			float		minX, minY, maxX, maxY;
			bool		blocked;
			float		cost;
			size_t		points;
			float		firstX, firstY;
		};

		struct Entry
		{
			NavMeshBoundsComponent*								bounds	= nullptr;
			NavMesh												mesh;
			bool												dirty	= true;		// build it all
		};

		Level*												level;
		std::vector<std::unique_ptr<Entry>>					meshes;
		std::vector<NavAgentComponent*>						agents;
		std::unordered_map<const void*, SourceState>		lastSources;
		std::vector<NavLinkSource>							lastLinks;
		std::chrono::steady_clock::time_point				lastCheck;
		bool												everChecked	= false;
		std::mt19937										random;

		void Gather(std::vector<NavSource>& _sources, std::vector<NavLinkSource>& _links) const;
	};
}
