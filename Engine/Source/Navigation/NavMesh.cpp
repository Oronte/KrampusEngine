#include "NavMesh.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <numeric>
#include <queue>

using namespace Krampus;

namespace
{
	constexpr float INF = 1e20f;

	bool InsidePolygon(const std::vector<FVector2>& _polygon, const float _x, const float _y)
	{
		bool _inside = false;
		const size_t _count = _polygon.size();
		for (size_t _i = 0, _j = _count - 1; _i < _count; _j = _i++)
		{
			const FVector2& _a = _polygon[_i];
			const FVector2& _b = _polygon[_j];
			if ((_a.y > _y) != (_b.y > _y) && _x < (_b.x - _a.x) * (_y - _a.y) / (_b.y - _a.y) + _a.x) _inside = !_inside;
		}
		return _inside;
	}

	// Felzenszwalb & Huttenlocher: exact squared distance to the nearest zero of f, along one line, in linear time.
	void DistanceTransform1D(const float* _f, float* _d, const int _n, int* _v, float* _z)
	{
		int _k = 0;
		_v[0] = 0;
		_z[0] = -INF;
		_z[1] = INF;
		for (int _q = 1; _q < _n; ++_q)
		{
			float _s = ((_f[_q] + _q * _q) - (_f[_v[_k]] + _v[_k] * _v[_k])) / (2.f * _q - 2.f * _v[_k]);
			while (_s <= _z[_k])
			{
				--_k;
				_s = ((_f[_q] + _q * _q) - (_f[_v[_k]] + _v[_k] * _v[_k])) / (2.f * _q - 2.f * _v[_k]);
			}
			++_k;
			_v[_k] = _q;
			_z[_k] = _s;
			_z[_k + 1] = INF;
		}
		_k = 0;
		for (int _q = 0; _q < _n; ++_q)
		{
			while (_z[_k + 1] < _q) ++_k;
			const float _dq = static_cast<float>(_q - _v[_k]);
			_d[_q] = _dq * _dq + _f[_v[_k]];
		}
	}

	float TriArea2(const FVector2& _a, const FVector2& _b, const FVector2& _c)
	{
		const float _ax = _b.x - _a.x, _ay = _b.y - _a.y, _bx = _c.x - _a.x, _by = _c.y - _a.y;
		return _bx * _ay - _ax * _by;
	}

	bool Same(const FVector2& _a, const FVector2& _b)
	{
		return std::abs(_a.x - _b.x) < 0.001f && std::abs(_a.y - _b.y) < 0.001f;
	}

	float Distance(const FVector2& _a, const FVector2& _b)
	{
		return std::sqrt((_b.x - _a.x) * (_b.x - _a.x) + (_b.y - _a.y) * (_b.y - _a.y));
	}

	int Find(std::vector<int>& _parent, int _i)
	{
		while (_parent[_i] != _i) { _parent[_i] = _parent[_parent[_i]]; _i = _parent[_i]; }
		return _i;
	}
}

void NavSource::ComputeBounds()
{
	if (circle)
	{
		minX = center.x - radius; maxX = center.x + radius;
		minY = center.y - radius; maxY = center.y + radius;
		return;
	}
	minX = minY = INF;
	maxX = maxY = -INF;
	for (const FVector2& _point : polygon)
	{
		minX = std::min(minX, _point.x); maxX = std::max(maxX, _point.x);
		minY = std::min(minY, _point.y); maxY = std::max(maxY, _point.y);
	}
}

bool NavMeshSettings::SameAs(const NavMeshSettings& _other) const
{
	return min.x == _other.min.x && min.y == _other.min.y && size.x == _other.size.x && size.y == _other.size.y
		&& cellSize == _other.cellSize && agentRadius == _other.agentRadius && tileSize == _other.tileSize && erodeEdges == _other.erodeEdges;
}

// ================= Building =================

