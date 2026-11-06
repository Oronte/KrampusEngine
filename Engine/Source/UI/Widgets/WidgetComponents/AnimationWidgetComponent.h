#pragma once
#include "Components/BasicAnimation.h"

namespace Krampus
{

	/// <summary>Sprite-sheet animations of the widget's ImageWidgetComponent.</summary>
	class AnimationWidgetComponent : public BasicAnimation<WidgetComponent, ImageWidgetComponent>
	{
	public:
		explicit AnimationWidgetComponent(Widget* _owner) : BasicAnimation(_owner) { name = NAME_OF(AnimationWidgetComponent); }
	};

}
