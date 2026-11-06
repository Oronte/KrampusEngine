#include "SpriteComponent.h"
#include "StainComponent.h"

void Krampus::SpriteComponent::Draw(Window& _window)
{
	BasicImage::Draw(_window);
	if (isActive && stainLayer) stainLayer->DrawOver(_window, *shape->GetShape()->Get());	// right after: the next sprites cover the marks too
}
