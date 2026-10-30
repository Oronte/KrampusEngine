#include "Polygon2D.h"
#include <sstream>

using namespace Krampus;

namespace
{
	constexpr float EPSILON = 1e-4f;

	float Cross(const FVector2& _a, const FVector2& _b, const FVector2& _c)
	{
		// (b - a) x (c - b): positive when a -> b -> c turns the same way as a polygon of positive area.
		return (_b.x - _a.x) * (_c.y - _b.y) - (_b.y - _a.y) * (_c.x - _b.x);
	}

	float Dot(const FVector2& _a, const FVector2& _b)
	{
		return _a.x * _b.x + _a.y * _b.y;
	}

	bool InTriangle(const FVector2& _p, const FVector2& _a, const FVector2& _b, const FVector2& _c)
	{
		// For a triangle of positive winding: inside or on an edge when the point is on the inner side of the three edges.
		return Cross(_a, _b, _p) >= -EPSILON && Cross(_b, _c, _p) >= -EPSILON && Cross(_c, _a, _p) >= -EPSILON;
	}

	bool SegmentsCross(const FVector2& _a, const FVector2& _b, const FVector2& _c, const FVector2& _d)
	{
		const auto _side = [](const FVector2& _p, const FVector2& _q, const FVector2& _r)
		{
			return (_q.x - _p.x) * (_r.y - _p.y) - (_q.y - _p.y) * (_r.x - _p.x);
		};
		const float _d1 = _side(_c, _d, _a), _d2 = _side(_c, _d, _b), _d3 = _side(_a, _b, _c), _d4 = _side(_a, _b, _d);
		return ((_d1 > EPSILON && _d2 < -EPSILON) || (_d1 < -EPSILON && _d2 > EPSILON))
			&& ((_d3 > EPSILON && _d4 < -EPSILON) || (_d3 < -EPSILON && _d4 > EPSILON));
	}

	bool IsConvexLoop(const std::vector<int>& _loop, const std::vector<FVector2>& _points)
	{
		const size_t _count = _loop.size();
		for (size_t _i = 0; _i < _count; ++_i)
			if (Cross(_points[_loop[_i]], _points[_loop[(_i + 1) % _count]], _points[_loop[(_i + 2) % _count]]) < -EPSILON)
				return false;
		return true;
	}

	/// <summary>Joins two loops (positive winding) that share the edge u-v: u -> v in _a, v -> u in _b.</summary>
	std::vector<int> Merge(const std::vector<int>& _a, const std::vector<int>& _b, const int _u, const int _v)
	{
		// _a from v around to u, then _b from u around to v without its two ends.
		std::vector<int> _merged;
		const size_t _startA = std::find(_a.begin(), _a.end(), _v) - _a.begin();
		for (size_t _i = 0; _i < _a.size(); ++_i)
			_merged.push_back(_a[(_startA + _i) % _a.size()]);
		const size_t _startB = std::find(_b.begin(), _b.end(), _u) - _b.begin();
		for (size_t _i = 1; _i + 1 < _b.size(); ++_i)
			_merged.push_back(_b[(_startB + _i) % _b.size()]);
		return _merged;
	}

	/// <summary>The edge u -> v of _a whose reverse v -> u is an edge of _b, if any.</summary>
	bool SharedEdge(const std::vector<int>& _a, const std::vector<int>& _b, int& _u, int& _v)
	{
		for (size_t _i = 0; _i < _a.size(); ++_i)
		{
			const int _from = _a[_i];
			const int _to = _a[(_i + 1) % _a.size()];
			for (size_t _j = 0; _j < _b.size(); ++_j)
				if (_b[_j] == _to && _b[(_j + 1) % _b.size()] == _from)
				{
					_u = _from;
					_v = _to;
					return true;
				}
		}
		return false;
	}
}

// ---- Polygon2D

