#include "EngineClassHelpers.h"

using namespace Krampus;
using namespace Krampus::EngineClassHelpers;

namespace Krampus::EngineClassHelpers
{
	void RegisterWidgetClasses()
	{
	// ================= Settings (the Settings window of the editor) =================
	{
		ClassInfo& _info = Describe("LoggerSettings", "IGameObject");
		AddLoggerFlag(_info, "verboseLogging", Logger::verboseLogging);
		AddLoggerFlag(_info, "veryVerboseLogging", Logger::veryVerboseLogging);
		AddLoggerFlag(_info, "colorText", Logger::canColorText);
		AddLoggerFlag(_info, "playSounds", Logger::canPlaySound);
		AddLoggerFlag(_info, "messageSound", Logger::canPlayMsgSound);
		AddLoggerFlag(_info, "warningSound", Logger::canPlayWarningSound);
		AddLoggerFlag(_info, "errorSound", Logger::canPlayErrorSound);
		AddLoggerFlag(_info, "fatalSound", Logger::canPlayFatalSound);
	}

	{
		ClassInfo& _info = Describe("PhysicsSettings", "IGameObject");
		AddLive(_info, "gravity", &PhysicsSettings::gravity);
		AddLive(_info, "subSteps", &PhysicsSettings::subSteps, 1.f, 32.f);
		AddLive(_info, "sleeping", &PhysicsSettings::sleeping);
		AddLive(_info, "continuous", &PhysicsSettings::continuous);
		AddLive(_info, "warmStarting", &PhysicsSettings::warmStarting);
		AddLive(_info, "maxLinearSpeed", &PhysicsSettings::maxLinearSpeed, 1.f, 2000.f);
		AddLive(_info, "restitutionThreshold", &PhysicsSettings::restitutionThreshold, 0.f, 20.f);
		AddLive(_info, "hitEventThreshold", &PhysicsSettings::hitEventThreshold, 0.f, 20.f);
		AddLive(_info, "contactHertz", &PhysicsSettings::contactHertz, 1.f, 120.f);
		AddLive(_info, "contactDampingRatio", &PhysicsSettings::contactDampingRatio, 0.f, 50.f);
		AddLive(_info, "maxContactPushSpeed", &PhysicsSettings::maxContactPushSpeed, 0.f, 20.f);
	}

	{
		ClassInfo& _info = Describe("TimeSettings", "IGameObject");
		AddLive(_info, "timeScale", &TimeSettings::timeScale, 0.f, 5.f);
		AddLive(_info, "maxFrameRate", &TimeSettings::maxFrameRate, 0.f, 1000.f);	// 0 = unlimited
	}

	{
		ClassInfo& _info = Describe("WindowSettings", "IGameObject");
		AddLive(_info, "verticalSync", &WindowSettings::verticalSync);
	}

	{
		ClassInfo& _info = Describe("AudioSettings", "IGameObject");
		AddLive(_info, "globalVolume", &AudioSettings::globalVolume, 0.f, 100.f);
	}

	{
		ClassInfo& _info = Describe("ShipSettings", "IGameObject");
		_info.AddProperty<ShipSettings, std::string>("startScene", &ShipSettings::startScene);
		_info.SetLastPropertyType(PropertyType::Scene);
		_info.AddProperty<ShipSettings, std::string>("gameName", &ShipSettings::gameName);
		_info.AddProperty<ShipSettings, int>("windowMode", &ShipSettings::windowMode, 0.f, 2.f);
		_info.SetLastPropertyType(PropertyType::Enum, { "Borderless (desktop size)", "Windowed", "Fullscreen" });
		_info.AddProperty<ShipSettings, int>("windowWidth", &ShipSettings::windowWidth, 320.f, 7680.f);
		_info.AddProperty<ShipSettings, int>("windowHeight", &ShipSettings::windowHeight, 240.f, 4320.f);
		_info.AddProperty<ShipSettings, bool>("showSplash", &ShipSettings::showSplash);
		_info.AddProperty<ShipSettings, std::string>("icon", &ShipSettings::icon);
		_info.SetLastPropertyType(PropertyType::Texture, { "(Krampus logo)" });
		_info.AddProperty<ShipSettings, std::string>("packageFolder", &ShipSettings::packageFolder);
	}

	// ================= HUD widgets =================
	{
		ClassInfo& _info = Describe("ButtonWidget", "Widget");
		_info.widgetFactory = [](Level* _level) -> Widget*
		{
			return _level->GetHUD().CreateWidget<ButtonWidget>(_level, RectangleShapeData());
		};
		_info.AddProperty<ButtonWidget, Int>("action", &ButtonWidget::action, 0.f, 2.f);	// 0 nothing, 1 open scene, 2 quit
		_info.SetLastPropertyType(PropertyType::Enum, { "Nothing", "Open scene", "Quit game" });
		_info.AddProperty<ButtonWidget, String>("targetScene", &ButtonWidget::targetScene);
	}

	// ================= Layout and input widgets (ControlWidget.h) =================
	{
		ClassInfo& _info = Describe("ControlWidget", "Widget");
		_info.AddProperty<ControlWidget, FVector2>("size", &ControlWidget::size);	// before the scale
		_info.AddProperty<ControlWidget, Color>("backgroundColor", &ControlWidget::backgroundColor);
		_info.AddProperty<ControlWidget, Color>("outlineColor", &ControlWidget::outlineColor);
		_info.AddProperty<ControlWidget, Float>("outlineThickness", &ControlWidget::outlineThickness, 0.f, 20.f);
		_info.AddAccessor<ControlWidget, int>("zOrder",		// 0 behind, 255 in front
			[](ControlWidget& _w) { return static_cast<int>(_w.GetZOrder()); },
			[](ControlWidget& _w, const int& _v) { _w.SetZOrder(static_cast<uint8_t>(std::clamp(_v, 0, 255))); }, 0.f, 255.f);
	}
	{
		ClassInfo& _info = Describe("PanelWidget", "ControlWidget");
		_info.AddProperty<PanelWidget, Float>("padding", &PanelWidget::padding, 0.f, 500.f);
		_info.AddProperty<PanelWidget, Float>("spacing", &PanelWidget::spacing, 0.f, 500.f);
		_info.AddProperty<PanelWidget, Int>("alignment", &PanelWidget::alignment, 0.f, 2.f);	// across the box
		_info.SetLastPropertyType(PropertyType::Enum, { "Start", "Center", "End" });
		_info.AddProperty<PanelWidget, Bool>("fitContent", &PanelWidget::fitContent);
	}
	Describe("CanvasWidget", "PanelWidget").widgetFactory = [](Level* _level) -> Widget* { return _level->GetHUD().CreateWidget<CanvasWidget>(_level); };
	{
		ClassInfo& _info = Describe("BoxWidget", "PanelWidget");
		_info.AddProperty<BoxWidget, Int>("justify", &BoxWidget::justify, 0.f, 2.f);	// along the box
		_info.SetLastPropertyType(PropertyType::Enum, { "Start", "Center", "End" });
	}
	Describe("HorizontalBoxWidget", "BoxWidget").widgetFactory = [](Level* _level) -> Widget* { return _level->GetHUD().CreateWidget<HorizontalBoxWidget>(_level); };
	Describe("VerticalBoxWidget", "BoxWidget").widgetFactory = [](Level* _level) -> Widget* { return _level->GetHUD().CreateWidget<VerticalBoxWidget>(_level); };
	{
		ClassInfo& _info = Describe("GridWidget", "PanelWidget");
		_info.widgetFactory = [](Level* _level) -> Widget* { return _level->GetHUD().CreateWidget<GridWidget>(_level); };
		_info.AddProperty<GridWidget, Int>("columns", &GridWidget::columns, 1.f, 64.f);
		_info.AddProperty<GridWidget, FVector2>("cellSize", &GridWidget::cellSize);	// 0: the biggest child
	}
	{
		ClassInfo& _info = Describe("ScrollBoxWidget", "PanelWidget");
		_info.widgetFactory = [](Level* _level) -> Widget* { return _level->GetHUD().CreateWidget<ScrollBoxWidget>(_level); };
		_info.AddProperty<ScrollBoxWidget, Float>("scroll", &ScrollBoxWidget::scroll, 0.f, 0.f);
		_info.AddProperty<ScrollBoxWidget, Float>("scrollSpeed", &ScrollBoxWidget::scrollSpeed, 0.f, 500.f);
		_info.AddProperty<ScrollBoxWidget, Bool>("showScrollBar", &ScrollBoxWidget::showScrollBar);
		_info.AddProperty<ScrollBoxWidget, Color>("scrollBarColor", &ScrollBoxWidget::scrollBarColor);
	}
	Describe("SpacerWidget", "ControlWidget").widgetFactory = [](Level* _level) -> Widget* { return _level->GetHUD().CreateWidget<SpacerWidget>(_level); };
	Describe("ImageWidget", "Widget").widgetFactory = [](Level* _level) -> Widget* { return _level->GetHUD().CreateWidget<ImageWidget>(_level, RectangleShapeData()); };
	Describe("TextWidget", "Widget").widgetFactory = [](Level* _level) -> Widget* { return _level->GetHUD().CreateWidget<TextWidget>(_level, TextData("Text")); };
	{
		ClassInfo& _info = Describe("SliderWidget", "ControlWidget");
		_info.widgetFactory = [](Level* _level) -> Widget* { return _level->GetHUD().CreateWidget<SliderWidget>(_level); };
		// The range before the value: a loaded value is clamped to it.
		_info.AddProperty<SliderWidget, Float>("minValue", &SliderWidget::minValue);
		_info.AddProperty<SliderWidget, Float>("maxValue", &SliderWidget::maxValue);
		_info.AddProperty<SliderWidget, Float>("step", &SliderWidget::step, 0.f, 0.f);
		_info.AddAccessor<SliderWidget, float>("value",
			[](SliderWidget& _w) { return static_cast<float>(_w.GetValue()); },
			[](SliderWidget& _w, const float& _v) { _w.SetValue(_v); });
		_info.AddProperty<SliderWidget, Bool>("vertical", &SliderWidget::vertical);
		_info.AddProperty<SliderWidget, Color>("trackColor", &SliderWidget::trackColor);
		_info.AddProperty<SliderWidget, Color>("fillColor", &SliderWidget::fillColor);
		_info.AddProperty<SliderWidget, Color>("handleColor", &SliderWidget::handleColor);
		_info.AddProperty<SliderWidget, Float>("handleSize", &SliderWidget::handleSize, 1.f, 200.f);
		_info.AddProperty<SliderWidget, EventList>("onValueChanged", &SliderWidget::onValueChangedCalls);
	}
	{
		ClassInfo& _info = Describe("ProgressBarWidget", "ControlWidget");
		_info.widgetFactory = [](Level* _level) -> Widget* { return _level->GetHUD().CreateWidget<ProgressBarWidget>(_level); };
		_info.AddAccessor<ProgressBarWidget, float>("value",	// 0 to 1
			[](ProgressBarWidget& _w) { return static_cast<float>(_w.GetValue()); },
			[](ProgressBarWidget& _w, const float& _v) { _w.SetValue(_v); }, 0.f, 1.f);
		_info.AddProperty<ProgressBarWidget, Color>("fillColor", &ProgressBarWidget::fillColor);
		_info.AddProperty<ProgressBarWidget, Int>("direction", &ProgressBarWidget::direction, 0.f, 3.f);
		_info.SetLastPropertyType(PropertyType::Enum, { "Left to right", "Right to left", "Bottom to top", "Top to bottom" });
		_info.AddProperty<ProgressBarWidget, Bool>("showText", &ProgressBarWidget::showText);
		_info.AddProperty<ProgressBarWidget, Color>("textColor", &ProgressBarWidget::textColor);
		_info.AddProperty<ProgressBarWidget, Int>("characterSize", &ProgressBarWidget::characterSize, 1.f, 200.f);
	}
	{
		ClassInfo& _info = Describe("CheckBoxWidget", "ControlWidget");
		_info.widgetFactory = [](Level* _level) -> Widget* { return _level->GetHUD().CreateWidget<CheckBoxWidget>(_level); };
		_info.AddProperty<CheckBoxWidget, String>("text", &CheckBoxWidget::text);
		_info.AddProperty<CheckBoxWidget, String>("group", &CheckBoxWidget::group);	// not empty: a radio button
		_info.AddAccessor<CheckBoxWidget, bool>("checked",
			[](CheckBoxWidget& _w) { return static_cast<bool>(_w.IsChecked()); },
			[](CheckBoxWidget& _w, const bool& _v) { _w.SetChecked(_v); });
		_info.AddProperty<CheckBoxWidget, Color>("boxColor", &CheckBoxWidget::boxColor);
		_info.AddProperty<CheckBoxWidget, Color>("checkColor", &CheckBoxWidget::checkColor);
		_info.AddProperty<CheckBoxWidget, Color>("textColor", &CheckBoxWidget::textColor);
		_info.AddProperty<CheckBoxWidget, Int>("characterSize", &CheckBoxWidget::characterSize, 1.f, 200.f);
		_info.AddProperty<CheckBoxWidget, EventList>("onChecked", &CheckBoxWidget::onCheckedCalls);
		_info.AddProperty<CheckBoxWidget, EventList>("onUnchecked", &CheckBoxWidget::onUncheckedCalls);
	}
	{
		ClassInfo& _info = Describe("TextEntryWidget", "ControlWidget");
		_info.widgetFactory = [](Level* _level) -> Widget* { return _level->GetHUD().CreateWidget<TextEntryWidget>(_level); };
		_info.AddProperty<TextEntryWidget, Int>("maxLength", &TextEntryWidget::maxLength, 0.f, 10000.f);	// before the text: it is cut to it
		_info.AddAccessor<TextEntryWidget, std::string>("text",
			[](TextEntryWidget& _w) { return _w.GetText(); },
			[](TextEntryWidget& _w, const std::string& _v) { _w.SetText(_v); });
		_info.AddProperty<TextEntryWidget, String>("placeholder", &TextEntryWidget::placeholder);
		_info.AddProperty<TextEntryWidget, Bool>("password", &TextEntryWidget::password);
		_info.AddProperty<TextEntryWidget, Int>("characterSize", &TextEntryWidget::characterSize, 1.f, 200.f);
		_info.AddProperty<TextEntryWidget, Color>("textColor", &TextEntryWidget::textColor);
		_info.AddProperty<TextEntryWidget, Color>("placeholderColor", &TextEntryWidget::placeholderColor);
		_info.AddProperty<TextEntryWidget, Color>("focusColor", &TextEntryWidget::focusColor);
		_info.AddProperty<TextEntryWidget, EventList>("onTextChanged", &TextEntryWidget::onTextChangedCalls);
		_info.AddProperty<TextEntryWidget, EventList>("onSubmit", &TextEntryWidget::onSubmitCalls);
	}
	{
		ClassInfo& _info = Describe("DropdownWidget", "ControlWidget");
		_info.widgetFactory = [](Level* _level) -> Widget* { return _level->GetHUD().CreateWidget<DropdownWidget>(_level); };
		_info.AddAccessor<DropdownWidget, std::string>("options",	// separated by ';', before the selection
			[](DropdownWidget& _w) { return _w.GetOptionsText(); },
			[](DropdownWidget& _w, const std::string& _v) { _w.SetOptions(_v); });
		_info.AddAccessor<DropdownWidget, int>("selected",
			[](DropdownWidget& _w) { return static_cast<int>(_w.GetSelected()); },
			[](DropdownWidget& _w, const int& _v) { _w.SetSelected(_v); });
		_info.AddProperty<DropdownWidget, Color>("itemColor", &DropdownWidget::itemColor);
		_info.AddProperty<DropdownWidget, Color>("hoverColor", &DropdownWidget::hoverColor);
		_info.AddProperty<DropdownWidget, Color>("textColor", &DropdownWidget::textColor);
		_info.AddProperty<DropdownWidget, Int>("characterSize", &DropdownWidget::characterSize, 1.f, 200.f);
		_info.AddProperty<DropdownWidget, EventList>("onSelectionChanged", &DropdownWidget::onSelectionChangedCalls);
	}

	// ================= Widget components =================
	{
		ClassInfo& _info = Describe("AnimationWidgetComponent", "WidgetComponent");
		_info.widgetComponentFactory = [](Widget* _owner) -> WidgetComponent*
		{
			if (!_owner->GetComponent<ImageWidgetComponent>())
			{
				LOG_WARNING("AnimationWidgetComponent needs an ImageWidgetComponent on the same widget");
				return nullptr;
			}
			return _owner->AddComponent<AnimationWidgetComponent>();
		};
		AddAnimationProperties<AnimationWidgetComponent>(_info);
	}

	{
		ClassInfo& _info = Describe("ImageWidgetComponent", "WidgetComponent");
		_info.widgetComponentFactory = [](Widget* _owner) -> WidgetComponent* { return _owner->AddComponent<ImageWidgetComponent>(RectangleShapeData()); };
		AddShapeProperties<ImageWidgetComponent>(_info);
	}

	{
		ClassInfo& _info = Describe("LabelWidgetComponent", "WidgetComponent");
		_info.widgetComponentFactory = [](Widget* _owner) -> WidgetComponent* { return _owner->AddComponent<LabelWidgetComponent>(TextData()); };
		AddLabelProperties<LabelWidgetComponent>(_info);
	}

	{
		ClassInfo& _info = Describe("ButtonWidgetComponent", "WidgetComponent");
		_info.AddProperty<ButtonWidgetComponent, EventList>("onPress", &ButtonWidgetComponent::onPressCalls);
		_info.AddProperty<ButtonWidgetComponent, EventList>("onRelease", &ButtonWidgetComponent::onReleaseCalls);
		_info.AddProperty<ButtonWidgetComponent, EventList>("onHover", &ButtonWidgetComponent::onHoverCalls);
		_info.AddProperty<ButtonWidgetComponent, EventList>("onUnhover", &ButtonWidgetComponent::onUnhoverCalls);
		_info.widgetComponentFactory = [](Widget* _owner) -> WidgetComponent*
		{
			if (!_owner->GetComponent<ImageWidgetComponent>())
			{
				LOG_WARNING("ButtonWidgetComponent needs an ImageWidgetComponent on the same widget");
				return nullptr;
			}
			return _owner->AddComponent<ButtonWidgetComponent>();
		};
	}

	}
}
