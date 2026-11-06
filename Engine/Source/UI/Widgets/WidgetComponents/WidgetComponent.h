#pragma once
#include "Components/OwnedComponent.h"

namespace Krampus
{
	class Widget;

	/// <summary>
	/// Base class for the components of a HUD widget (screen space).
	/// </summary>
	class WidgetComponent : public OwnedComponent<Widget>
	{
	protected:
		explicit WidgetComponent(Widget* _owner) : OwnedComponent(_owner) { name = NAME_OF(WidgetComponent); }
	};
}