bool Polygon2D::IsValid() const
{
	return points.size() >= 3 && Area() > 1e-3f;
}

float Polygon2D::SignedArea() const
{
	float _sum = 0.f;
	for (size_t _i = 0; _i < points.size(); ++_i)
	{
		const FVector2& _a = points[_i];
		const FVector2& _b = points[(_i + 1) % points.size()];
		_sum += _a.x * _b.y - _b.x * _a.y;
	}
	return _sum * 0.5f;
}

void Polygon2D::OrderPoints()
{
	if (SignedArea() < 0.f) std::reverse(points.begin(), points.end());
}

FVector2 Polygon2D::Centroid() const
{
	const float _area = SignedArea();
	if (points.empty()) return FVector2(0.f, 0.f);
	if (std::abs(_area) < EPSILON) return ConvexMath::Center(points);

	float _x = 0.f, _y = 0.f;
	for (size_t _i = 0; _i < points.size(); ++_i)
	{
		const FVector2& _a = points[_i];
		const FVector2& _b = points[(_i + 1) % points.size()];
		const float _cross = _a.x * _b.y - _b.x * _a.y;
		_x += (_a.x + _b.x) * _cross;
		_y += (_a.y + _b.y) * _cross;
	}
	return FVector2(_x / (6.f * _area), _y / (6.f * _area));
}

FRect Polygon2D::Bounds() const
{
	if (points.empty()) return FRect(FVector2(0.f, 0.f), FVector2(0.f, 0.f));
	float _minX = points[0].x, _maxX = points[0].x, _minY = points[0].y, _maxY = points[0].y;
	for (const FVector2& _point : points)
	{
		_minX = std::min(_minX, _point.x);
		_maxX = std::max(_maxX, _point.x);
		_minY = std::min(_minY, _point.y);
		_maxY = std::max(_maxY, _point.y);
	}
	return FRect(FVector2(_minX, _minY), FVector2(_maxX - _minX, _maxY - _minY));
}

bool Polygon2D::IsConvex() const
{
	if (points.size() < 3) return false;
	Polygon2D _ordered = *this;
	_ordered.OrderPoints();
	std::vector<int> _loop(points.size());
	for (size_t _i = 0; _i < _loop.size(); ++_i) _loop[_i] = static_cast<int>(_i);
	return IsConvexLoop(_loop, _ordered.points);
}

bool Polygon2D::IsSimple() const
{
	const size_t _count = points.size();
	if (_count < 3) return false;
	for (size_t _i = 0; _i < _count; ++_i)
		for (size_t _j = _i + 1; _j < _count; ++_j)
		{
			// Neighbor edges share a point: they do not count.
			if (_j == _i + 1 || (_i == 0 && _j == _count - 1)) continue;
			if (SegmentsCross(points[_i], points[(_i + 1) % _count], points[_j], points[(_j + 1) % _count])) return false;
		}
	return true;
}

bool Polygon2D::Contains(const FVector2& _point) const
{
	// Even-odd rule: a ray to the right crosses the edges an odd number of times when the point is inside.
	bool _inside = false;
	for (size_t _i = 0, _j = points.size() - 1; _i < points.size(); _j = _i++)
	{
		const FVector2& _a = points[_i];
		const FVector2& _b = points[_j];
		if ((_a.y > _point.y) != (_b.y > _point.y) && _point.x < (_b.x - _a.x) * (_point.y - _a.y) / (_b.y - _a.y) + _a.x)
			_inside = !_inside;
	}
	return _inside;
}