void NavMesh::Build(const NavMeshSettings& _settings, const std::vector<NavSource>& _sources, const std::vector<NavLinkSource>& _links)
{
	const auto _begin = std::chrono::steady_clock::now();
	settings = _settings;
	settings.cellSize = std::max(1.f, settings.cellSize);
	settings.tileSize = std::clamp(settings.tileSize, 4, 256);
	settings.agentRadius = std::max(0.f, settings.agentRadius);
	width = std::max(1, static_cast<int>(std::ceil(std::max(1.f, static_cast<float>(settings.size.x)) / settings.cellSize)));
	height = std::max(1, static_cast<int>(std::ceil(std::max(1.f, static_cast<float>(settings.size.y)) / settings.cellSize)));
	tilesX = (width + settings.tileSize - 1) / settings.tileSize;
	tilesY = (height + settings.tileSize - 1) / settings.tileSize;

	raw.assign(static_cast<size_t>(width) * height, 0);
	walk.assign(raw.size(), NAV_BLOCKED);
	cellPoly.assign(raw.size(), -1);
	tilePolys.assign(static_cast<size_t>(tilesX) * tilesY, {});
	std::fill(areaCosts.begin(), areaCosts.end(), 1.f);
	areaIdCost = { 1.f };
	minCost = 1.f;

	Rasterize(0, 0, width, height, _sources);
	Erode(0, 0, width, height);
	for (int _ty = 0; _ty < tilesY; ++_ty)
		for (int _tx = 0; _tx < tilesX; ++_tx) MeshTile(_tx, _ty);
	Finish(_links);
	lastBuildMs = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - _begin).count();
}

void NavMesh::Update(float _minX, float _minY, float _maxX, float _maxY, const std::vector<NavSource>& _sources, const std::vector<NavLinkSource>& _links)
{
	if (!IsBuilt()) return;
	const auto _begin = std::chrono::steady_clock::now();
	const float _cs = settings.cellSize;
	const int _x0 = std::clamp(static_cast<int>(std::floor((_minX - settings.min.x) / _cs)) - 1, 0, width);
	const int _y0 = std::clamp(static_cast<int>(std::floor((_minY - settings.min.y) / _cs)) - 1, 0, height);
	const int _x1 = std::clamp(static_cast<int>(std::ceil((_maxX - settings.min.x) / _cs)) + 1, 0, width);
	const int _y1 = std::clamp(static_cast<int>(std::ceil((_maxY - settings.min.y) / _cs)) + 1, 0, height);
	if (_x0 >= _x1 || _y0 >= _y1) { Finish(_links); return; }	// only the links may have changed

	// A change of the cells moves the eroded border up to the agent radius away.
	const int _reach = static_cast<int>(std::ceil(settings.agentRadius / _cs + 0.5f)) + 1;
	const int _ex0 = std::max(0, _x0 - _reach), _ey0 = std::max(0, _y0 - _reach), _ex1 = std::min(width, _x1 + _reach), _ey1 = std::min(height, _y1 + _reach);

	Rasterize(_x0, _y0, _x1, _y1, _sources);
	Erode(_ex0, _ey0, _ex1, _ey1);
	const int _t = settings.tileSize;
	for (int _ty = _ey0 / _t; _ty <= (_ey1 - 1) / _t; ++_ty)
		for (int _tx = _ex0 / _t; _tx <= (_ex1 - 1) / _t; ++_tx) MeshTile(_tx, _ty);
	Finish(_links);
	lastBuildMs = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - _begin).count();
}

std::uint8_t NavMesh::AreaFor(const float& _cost)
{
	const float _clamped = std::clamp(_cost, 0.05f, 1000.f);
	if (std::abs(_clamped - 1.f) < 0.0001f) return 0;
	for (size_t _id = 1; _id < areaIdCost.size(); ++_id)
		if (std::abs(areaIdCost[_id] - _clamped) < 0.0001f) return static_cast<std::uint8_t>(_id);
	if (areaIdCost.size() >= NAV_BLOCKED) return static_cast<std::uint8_t>(areaIdCost.size() - 1);	// 254 kinds of areas at most
	areaIdCost.push_back(_clamped);
	const std::uint8_t _id = static_cast<std::uint8_t>(areaIdCost.size() - 1);
	areaCosts[_id] = _clamped;
	minCost = std::min(minCost, _clamped);
	return _id;
}

FVector2 NavMesh::CellCenter(const int& _x, const int& _y) const
{
	return FVector2(settings.min.x + (_x + 0.5f) * settings.cellSize, settings.min.y + (_y + 0.5f) * settings.cellSize);
}

