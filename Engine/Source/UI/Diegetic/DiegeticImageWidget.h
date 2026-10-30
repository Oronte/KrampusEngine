#pragma once
#include "DiegeticWidget.h"
#include "Components/SpriteComponent.h"

namespace Krampus
{

	/// <summary>
	/// Static in-world diegetic image widget displaying a textured 2D shape.
	/// </summary>
	class DiegeticImageWidget : public DiegeticWidget
	{
	protected:
		SpriteComponent*	sprite	= nullptr;

	public:
		/// <summary>Constructs a circular diegetic image.</summary>
		/// <param name="_level">Owning Level.</param>
		/// <param name="_data">Circle shape parameters.</param>
		/// <summary>A 200 x 80 rectangle: what the editor spawns (change its SpriteComponent in the Inspector).</summary>
		explicit DiegeticImageWidget(Level* _level);

		DiegeticImageWidget(Level* _level, const CircleShapeData& _data);

		/// <summary>Constructs a rectangular diegetic image.</summary>
		/// <param name="_level">Owning Level.</param>
		/// <param name="_data">Rectangle shape parameters.</param>
		DiegeticImageWidget(Level* _level, const RectangleShapeData& _data);

		/// <summary>Returns a string representation of the diegetic image.</summary>
		/// <returns>String representation.</returns>
		virtual std::string ToString() const override;
	};

}