std::vector<std::array<int, 3>> Polygon2D::Triangulate() const
{
	std::vector<std::array<int, 3>> _triangles;
	if (points.size() < 3) return _triangles;

	// Ear clipping on the indices, in the positive winding.
	std::vector<int> _remaining(points.size());
	for (size_t _i = 0; _i < _remaining.size(); ++_i) _remaining[_i] = static_cast<int>(_i);
	if (SignedArea() < 0.f) std::reverse(_remaining.begin(), _remaining.end());

	size_t _guard = 0;
	while (_remaining.size() > 3 && _guard++ < points.size() * points.size())
	{
		bool _found = false;
		const size_t _count = _remaining.size();
		for (size_t _i = 0; _i < _count; ++_i)
		{
			const int _prev = _remaining[(_i + _count - 1) % _count];
			const int _current = _remaining[_i];
			const int _next = _remaining[(_i + 1) % _count];
			const FVector2& _a = points[_prev];
			const FVector2& _b = points[_current];
			const FVector2& _c = points[_next];
			if (Cross(_a, _b, _c) <= EPSILON) continue;	// a corner turning inward (or flat) is not an ear

			bool _empty = true;
			for (const int _other : _remaining)
			{
				if (_other == _prev || _other == _current || _other == _next) continue;
				if (InTriangle(points[_other], _a, _b, _c))
				{
					_empty = false;
					break;
				}
			}
			if (!_empty) continue;

			_triangles.push_back({ _prev, _current, _next });
			_remaining.erase(_remaining.begin() + _i);
			_found = true;
			break;
		}

		// No ear: the edges cross or points are flat. Finish with a fan so that something is drawn.
		if (!_found)
		{
			for (size_t _i = 1; _i + 1 < _remaining.size(); ++_i)
				_triangles.push_back({ _remaining[0], _remaining[_i], _remaining[_i + 1] });
			return _triangles;
		}
	}
	if (_remaining.size() == 3) _triangles.push_back({ _remaining[0], _remaining[1], _remaining[2] });
	return _triangles;
}

std::vector<std::vector<FVector2>> Polygon2D::ConvexParts(const int _maxPoints) const
{
	std::vector<std::vector<FVector2>> _parts;
	if (points.size() < 3) return _parts;

	// A convex polygon small enough is its own only part.
	Polygon2D _ordered = *this;
	_ordered.OrderPoints();
	if (static_cast<int>(_ordered.points.size()) <= _maxPoints && _ordered.IsConvex())
	{
		_parts.push_back(_ordered.points);
		return _parts;
	}

	// Hertel-Mehlhorn: start from triangles and join two pieces across their shared edge while the result stays convex.
	std::vector<std::vector<int>> _pieces;
	for (const std::array<int, 3>& _triangle : _ordered.Triangulate())
		if (std::abs(Cross(_ordered.points[_triangle[0]], _ordered.points[_triangle[1]], _ordered.points[_triangle[2]])) > EPSILON)
			_pieces.push_back({ _triangle[0], _triangle[1], _triangle[2] });

	bool _merged = true;
	while (_merged)
	{
		_merged = false;
		for (size_t _i = 0; _i < _pieces.size() && !_merged; ++_i)
			for (size_t _j = _i + 1; _j < _pieces.size() && !_merged; ++_j)
			{
				int _u = 0, _v = 0;
				if (!SharedEdge(_pieces[_i], _pieces[_j], _u, _v)) continue;
				if (static_cast<int>(_pieces[_i].size() + _pieces[_j].size()) - 2 > _maxPoints) continue;

				std::vector<int> _joined = Merge(_pieces[_i], _pieces[_j], _u, _v);
				if (!IsConvexLoop(_joined, _ordered.points)) continue;

				_pieces[_i] = std::move(_joined);
				_pieces.erase(_pieces.begin() + _j);
				_merged = true;
			}
	}

	for (const std::vector<int>& _piece : _pieces)
	{
		std::vector<FVector2> _points;
		for (const int _index : _piece) _points.push_back(_ordered.points[_index]);
		_parts.push_back(std::move(_points));
	}
	return _parts;
}