void NavMesh::Rasterize(int _x0, int _y0, int _x1, int _y1, const std::vector<NavSource>& _sources)
{
	for (int _y = _y0; _y < _y1; ++_y)
		std::fill(raw.begin() + Index(_x0, _y), raw.begin() + Index(_x0, _y) + (_x1 - _x0), static_cast<std::uint8_t>(0));

	const float _cs = settings.cellSize;
	const auto _draw = [&](const NavSource& _source)
	{
		// The cells of the region whose center is in the source.
		const int _sx0 = std::max(_x0, static_cast<int>(std::floor((_source.minX - settings.min.x) / _cs)));
		const int _sy0 = std::max(_y0, static_cast<int>(std::floor((_source.minY - settings.min.y) / _cs)));
		const int _sx1 = std::min(_x1, static_cast<int>(std::ceil((_source.maxX - settings.min.x) / _cs)) + 1);
		const int _sy1 = std::min(_y1, static_cast<int>(std::ceil((_source.maxY - settings.min.y) / _cs)) + 1);
		const std::uint8_t _area = _source.blocked ? NAV_BLOCKED : AreaFor(_source.cost);
		for (int _y = _sy0; _y < _sy1; ++_y)
			for (int _x = _sx0; _x < _sx1; ++_x)
			{
				const FVector2 _c = CellCenter(_x, _y);
				const bool _in = _source.circle
					? (_c.x - _source.center.x) * (_c.x - _source.center.x) + (_c.y - _source.center.y) * (_c.y - _source.center.y) <= _source.radius * _source.radius
					: _source.polygon.size() >= 3 && InsidePolygon(_source.polygon, _c.x, _c.y);
				if (!_in) continue;
				std::uint8_t& _cell = raw[Index(_x, _y)];
				if (_area == NAV_BLOCKED || _cell == 0 || (_cell != NAV_BLOCKED && areaIdCost[_area] > areaIdCost[_cell])) _cell = _area;	// the dearest area wins
			}
	};
	for (const NavSource& _source : _sources)
		if (_source.maxX >= settings.min.x + _x0 * _cs && _source.minX <= settings.min.x + _x1 * _cs
			&& _source.maxY >= settings.min.y + _y0 * _cs && _source.minY <= settings.min.y + _y1 * _cs) _draw(_source);
}

void NavMesh::Erode(int _x0, int _y0, int _x1, int _y1)
{
	const float _threshold = settings.agentRadius > 0.f ? settings.agentRadius / settings.cellSize + 0.5f : 0.f;
	if (_threshold <= 0.f && !settings.erodeEdges)
	{
		for (int _y = _y0; _y < _y1; ++_y)
			for (int _x = _x0; _x < _x1; ++_x) walk[Index(_x, _y)] = raw[Index(_x, _y)];
		return;
	}

	// The exact distance to the nearest obstacle, computed on the region plus a margin (obstacles a radius away count).
	const int _margin = static_cast<int>(std::ceil(_threshold)) + 1;
	const int _px0 = std::max(0, _x0 - _margin), _py0 = std::max(0, _y0 - _margin);
	const int _px1 = std::min(width, _x1 + _margin), _py1 = std::min(height, _y1 + _margin);
	const int _pw = _px1 - _px0, _ph = _py1 - _py0;
	const int _longest = std::max(_pw, _ph);

	std::vector<float> _grid(static_cast<size_t>(_pw) * _ph);
	for (int _y = 0; _y < _ph; ++_y)
		for (int _x = 0; _x < _pw; ++_x) _grid[_y * _pw + _x] = raw[Index(_px0 + _x, _py0 + _y)] == NAV_BLOCKED ? 0.f : INF;

	std::vector<float> _f(_longest), _d(_longest), _z(_longest + 1);
	std::vector<int> _v(_longest);
	for (int _x = 0; _x < _pw; ++_x)	// columns
	{
		for (int _y = 0; _y < _ph; ++_y) _f[_y] = _grid[_y * _pw + _x];
		DistanceTransform1D(_f.data(), _d.data(), _ph, _v.data(), _z.data());
		for (int _y = 0; _y < _ph; ++_y) _grid[_y * _pw + _x] = _d[_y];
	}
	for (int _y = 0; _y < _ph; ++_y)	// rows
	{
		DistanceTransform1D(&_grid[_y * _pw], _d.data(), _pw, _v.data(), _z.data());
		std::copy(_d.begin(), _d.begin() + _pw, _grid.begin() + _y * _pw);
	}

	const float _limit = _threshold * _threshold;
	for (int _y = _y0; _y < _y1; ++_y)
		for (int _x = _x0; _x < _x1; ++_x)
		{
			float _squared = _grid[(_y - _py0) * _pw + (_x - _px0)];
			if (settings.erodeEdges)
			{
				const float _edge = std::min({ _x + 0.5f, width - _x - 0.5f, _y + 0.5f, height - _y - 0.5f });
				_squared = std::min(_squared, _edge * _edge);
			}
			const std::uint8_t _cell = raw[Index(_x, _y)];
			walk[Index(_x, _y)] = _cell == NAV_BLOCKED || _squared < _limit ? NAV_BLOCKED : _cell;
		}
}

