#pragma once
#include "Component.h"
#include "UI/Widgets/WidgetComponents/WidgetComponent.h"
#include "Utilities/System/Drawable.h"

namespace Krampus
{

	/// <summary>
	/// A component drawn by its level's camera: in the world view for an actor component, in the HUD view for a widget
	/// component. It is in the camera's lists from Construct to Deconstruct, at its zOrder.
	/// </summary>
	template <typename Base>
	class DrawnComponent : public Base, public IDrawable
	{
		bool drawn = false;

	public:
		void Construct() override;
		void Deconstruct() override;

		/// <summary>Moves the component to another drawing layer (higher = drawn later, on top).</summary>
		void SetZOrder(const uint8_t& _zOrder) override;
		void SetZOrder(const ZOrder& _zOrder) override { SetZOrder(static_cast<uint8_t>(_zOrder)); }

	protected:
		explicit DrawnComponent(typename Base::OwnerType* _owner) : Base(_owner) {}
	};

	extern template class DrawnComponent<Component>;
	extern template class DrawnComponent<WidgetComponent>;

}
