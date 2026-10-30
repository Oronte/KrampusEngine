#include "PolygonShape.h"

using namespace Krampus;

namespace
{
	/// <summary>
	/// sf::Shape draws its inside as a fan around its center: right for convex shapes only. This one keeps everything sf::Shape gives
	/// (transform, texture, colors, bounds) and draws its own triangles, so a concave polygon is filled where it should be.
	/// </summary>
	class PolygonSfShape : public sf::Shape
	{
		Polygon2D								polygon;
		std::vector<std::array<int, 3>>		triangles;
		std::vector<sf::Vector2f>			outlineNormals;		// outward, scaled for the corner, one per point

	public:
		explicit PolygonSfShape(const Polygon2D& _polygon)
		{
			SetPolygon(_polygon);
		}

		void SetPolygon(const Polygon2D& _polygon)
		{
			polygon = _polygon;
			if (polygon.points.size() < 3) polygon = Polygon2D::Regular(6, 50.f);
			triangles = polygon.Triangulate();
			ComputeOutlineNormals();
			update();
		}

		const Polygon2D& GetPolygon() const
		{
			return polygon;
		}

		std::size_t getPointCount() const override
		{
			return polygon.points.size();
		}

		sf::Vector2f getPoint(const std::size_t _index) const override
		{
			const FVector2& _point = polygon.points[_index];
			return sf::Vector2f(_point.x, _point.y);
		}

	private:
		void ComputeOutlineNormals()
		{
			// For each corner: the direction that moves it outward along both of its edges (a miter, capped for sharp corners).
			const size_t _count = polygon.points.size();
			const float _outward = polygon.SignedArea() > 0.f ? 1.f : -1.f;
			outlineNormals.assign(_count, sf::Vector2f());
			const auto _edgeNormal = [&](const FVector2& _a, const FVector2& _b)
			{
				sf::Vector2f _normal(_b.y - _a.y, -(_b.x - _a.x));
				const float _length = std::sqrt(_normal.x * _normal.x + _normal.y * _normal.y);
				if (_length > 0.f) _normal /= _length;
				return _normal * _outward;
			};
			for (size_t _i = 0; _i < _count; ++_i)
			{
				const FVector2& _previous = polygon.points[(_i + _count - 1) % _count];
				const FVector2& _current = polygon.points[_i];
				const FVector2& _next = polygon.points[(_i + 1) % _count];
				const sf::Vector2f _n1 = _edgeNormal(_previous, _current);
				const sf::Vector2f _n2 = _edgeNormal(_current, _next);
				const float _factor = std::max(0.25f, 1.f + _n1.x * _n2.x + _n1.y * _n2.y);
				outlineNormals[_i] = (_n1 + _n2) / _factor;
			}
		}

		void draw(sf::RenderTarget& _target, sf::RenderStates _states) const override
		{
			_states.transform *= getTransform();
			_states.coordinateType = sf::CoordinateType::Pixels;

			// The texture is stretched over the bounds of the points, like an sf::Shape does with its texture rectangle.
			const FRect _bounds = polygon.Bounds();
			const sf::IntRect _rect = getTextureRect();
			const float _width = std::max(1e-3f, _bounds.GetSize().x), _height = std::max(1e-3f, _bounds.GetSize().y);
			const auto _texCoords = [&](const FVector2& _point)
			{
				return sf::Vector2f(
					static_cast<float>(_rect.position.x) + (_point.x - _bounds.GetPosition().x) / _width * static_cast<float>(_rect.size.x),
					static_cast<float>(_rect.position.y) + (_point.y - _bounds.GetPosition().y) / _height * static_cast<float>(_rect.size.y));
			};

			std::vector<sf::Vertex> _fill;
			_fill.reserve(triangles.size() * 3);
			const sf::Color _color = getFillColor();
			for (const std::array<int, 3>& _triangle : triangles)
				for (const int _index : _triangle)
				{
					const FVector2& _point = polygon.points[_index];
					_fill.push_back(sf::Vertex{ sf::Vector2f(_point.x, _point.y), _color, _texCoords(_point) });
				}

			sf::RenderStates _fillStates = _states;
			_fillStates.texture = getTexture();
			if (!_fill.empty()) _target.draw(_fill.data(), _fill.size(), sf::PrimitiveType::Triangles, _fillStates);

			// The outline: a strip along the edges, outward for a positive thickness.
			const float _thickness = getOutlineThickness();
			if (_thickness == 0.f) return;
			std::vector<sf::Vertex> _outline;
			_outline.reserve((polygon.points.size() + 1) * 2);
			const sf::Color _outlineColor = getOutlineColor();
			for (size_t _i = 0; _i <= polygon.points.size(); ++_i)
			{
				const size_t _index = _i % polygon.points.size();
				const sf::Vector2f _point(polygon.points[_index].x, polygon.points[_index].y);
				_outline.push_back(sf::Vertex{ _point, _outlineColor });
				_outline.push_back(sf::Vertex{ _point + outlineNormals[_index] * _thickness, _outlineColor });
			}
			sf::RenderStates _outlineStates = _states;
			_outlineStates.texture = nullptr;
			_target.draw(_outline.data(), _outline.size(), sf::PrimitiveType::TriangleStrip, _outlineStates);
		}
	};
}

PolygonShape::PolygonShape(const Polygon2D& _polygon)
{
	shape = std::make_unique<PolygonSfShape>(_polygon);
	shape->setOrigin(sf::Vector2f(0.f, 0.f));	// (0, 0) of the points is the pivot
}

void PolygonShape::SetPolygon(const Polygon2D& _polygon)
{
	static_cast<PolygonSfShape*>(shape.get())->SetPolygon(_polygon);
}

const Polygon2D& PolygonShape::GetPolygon() const
{
	return static_cast<const PolygonSfShape*>(shape.get())->GetPolygon();
}