void NavMesh::MeshTile(const int& _tx, const int& _ty)
{
	// Greedy meshing: the largest rectangles of cells with the same area, row by row.
	const int _t = settings.tileSize;
	const int _cx0 = _tx * _t, _cy0 = _ty * _t;
	const int _cx1 = std::min(width, _cx0 + _t), _cy1 = std::min(height, _cy0 + _t);
	const int _tw = _cx1 - _cx0;
	std::vector<NavPoly>& _out = tilePolys[_ty * tilesX + _tx];
	_out.clear();
	std::vector<char> _used(static_cast<size_t>(_tw) * (_cy1 - _cy0), 0);
	const auto _free = [&](const int _x, const int _y, const std::uint8_t _area)
	{
		return !_used[(_y - _cy0) * _tw + (_x - _cx0)] && walk[Index(_x, _y)] == _area;
	};

	for (int _y = _cy0; _y < _cy1; ++_y)
		for (int _x = _cx0; _x < _cx1; ++_x)
		{
			const std::uint8_t _area = walk[Index(_x, _y)];
			if (_area == NAV_BLOCKED || _used[(_y - _cy0) * _tw + (_x - _cx0)]) continue;
			int _w = 1;
			while (_x + _w < _cx1 && _free(_x + _w, _y, _area)) ++_w;
			int _h = 1;
			for (bool _grow = true; _grow && _y + _h < _cy1; )
			{
				for (int _i = 0; _i < _w && _grow; ++_i) _grow = _free(_x + _i, _y + _h, _area);
				if (_grow) ++_h;
			}
			for (int _j = 0; _j < _h; ++_j)
				for (int _i = 0; _i < _w; ++_i) _used[(_y + _j - _cy0) * _tw + (_x + _i - _cx0)] = 1;
			NavPoly _poly;
			_poly.x = _x; _poly.y = _y; _poly.w = _w; _poly.h = _h; _poly.area = _area;
			_out.push_back(_poly);
		}
}

FVector2 NavMesh::PolyMin(const NavPoly& _poly) const
{
	return FVector2(settings.min.x + _poly.x * settings.cellSize, settings.min.y + _poly.y * settings.cellSize);
}

FVector2 NavMesh::PolyMax(const NavPoly& _poly) const
{
	return FVector2(settings.min.x + (_poly.x + _poly.w) * settings.cellSize, settings.min.y + (_poly.y + _poly.h) * settings.cellSize);
}