std::vector<FVector2> Polygon2D::Transform(const std::vector<FVector2>& _points, const FVector2& _position, const Angle& _rotation, const FVector2& _scale)
{
	const float _radians = _rotation.ToRadians();
	const float _cos = std::cos(_radians), _sin = std::sin(_radians);
	std::vector<FVector2> _result;
	_result.reserve(_points.size());
	for (const FVector2& _point : _points)
	{
		const float _x = _point.x * _scale.x, _y = _point.y * _scale.y;
		_result.emplace_back(_position.x + _x * _cos - _y * _sin, _position.y + _x * _sin + _y * _cos);
	}
	return _result;
}

Polygon2D Polygon2D::Regular(const int _sides, const float _radius, const float _startDegrees)
{
	Polygon2D _polygon;
	const int _count = std::max(3, _sides);
	for (int _i = 0; _i < _count; ++_i)
	{
		const float _angle = (_startDegrees + 360.f * _i / _count) * 3.14159265f / 180.f;
		_polygon.points.emplace_back(std::round(std::cos(_angle) * _radius * 100.f) / 100.f, std::round(std::sin(_angle) * _radius * 100.f) / 100.f);
	}
	return _polygon;
}

Polygon2D Polygon2D::Star(const int _branches, const float _outerRadius, const float _innerRadius)
{
	Polygon2D _polygon;
	const int _count = std::max(3, _branches) * 2;
	for (int _i = 0; _i < _count; ++_i)
	{
		const float _radius = _i % 2 == 0 ? _outerRadius : _innerRadius;
		const float _angle = (-90.f + 360.f * _i / _count) * 3.14159265f / 180.f;
		_polygon.points.emplace_back(std::round(std::cos(_angle) * _radius * 100.f) / 100.f, std::round(std::sin(_angle) * _radius * 100.f) / 100.f);
	}
	return _polygon;
}

Polygon2D Polygon2D::Box(const FVector2& _size)
{
	const float _x = _size.x * 0.5f, _y = _size.y * 0.5f;
	return Polygon2D({ FVector2(-_x, -_y), FVector2(_x, -_y), FVector2(_x, _y), FVector2(-_x, _y) });
}

std::string Polygon2D::Encode() const
{
	std::ostringstream _text;
	for (size_t _i = 0; _i < points.size(); ++_i)
		_text << (_i ? "; " : "") << points[_i].x << ", " << points[_i].y;
	return _text.str();
}

Polygon2D Polygon2D::Decode(const std::string& _text)
{
	Polygon2D _polygon;
	std::stringstream _stream(_text);
	std::string _pair;
	while (std::getline(_stream, _pair, ';'))
	{
		const size_t _comma = _pair.find(',');
		if (_comma == std::string::npos) continue;
		try
		{
			_polygon.points.emplace_back(std::stof(_pair.substr(0, _comma)), std::stof(_pair.substr(_comma + 1)));
		}
		catch (const std::exception&) { }
	}
	return _polygon;
}

// ---- ConvexMath

FVector2 ConvexMath::Center(const std::vector<FVector2>& _points)
{
	FVector2 _sum(0.f, 0.f);
	for (const FVector2& _point : _points) _sum = FVector2(_sum.x + _point.x, _sum.y + _point.y);
	const float _count = static_cast<float>(std::max<size_t>(1, _points.size()));
	return FVector2(_sum.x / _count, _sum.y / _count);
}

