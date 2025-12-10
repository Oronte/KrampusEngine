#pragma once
#include "Components/CameraComponent.h"
#include "Components/SpriteComponent.h"
#include "Actors/Actor.h"

namespace Krampus
{

	class CameraManager
	{
		CameraComponent* current;
		std::map<int, std::vector<IDrawable*>> drawer;

	public:
		FORCEINLINE void SetCurrent(CameraComponent* _camera)
		{
			if (!_camera) return;
			current = _camera;
		}
		FORCEINLINE void AddToWindow(IDrawable* _drawable)
		{
			if (!_drawable)
			{
				LOG(VerbosityType::Error, "You try to add to window an invalide sprite");
				return;
			}
			drawer[_drawable->zOrder].push_back(_drawable);
		}
		FORCEINLINE void RemoveToWindow(IDrawable* _drawable)
		{
			if (!_drawable)
			{
				LOG(VerbosityType::Error, "You try to remove to window an nullptr sprite");
				return;
			} // TODO fix
			//drawer.erase(std::remove(drawer.begin(), drawer.end(), _sprite), drawer.end());
		}

		CameraManager() = default;

	public:
		void Render(Window& _window, const bool& splitScreen);
	};

}