void NavMesh::Finish(const std::vector<NavLinkSource>& _links)
{
	// All the polygons in one list, and the cell -> polygon map.
	polys.clear();
	for (const std::vector<NavPoly>& _tile : tilePolys) polys.insert(polys.end(), _tile.begin(), _tile.end());
	std::fill(cellPoly.begin(), cellPoly.end(), -1);
	for (int _p = 0; _p < static_cast<int>(polys.size()); ++_p)
	{
		const NavPoly& _poly = polys[_p];
		for (int _y = _poly.y; _y < _poly.y + _poly.h; ++_y)
			std::fill(cellPoly.begin() + Index(_poly.x, _y), cellPoly.begin() + Index(_poly.x, _y) + _poly.w, _p);
	}

	// Portals: along each side, the runs of cells that belong to the same neighbor.
	const float _cs = settings.cellSize;
	const FVector2 _o = settings.min;
	std::vector<std::vector<NavEdge>> _byPoly(polys.size());
	for (int _p = 0; _p < static_cast<int>(polys.size()); ++_p)
	{
		const NavPoly& _poly = polys[_p];
		const auto _side = [&](const int _fixed, const bool _vertical, const int _from, const int _to, const float _line)
		{
			int _runStart = _from, _current = -2;
			for (int _i = _from; _i <= _to; ++_i)
			{
				int _q = -1;
				if (_i < _to)
				{
					const int _cx = _vertical ? _fixed : _i, _cy = _vertical ? _i : _fixed;
					if (_cx >= 0 && _cy >= 0 && _cx < width && _cy < height) _q = cellPoly[Index(_cx, _cy)];
				}
				if (_q == _current) continue;
				if (_current >= 0 && _current != _p)
				{
					NavEdge _edge;
					_edge.to = _current;
					_edge.a = _vertical ? FVector2(_line, _o.y + _runStart * _cs) : FVector2(_o.x + _runStart * _cs, _line);
					_edge.b = _vertical ? FVector2(_line, _o.y + _i * _cs) : FVector2(_o.x + _i * _cs, _line);
					_byPoly[_p].push_back(_edge);
				}
				_current = _q;
				_runStart = _i;
			}
		};
		_side(_poly.x + _poly.w, true, _poly.y, _poly.y + _poly.h, _o.x + (_poly.x + _poly.w) * _cs);	// right
		_side(_poly.x - 1, true, _poly.y, _poly.y + _poly.h, _o.x + _poly.x * _cs);						// left
		_side(_poly.y + _poly.h, false, _poly.x, _poly.x + _poly.w, _o.y + (_poly.y + _poly.h) * _cs);	// bottom
		_side(_poly.y - 1, false, _poly.x, _poly.x + _poly.w, _o.y + _poly.y * _cs);						// top
	}

	// Off-mesh links, from where their ends land on the mesh.
	links = _links;
	const float _extent = _cs * 3.f + settings.agentRadius;
	for (int _l = 0; _l < static_cast<int>(links.size()); ++_l)
	{
		FVector2 _start, _end;
		if (!Project(links[_l].start, _extent, _start) || !Project(links[_l].end, _extent, _end)) continue;
		const int _from = PolyAt(_start), _to = PolyAt(_end);
		if (_from < 0 || _to < 0) continue;
		links[_l].start = _start;
		links[_l].end = _end;
		_byPoly[_from].push_back({ _to, _start, _end, _l });
		if (links[_l].bidirectional) _byPoly[_to].push_back({ _from, _end, _start, _l });
	}

	edges.clear();
	for (int _p = 0; _p < static_cast<int>(polys.size()); ++_p)
	{
		polys[_p].firstEdge = static_cast<int>(edges.size());
		polys[_p].edgeCount = static_cast<int>(_byPoly[_p].size());
		edges.insert(edges.end(), _byPoly[_p].begin(), _byPoly[_p].end());
	}

	// Connected parts (links count both ways here: IsReachable is a quick answer, FindPath the exact one).
	std::vector<int> _parent(polys.size());
	std::iota(_parent.begin(), _parent.end(), 0);
	for (int _p = 0; _p < static_cast<int>(polys.size()); ++_p)
		for (int _e = polys[_p].firstEdge; _e < polys[_p].firstEdge + polys[_p].edgeCount; ++_e)
			_parent[Find(_parent, _p)] = Find(_parent, edges[_e].to);
	for (int _p = 0; _p < static_cast<int>(polys.size()); ++_p) polys[_p].component = Find(_parent, _p);

	++version;
}

int NavMesh::GetWalkableCells() const
{
	return static_cast<int>(std::count_if(walk.begin(), walk.end(), [](const std::uint8_t _c) { return _c != NAV_BLOCKED; }));
}

// ================= Queries =================

int NavMesh::PolyAt(const FVector2& _point) const
{
	if (!IsBuilt()) return -1;
	const int _x = static_cast<int>(std::floor((_point.x - settings.min.x) / settings.cellSize));
	const int _y = static_cast<int>(std::floor((_point.y - settings.min.y) / settings.cellSize));
	if (_x < 0 || _y < 0 || _x >= width || _y >= height) return -1;
	return cellPoly[Index(_x, _y)];
}

