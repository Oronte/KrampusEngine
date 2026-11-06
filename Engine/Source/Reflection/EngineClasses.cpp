#include "EngineClassHelpers.h"


// Everything the Inspector shows for the classes of the engine. The classes themselves stay untouched: values are read
// and written through their public members and Get/Set methods. Classes of the game use KACTOR / KCOMPONENT / KPROPERTY.
using namespace Krampus;
using namespace Krampus::EngineClassHelpers;

void Krampus::RegisterEngineClasses()
{
	// ================= Bases =================
	Describe("IGameObject", "");
	Describe("Actor", "IGameObject");
	Describe("Component", "IGameObject");
	Describe("Widget", "IGameObject").widgetFactory = [](Level* _level) -> Widget*
	{
		return _level->GetHUD().CreateWidget<Widget>(_level);
	};
	Describe("WidgetComponent", "IGameObject");

	// ================= Actors =================
	// CameraActor and EmptyActor describe themselves with KACTOR.

	// Diegetic widgets: actors of the world kept at a place of the screen. The base cannot be spawned (a class can derive from it).
	{
		ClassInfo& _info = Describe("DiegeticWidget", "Actor");
		_info.AddAccessor<DiegeticWidget, FVector2>("screenPosition",	// window pixels
			[](DiegeticWidget& _w) { return _w.GetScreenPosition(); },
			[](DiegeticWidget& _w, const FVector2& _v) { _w.SetScreenPosition(_v); });
		_info.AddAccessor<DiegeticWidget, float>("screenRotation",		// degrees
			[](DiegeticWidget& _w) { return static_cast<float>(_w.GetLocalRotation().ToDegrees()); },
			[](DiegeticWidget& _w, const float& _v) { _w.SetLocalRotation(Angle::FromDegrees(_v)); });
	}
	Describe("DiegeticImageWidget", "DiegeticWidget").factory = [](Level* _level) -> Actor* { return _level->SpawnActor<DiegeticImageWidget>(); };
	Describe("DiegeticLabelWidget", "DiegeticWidget").factory = [](Level* _level) -> Actor* { return _level->SpawnActor<DiegeticLabelWidget>(); };
	{
		ClassInfo& _info = Describe("DiegeticButtonWidget", "DiegeticImageWidget");
		_info.factory = [](Level* _level) -> Actor* { return _level->SpawnActor<DiegeticButtonWidget>(); };
		_info.AddProperty<DiegeticButtonWidget, EventList>("onPress", &DiegeticButtonWidget::onPressCalls);
		_info.AddProperty<DiegeticButtonWidget, EventList>("onRelease", &DiegeticButtonWidget::onReleaseCalls);
		_info.AddProperty<DiegeticButtonWidget, EventList>("onHover", &DiegeticButtonWidget::onHoverCalls);
		_info.AddProperty<DiegeticButtonWidget, EventList>("onUnhover", &DiegeticButtonWidget::onUnhoverCalls);
	}

	RegisterComponentClasses();
	RegisterEffectClasses();
	RegisterWidgetClasses();
	RegisterFunctionClasses();
}
