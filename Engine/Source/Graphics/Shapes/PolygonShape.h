#pragma once
#include "Shape.h"
#include "Utilities/Math/Polygon2D.h"

namespace Krampus
{

	/// <summary>
	/// A shape made of any points, concave included (see Polygon2D). Its local (0, 0) is the pivot of the polygon: the shape is drawn
	/// around the position of its owner exactly as the points were placed in the Shape Editor. The texture covers the bounds of the points.
	/// </summary>
	class PolygonShape : public Shape
	{
	public:
		explicit PolygonShape(const Polygon2D& _polygon);

		void SetPolygon(const Polygon2D& _polygon);

		const Polygon2D& GetPolygon() const;
	};

}