FVector2 NavMesh::ClampToPoly(const FVector2& _point, const NavPoly& _poly) const
{
	const FVector2 _min = PolyMin(_poly), _max = PolyMax(_poly);
	const float _inset = std::min(0.01f, settings.cellSize * 0.01f);	// strictly inside: PolyAt finds this polygon
	return FVector2(std::clamp(static_cast<float>(_point.x), _min.x + _inset, _max.x - _inset), std::clamp(static_cast<float>(_point.y), _min.y + _inset, _max.y - _inset));
}

bool NavMesh::Project(const FVector2& _point, const float& _extent, FVector2& _out) const
{
	if (!IsBuilt()) return false;
	if (const int _poly = PolyAt(_point); _poly >= 0) { _out = _point; return true; }

	const float _cs = settings.cellSize;
	const int _cx = static_cast<int>(std::floor((_point.x - settings.min.x) / _cs));
	const int _cy = static_cast<int>(std::floor((_point.y - settings.min.y) / _cs));
	const int _r = static_cast<int>(std::ceil(_extent / _cs)) + 1;
	float _best = _extent * _extent;
	int _bestPoly = -1;
	FVector2 _bestPoint;
	for (int _y = std::max(0, _cy - _r); _y <= std::min(height - 1, _cy + _r); ++_y)
		for (int _x = std::max(0, _cx - _r); _x <= std::min(width - 1, _cx + _r); ++_x)
		{
			const int _p = cellPoly[Index(_x, _y)];
			if (_p < 0) continue;
			const FVector2 _candidate = ClampToPoly(_point, polys[_p]);
			const float _d = (_candidate.x - _point.x) * (_candidate.x - _point.x) + (_candidate.y - _point.y) * (_candidate.y - _point.y);
			if (_d < _best) { _best = _d; _bestPoly = _p; _bestPoint = _candidate; }
		}
	if (_bestPoly < 0) return false;
	_out = _bestPoint;
	return true;
}

bool NavMesh::Raycast(const FVector2& _from, const FVector2& _to, FVector2* _hit) const
{
	if (PolyAt(_from) < 0) { if (_hit) *_hit = _from; return false; }
	const float _length = Distance(_from, _to);
	const int _steps = std::max(1, static_cast<int>(std::ceil(_length / (settings.cellSize * 0.25f))));
	FVector2 _last = _from;
	for (int _i = 1; _i <= _steps; ++_i)
	{
		const float _t = static_cast<float>(_i) / _steps;
		const FVector2 _p(_from.x + (_to.x - _from.x) * _t, _from.y + (_to.y - _from.y) * _t);
		if (PolyAt(_p) < 0)
		{
			if (_hit) *_hit = _last;
			return false;
		}
		_last = _p;
	}
	if (_hit) *_hit = _to;
	return true;
}

bool NavMesh::IsReachable(const FVector2& _from, const FVector2& _to) const
{
	const float _extent = settings.cellSize * 4.f + settings.agentRadius * 2.f;
	FVector2 _a, _b;
	if (!Project(_from, _extent, _a) || !Project(_to, _extent, _b)) return false;
	return polys[PolyAt(_a)].component == polys[PolyAt(_b)].component;
}

bool NavMesh::RandomPoint(const FVector2& _center, const float& _radius, FVector2& _out, std::mt19937& _random) const
{
	FVector2 _start;
	if (!Project(_center, settings.cellSize * 4.f + settings.agentRadius * 2.f, _start)) return false;
	const int _component = polys[PolyAt(_start)].component;

	// The polygons of the same part of the mesh that the circle touches, by their area.
	std::vector<int> _candidates;
	std::vector<float> _weights;
	for (int _p = 0; _p < static_cast<int>(polys.size()); ++_p)
	{
		if (polys[_p].component != _component) continue;
		const FVector2 _closest = ClampToPoly(_center, polys[_p]);
		if (Distance(_closest, _center) > _radius) continue;
		_candidates.push_back(_p);
		_weights.push_back(static_cast<float>(polys[_p].w * polys[_p].h));
	}
	if (_candidates.empty()) return false;
	std::discrete_distribution<int> _pick(_weights.begin(), _weights.end());
	std::uniform_real_distribution<float> _unit(0.f, 1.f);
	for (int _attempt = 0; _attempt < 24; ++_attempt)
	{
		const NavPoly& _poly = polys[_candidates[_pick(_random)]];
		const FVector2 _min = PolyMin(_poly), _max = PolyMax(_poly);
		const FVector2 _p(_min.x + (_max.x - _min.x) * _unit(_random), _min.y + (_max.y - _min.y) * _unit(_random));
		if (Distance(_p, _center) <= _radius) { _out = ClampToPoly(_p, _poly); return true; }
	}
	return false;
}

