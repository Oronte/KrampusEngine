#pragma once
#include "Utilities/Math/Vector2D.h"
#include <cstdint>
#include <random>
#include <vector>

// The navigation mesh of a 2D level, built like Recast (the one of Unreal and Unity) but flat:
//   1. the area of a NavMesh Bounds Volume is a grid of cells;
//   2. obstacles (static physics bodies, blocking modifiers) and areas with a cost (mud, water...) are drawn into it;
//   3. the walkable cells are eroded by the radius of the agent (exact distance transform): a path never brushes a wall;
//   4. each tile of the grid is cut into rectangles of the same area (convex polygons), linked by portals where they touch;
//   5. off-mesh links (jumps, ladders, teleporters) join polygons that do not touch.
// Paths are found with A* over the polygons, then pulled tight with the funnel algorithm (straight lines, corners only where needed).
// A change (an obstacle that moved) rebuilds only the tiles it touches.

namespace Krampus
{
	constexpr std::uint8_t NAV_BLOCKED = 255;

	/// <summary>Something that blocks the navigation or changes its cost, in world pixels.</summary>
	struct NavSource
	{
		const void*				key			= nullptr;	// where it comes from (to see when it moved)
		bool					blocked		= true;		// false: an area with a cost
		float					cost		= 1.f;		// cost of a pixel walked in it (1 = normal, 5 = avoided, 0.5 = preferred)
		bool					circle		= false;
		FVector2				center;
		float					radius		= 0.f;
		std::vector<FVector2>	polygon;				// world points, any polygon (concave included)
		float					minX = 0.f, minY = 0.f, maxX = 0.f, maxY = 0.f;

		/// <summary>The rectangle around it (call it after setting the shape).</summary>
		void ComputeBounds();
	};

	/// <summary>A connection between two places that do not touch (a jump, a ladder, a teleporter).</summary>
	struct NavLinkSource
	{
		const void*		key				= nullptr;
		FVector2		start;
		FVector2		end;
		bool			bidirectional	= true;
		float			cost			= 1.f;		// times its length
	};

	struct NavMeshSettings
	{
		FVector2	min;					// top left corner of the area, world pixels
		FVector2	size;					// pixels
		float		cellSize		= 8.f;	// pixels: smaller = more precise, slower to build
		float		agentRadius		= 16.f;	// the agents stay this far from obstacles
		int			tileSize		= 32;	// cells: what is rebuilt when something moves
		bool		erodeEdges		= false;	// the border of the area counts as a wall

		bool SameAs(const NavMeshSettings& _other) const;
	};

	/// <summary>A convex polygon of the mesh: a rectangle of cells with the same area.</summary>
	struct NavPoly
	{
		int				x = 0, y = 0, w = 0, h = 0;	// cells
		std::uint8_t	area		= 0;
		int				firstEdge	= 0;
		int				edgeCount	= 0;
		int				component	= 0;			// polygons with the same number can reach each other
	};

	/// <summary>A way from a polygon to another: the segment they share (a portal) or an off-mesh link.</summary>
	struct NavEdge
	{
		int			to		= -1;
		FVector2	a, b;					// the portal; for a link: a = where it starts, b = where it lands
		int			link	= -1;			// index of the link, -1 for a portal
	};

	struct NavPathPoint
	{
		FVector2	position;
		bool		linkStart	= false;	// the move from this point to the next one goes through an off-mesh link
	};

	struct NavPath
	{
		std::vector<NavPathPoint>	points;		// from the start to the end
		bool						found		= false;
		bool						partial		= false;	// the goal cannot be reached: the path goes as close as it can
		float						length		= 0.f;
		float						cost		= 0.f;
	};

	class NavMesh
	{
	public:
		/// <summary>Builds everything from nothing.</summary>
		void Build(const NavMeshSettings& _settings, const std::vector<NavSource>& _sources, const std::vector<NavLinkSource>& _links);

		/// <summary>Rebuilds only what a world rectangle touches (an obstacle that moved: its old and new place). Links are redone.</summary>
		void Update(float _minX, float _minY, float _maxX, float _maxY, const std::vector<NavSource>& _sources, const std::vector<NavLinkSource>& _links);