bool ConvexMath::Overlap(const std::vector<FVector2>& _a, const std::vector<FVector2>& _b, FVector2& _normal, float& _depth)
{
	_depth = std::numeric_limits<float>::max();
	const auto _testAxes = [&](const std::vector<FVector2>& _shape) -> bool
	{
		for (size_t _i = 0; _i < _shape.size(); ++_i)
		{
			const FVector2& _p = _shape[_i];
			const FVector2& _q = _shape[(_i + 1) % _shape.size()];
			FVector2 _axis(-(_q.y - _p.y), _q.x - _p.x);
			const float _length = std::sqrt(Dot(_axis, _axis));
			if (_length < EPSILON) continue;
			_axis = FVector2(_axis.x / _length, _axis.y / _length);

			float _minA = std::numeric_limits<float>::max(), _maxA = -_minA, _minB = _minA, _maxB = -_minA;
			for (const FVector2& _point : _a) { const float _d = Dot(_point, _axis); _minA = std::min(_minA, _d); _maxA = std::max(_maxA, _d); }
			for (const FVector2& _point : _b) { const float _d = Dot(_point, _axis); _minB = std::min(_minB, _d); _maxB = std::max(_maxB, _d); }

			const float _overlap = std::min(_maxA, _maxB) - std::max(_minA, _minB);
			if (_overlap <= 0.f) return false;
			if (_overlap < _depth)
			{
				_depth = _overlap;
				_normal = _axis;
			}
		}
		return true;
	};
	if (_a.size() < 3 || _b.size() < 3) return false;
	if (!_testAxes(_a) || !_testAxes(_b)) return false;

	// From a to b.
	const FVector2 _centerA = Center(_a), _centerB = Center(_b);
	if (Dot(FVector2(_centerB.x - _centerA.x, _centerB.y - _centerA.y), _normal) < 0.f)
		_normal = FVector2(-_normal.x, -_normal.y);
	return true;
}

bool ConvexMath::OverlapCircle(const FVector2& _center, const float _radius, const std::vector<FVector2>& _polygon, FVector2& _normal, float& _depth, FVector2& _contact)
{
	if (_polygon.size() < 3) return false;

	// The closest point of the edges, and whether the center is inside.
	float _best = std::numeric_limits<float>::max();
	FVector2 _closest(0.f, 0.f);
	bool _inside = true;
	const FVector2 _middle = Center(_polygon);
	for (size_t _i = 0; _i < _polygon.size(); ++_i)
	{
		const FVector2& _p = _polygon[_i];
		const FVector2& _q = _polygon[(_i + 1) % _polygon.size()];
		const FVector2 _edge(_q.x - _p.x, _q.y - _p.y);
		const float _lengthSquared = std::max(EPSILON, Dot(_edge, _edge));
		const float _t = std::clamp(Dot(FVector2(_center.x - _p.x, _center.y - _p.y), _edge) / _lengthSquared, 0.f, 1.f);
		const FVector2 _point(_p.x + _edge.x * _t, _p.y + _edge.y * _t);
		const float _distance = std::sqrt(Dot(FVector2(_center.x - _point.x, _center.y - _point.y), FVector2(_center.x - _point.x, _center.y - _point.y)));
		if (_distance < _best)
		{
			_best = _distance;
			_closest = _point;
		}

		// Outside of an edge: the center and the middle of the polygon are on different sides of it.
		const float _sideCenter = _edge.x * (_center.y - _p.y) - _edge.y * (_center.x - _p.x);
		const float _sideMiddle = _edge.x * (_middle.y - _p.y) - _edge.y * (_middle.x - _p.x);
		if (_sideCenter * _sideMiddle < 0.f) _inside = false;
	}

	if (!_inside && _best >= _radius) return false;

	_contact = _closest;
	FVector2 _direction(_closest.x - _center.x, _closest.y - _center.y);
	const float _length = std::sqrt(Dot(_direction, _direction));
	if (_length < EPSILON)
	{
		_direction = FVector2(_middle.x - _center.x, _middle.y - _center.y);
		const float _fallback = std::max(EPSILON, std::sqrt(Dot(_direction, _direction)));
		_direction = FVector2(_direction.x / _fallback, _direction.y / _fallback);
	}
	else
		_direction = FVector2(_direction.x / _length, _direction.y / _length);

	// Inside: the circle must go out through the closest edge, the other way.
	_normal = _inside ? FVector2(-_direction.x, -_direction.y) : _direction;
	_depth = _inside ? _radius + _best : _radius - _best;
	return true;
}