void NavMesh::StringPull(const std::vector<std::pair<FVector2, FVector2>>& _portals, std::vector<NavPathPoint>& _out) const
{
	// The funnel algorithm (Mikko Mononen's "simple stupid funnel"): portals are (left, right) pairs, the first and the last are points.
	FVector2 _apex = _portals[0].first, _left = _portals[0].first, _right = _portals[0].second;
	int _apexIndex = 0, _leftIndex = 0, _rightIndex = 0;
	const auto _add = [&](const FVector2& _p) { if (_out.empty() || !Same(_out.back().position, _p)) _out.push_back({ _p, false }); };
	_add(_apex);

	for (int _i = 1; _i < static_cast<int>(_portals.size()); ++_i)
	{
		const FVector2& _l = _portals[_i].first;
		const FVector2& _r = _portals[_i].second;

		if (TriArea2(_apex, _right, _r) <= 0.f)
		{
			if (Same(_apex, _right) || TriArea2(_apex, _left, _r) > 0.f) { _right = _r; _rightIndex = _i; }
			else
			{
				_add(_left);
				_apex = _left; _apexIndex = _leftIndex;
				_left = _right = _apex; _leftIndex = _rightIndex = _apexIndex;
				_i = _apexIndex;
				continue;
			}
		}
		if (TriArea2(_apex, _left, _l) >= 0.f)
		{
			if (Same(_apex, _left) || TriArea2(_apex, _right, _l) < 0.f) { _left = _l; _leftIndex = _i; }
			else
			{
				_add(_right);
				_apex = _right; _apexIndex = _rightIndex;
				_left = _right = _apex; _leftIndex = _rightIndex = _apexIndex;
				_i = _apexIndex;
				continue;
			}
		}
	}
	_add(_portals.back().first);
}

