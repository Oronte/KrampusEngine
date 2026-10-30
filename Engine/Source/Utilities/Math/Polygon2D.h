#pragma once
#include "Rectangle.h"
#include "Angle.h"
#include <array>

namespace Krampus
{

	/// <summary>
	/// A closed shape made of points (in pixels, around the actor's position: (0, 0) is the pivot). It may be concave; its edges must
	/// not cross each other. What uses it splits it as it needs:
	///   - Triangulate() for drawing (any simple polygon),
	///   - ConvexParts() for collisions and box2d (convex pieces of at most 8 points).
	/// Made in the Shape Editor and stored in Content/Shapes (see ShapeAssets).
	/// </summary>
	class Polygon2D
	{
	public:
		std::vector<FVector2>	points;

		Polygon2D() = default;
		explicit Polygon2D(std::vector<FVector2> _points) : points(std::move(_points)) { }

		/// <returns>True with at least 3 points and an area.</returns>
		bool IsValid() const;

		/// <returns>Area with a sign (shoelace formula): positive when the points turn clockwise on screen (y going down).</returns>
		float SignedArea() const;

		float Area() const { return std::abs(SignedArea()); }

		/// <summary>Reverses the points if needed so that SignedArea() is positive: the winding Triangulate and ConvexParts work in.</summary>
		void OrderPoints();

		FVector2 Centroid() const;

		/// <returns>The smallest rectangle around the points (position = top left).</returns>
		FRect Bounds() const;

		/// <returns>True when no corner turns inward (collinear points are allowed).</returns>
		bool IsConvex() const;

		/// <returns>True when no two edges cross each other.</returns>
		bool IsSimple() const;

		bool Contains(const FVector2& _point) const;

		/// <returns>Triangles (indices in points) that cover the polygon. Ear clipping: works for any simple polygon.</returns>
		std::vector<std::array<int, 3>> Triangulate() const;

		/// <returns>Convex pieces (points, positive winding) of at most _maxPoints points that cover the polygon.</returns>
		std::vector<std::vector<FVector2>> ConvexParts(int _maxPoints = 8) const;

		/// <returns>The points moved to _position, turned by _rotation and scaled by _scale.</returns>
		static std::vector<FVector2> Transform(const std::vector<FVector2>& _points, const FVector2& _position, const Angle& _rotation, const FVector2& _scale = FVector2(1.f, 1.f));

		// ---- Ready-made shapes

		static Polygon2D Regular(int _sides, float _radius, float _startDegrees = -90.f);
		static Polygon2D Star(int _branches, float _outerRadius, float _innerRadius);
		static Polygon2D Box(const FVector2& _size);

		// ---- Text form: "x,y; x,y; ..."

		std::string Encode() const;
		static Polygon2D Decode(const std::string& _text);
	};

	/// <summary>Convex polygon helpers shared by the collision code (points of a convex polygon, any winding).</summary>
	namespace ConvexMath
	{
		/// <summary>Separating axis test between two convex polygons. _normal goes from _a to _b, _depth is the overlap.</summary>
		bool Overlap(const std::vector<FVector2>& _a, const std::vector<FVector2>& _b, FVector2& _normal, float& _depth);

		/// <summary>A circle against a convex polygon. _normal goes from the circle to the polygon.</summary>
		bool OverlapCircle(const FVector2& _center, float _radius, const std::vector<FVector2>& _polygon, FVector2& _normal, float& _depth, FVector2& _contact);

		FVector2 Center(const std::vector<FVector2>& _points);
	}

}
