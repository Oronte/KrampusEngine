#pragma once
#include "Widget.h"
#include "Components/SpriteComponent.h"

namespace Krampus
{

	class ImageWidget : public Widget
	{
	protected:
		SpriteComponent*	sprite	= nullptr;

	public:
		ImageWidget(Level* _level, const CircleShapeData& _data);
		ImageWidget(Level* _level, const RectangleShapeData& _data);

		virtual std::string ToString() const override;
	};

}