NavPath NavMesh::FindPath(const FVector2& _startPoint, const FVector2& _goalPoint) const
{
	NavPath _path;
	if (!IsBuilt() || polys.empty()) return _path;
	const float _extent = settings.cellSize * 4.f + settings.agentRadius * 2.f;
	FVector2 _start, _goal;
	if (!Project(_startPoint, _extent, _start)) return _path;
	const bool _goalOnMesh = Project(_goalPoint, _extent, _goal);
	if (!_goalOnMesh) _goal = _goalPoint;
	const int _startPoly = PolyAt(_start);
	const int _goalPoly = _goalOnMesh ? PolyAt(_goal) : -1;

	// ---- A* over the polygons; a polygon's position is where it was entered.
	const size_t _count = polys.size();
	if (cost.size() != _count) { cost.resize(_count); parent.resize(_count); parentEdge.resize(_count); entry.resize(_count); visited.assign(_count, 0); stamp = 0; }
	// A stamp per query, so nothing is cleared: visited holds the stamp (seen) or the stamp with the high bit (closed).
	const std::uint32_t CLOSED = 0x80000000u;
	if (++stamp >= CLOSED) { std::fill(visited.begin(), visited.end(), 0); stamp = 1; }

	using Item = std::pair<float, int>;
	std::priority_queue<Item, std::vector<Item>, std::greater<Item>> _queue;
	cost[_startPoly] = 0.f;
	parent[_startPoly] = -1;
	parentEdge[_startPoly] = -1;
	entry[_startPoly] = _start;
	visited[_startPoly] = stamp;
	_queue.push({ Distance(_start, _goal) * minCost, _startPoly });

	int _reached = -1;
	int _closest = _startPoly;
	float _closestDistance = Distance(ClampToPoly(_goal, polys[_startPoly]), _goal);
	while (!_queue.empty())
	{
		const int _p = _queue.top().second;
		_queue.pop();
		if (visited[_p] == (stamp | CLOSED)) continue;
		visited[_p] = stamp | CLOSED;
		if (_p == _goalPoly) { _reached = _p; break; }

		const float _distance = Distance(ClampToPoly(_goal, polys[_p]), _goal);
		if (_distance < _closestDistance) { _closestDistance = _distance; _closest = _p; }

		const NavPoly& _poly = polys[_p];
		for (int _e = _poly.firstEdge; _e < _poly.firstEdge + _poly.edgeCount; ++_e)
		{
			const NavEdge& _edge = edges[_e];
			const int _q = _edge.to;
			if (visited[_q] == (stamp | CLOSED)) continue;
			FVector2 _at;
			float _step;
			if (_edge.link >= 0)
			{
				_at = _edge.b;
				_step = Distance(entry[_p], _edge.a) * areaCosts[_poly.area] + Distance(_edge.a, _edge.b) * std::max(0.05f, links[_edge.link].cost);
			}
			else
			{
				// Entered at the middle of the portal.
				const FVector2 _mid((_edge.a.x + _edge.b.x) * 0.5f, (_edge.a.y + _edge.b.y) * 0.5f);
				_at = _mid;
				_step = Distance(entry[_p], _at) * areaCosts[_poly.area];
			}
			const float _new = cost[_p] + _step;
			if (visited[_q] == stamp && _new >= cost[_q]) continue;
			cost[_q] = _new;
			parent[_q] = _p;
			parentEdge[_q] = _e;
			entry[_q] = _at;
			visited[_q] = stamp;
			_queue.push({ _new + Distance(_at, _goal) * minCost, _q });
		}
	}

	int _target = _reached;
	FVector2 _end = _goal;
	if (_target < 0)
	{
		_target = _closest;	// cannot reach it: as close as possible
		_end = ClampToPoly(_goal, polys[_target]);
		_path.partial = true;
	}
	_path.cost = cost[_target] + Distance(entry[_target], _end) * areaCosts[polys[_target].area];

	// ---- The corridor, then the funnel between links.
	std::vector<int> _corridor;
	for (int _p = _target; _p >= 0 && parentEdge[_p] >= 0; _p = parent[_p]) _corridor.push_back(parentEdge[_p]);
	std::reverse(_corridor.begin(), _corridor.end());

	std::vector<std::pair<FVector2, FVector2>> _portals{ { _start, _start } };
	int _from = _startPoly;
	const auto _center = [&](const int _p)
	{
		const FVector2 _min = PolyMin(polys[_p]), _max = PolyMax(polys[_p]);
		return FVector2((_min.x + _max.x) * 0.5f, (_min.y + _max.y) * 0.5f);
	};
	for (const int _e : _corridor)
	{
		const NavEdge& _edge = edges[_e];
		if (_edge.link >= 0)
		{
			_portals.push_back({ _edge.a, _edge.a });
			StringPull(_portals, _path.points);
			_path.points.back().linkStart = true;
			_portals = { { _edge.b, _edge.b } };
		}
		else
		{
			// Left and right as seen when going from this polygon to the next.
			const FVector2 _c0 = _center(_from), _c1 = _center(_edge.to);
			const FVector2 _d(_c1.x - _c0.x, _c1.y - _c0.y);
			const FVector2 _mid((_edge.a.x + _edge.b.x) * 0.5f, (_edge.a.y + _edge.b.y) * 0.5f);
			const float _side = _d.x * (_edge.a.y - _mid.y) - _d.y * (_edge.a.x - _mid.x);
			_portals.push_back(_side > 0.f ? std::make_pair(_edge.a, _edge.b) : std::make_pair(_edge.b, _edge.a));
		}
		_from = _edge.to;
	}
	_portals.push_back({ _end, _end });
	StringPull(_portals, _path.points);

	for (size_t _i = 1; _i < _path.points.size(); ++_i) _path.length += Distance(_path.points[_i - 1].position, _path.points[_i].position);
	_path.found = true;
	return _path;
}
