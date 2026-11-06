#pragma once
#include "Components/BasicImage.h"

namespace Krampus
{

	/// <summary>A shape drawn on the HUD at its widget's transform.</summary>
	class ImageWidgetComponent : public BasicImage<WidgetComponent>
	{
	public:
		ImageWidgetComponent(Widget* _owner, const CircleShapeData& _data)	  : BasicImage(_owner, _data) { name = NAME_OF(ImageWidgetComponent); }
		ImageWidgetComponent(Widget* _owner, const RectangleShapeData& _data) : BasicImage(_owner, _data) { name = NAME_OF(ImageWidgetComponent); }
	};

}
