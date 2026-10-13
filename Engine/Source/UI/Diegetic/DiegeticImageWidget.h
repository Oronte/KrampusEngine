#pragma once
#include "DiegeticWidget.h"
#include "Components/SpriteComponent.h"

namespace Krampus
{

	class DiegeticImageWidget : public DiegeticWidget
	{
	protected:
		SpriteComponent*	sprite	= nullptr;

	public:
		DiegeticImageWidget(Level* _level, const CircleShapeData& _data);
		DiegeticImageWidget(Level* _level, const RectangleShapeData& _data);

		virtual std::string ToString() const override;
	};

}
