#pragma once
#include "Components/BasicLabel.h"

namespace Krampus
{

	/// <summary>A text drawn on the HUD, at its widget's transform.</summary>
	class LabelWidgetComponent : public BasicLabel<WidgetComponent>
	{
	public:
		LabelWidgetComponent(Widget* _owner, const TextData& _data) : BasicLabel(_owner, _data) { name = NAME_OF(LabelWidgetComponent); }
	};

}