		bool IsBuilt() const { return width > 0; }
		const NavMeshSettings& GetSettings() const { return settings; }
		int GetVersion() const { return version; }		// changes at each build: paths made before may be wrong

		// ---- Queries

		/// <returns>The path from _start to _goal (both are moved onto the mesh first). Partial when the goal cannot be reached.</returns>
		NavPath FindPath(const FVector2& _start, const FVector2& _goal) const;

		/// <returns>True with the closest point of the mesh within _extent pixels.</returns>
		bool Project(const FVector2& _point, const float& _extent, FVector2& _out) const;

		/// <returns>True when nothing blocks the straight line; else false with where it stops in _hit.</returns>
		bool Raycast(const FVector2& _from, const FVector2& _to, FVector2* _hit = nullptr) const;

		/// <returns>Whether a path exists between the two points (fast: same connected part of the mesh).</returns>
		bool IsReachable(const FVector2& _from, const FVector2& _to) const;

		/// <returns>True with a random walkable point within _radius of _center that can be reached from it.</returns>
		bool RandomPoint(const FVector2& _center, const float& _radius, FVector2& _out, std::mt19937& _random) const;

		/// <returns>The polygon under a point, -1 if it is not on the mesh.</returns>
		int PolyAt(const FVector2& _point) const;
		bool IsWalkable(const FVector2& _point) const { return PolyAt(_point) >= 0; }

		/// <returns>Cost of a pixel walked in an area (NavSource::cost).</returns>
		float AreaCost(const std::uint8_t& _area) const { return areaCosts[_area]; }

		// ---- What it is made of (debug drawing, statistics)

		const std::vector<NavPoly>& GetPolys() const { return polys; }
		const std::vector<NavEdge>& GetEdges() const { return edges; }
		FVector2 PolyMin(const NavPoly& _poly) const;
		FVector2 PolyMax(const NavPoly& _poly) const;
		int GetWidth() const { return width; }
		int GetHeight() const { return height; }
		float GetLastBuildMs() const { return lastBuildMs; }
		int GetWalkableCells() const;

	private:
		NavMeshSettings					settings;
		int								width = 0, height = 0;		// cells
		int								tilesX = 0, tilesY = 0;
		std::vector<std::uint8_t>		raw;		// area of each cell from the sources (NAV_BLOCKED: an obstacle)
		std::vector<std::uint8_t>		walk;		// the same, eroded by the agent radius
		std::vector<int>				cellPoly;	// polygon of each cell, -1: not walkable
		std::vector<std::vector<NavPoly>>	tilePolys;	// rectangles of each tile (only dirty tiles are made again)
		std::vector<NavPoly>			polys;
		std::vector<NavEdge>			edges;
		std::vector<NavLinkSource>		links;
		std::vector<float>				areaCosts	= std::vector<float>(256, 1.f);
		std::vector<float>				areaIdCost;	// cost of each area id in use (id 0: 1)
		float							minCost		= 1.f;
		int								version		= 0;
		float							lastBuildMs	= 0.f;

		// A* scratch, kept between queries (no memory taken per path)
		mutable std::vector<float>		cost;
		mutable std::vector<int>		parent, parentEdge;
		mutable std::vector<FVector2>	entry;
		mutable std::vector<std::uint32_t>	visited;
		mutable std::uint32_t			stamp		= 0;

		int Index(const int& _x, const int& _y) const { return _y * width + _x; }
		std::uint8_t AreaFor(const float& _cost);
		void Rasterize(int _x0, int _y0, int _x1, int _y1, const std::vector<NavSource>& _sources);
		void Erode(int _x0, int _y0, int _x1, int _y1);
		void MeshTile(const int& _tx, const int& _ty);
		void Finish(const std::vector<NavLinkSource>& _links);
		FVector2 CellCenter(const int& _x, const int& _y) const;
		FVector2 ClampToPoly(const FVector2& _point, const NavPoly& _poly) const;
		void StringPull(const std::vector<std::pair<FVector2, FVector2>>& _portals, std::vector<NavPathPoint>& _out) const;
	};
}
