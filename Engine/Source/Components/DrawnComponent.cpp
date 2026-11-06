#include "DrawnComponent.h"
#include "GameFramework/Level.h"
#include "Managers/CameraManager.h"

namespace
{
	template <typename Base>
	constexpr bool IN_SCREEN = std::is_same_v<Base, Krampus::WidgetComponent>;
}

template <typename Base>
void Krampus::DrawnComponent<Base>::Construct()
{
	Base::Construct();
	CameraManager& _camera = this->GetLevel()->GetCameraManager();
	if constexpr (IN_SCREEN<Base>) _camera.AddToScreen(this);
	else						   _camera.AddToWorld(this);
	drawn = true;
}

template <typename Base>
void Krampus::DrawnComponent<Base>::Deconstruct()
{
	Base::Deconstruct();
	CameraManager& _camera = this->GetLevel()->GetCameraManager();
	if constexpr (IN_SCREEN<Base>) _camera.RemoveToScreen(this);
	else						   _camera.RemoveToWorld(this);
	drawn = false;
}

template <typename Base>
void Krampus::DrawnComponent<Base>::SetZOrder(const uint8_t& _zOrder)
{
	const uint8_t _oldZOrder = GetZOrder();
	IDrawable::SetZOrder(_zOrder);
	if (!drawn) return;	// not in the camera yet: Construct puts it at the right layer

	CameraManager& _camera = this->GetLevel()->GetCameraManager();
	if constexpr (IN_SCREEN<Base>) _camera.ChangeScreenZOrder(this, _oldZOrder);
	else						   _camera.ChangeWorldZOrder(this, _oldZOrder);
}

template class Krampus::DrawnComponent<Krampus::Component>;
template class Krampus::DrawnComponent<Krampus::WidgetComponent>;
