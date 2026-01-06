#pragma once
#include "Components/CameraComponent.h"
#include "Components/SpriteComponent.h"
#include "Actors/Actor.h"

namespace Krampus
{

	class CameraManager
	{
		CameraComponent* current = nullptr;
		std::map<uint8_t, std::vector<IDrawable*>> drawer;

	public:
		INLINE void SetCurrent(CameraComponent* _camera)
		{
			if (!_camera) return;
			current = _camera;
		}
		INLINE void AddToWindow(IDrawable* _drawable)
		{
			if (!_drawable)
			{
				LOG(VerbosityType::Error, "You try to add to window an invalide sprite");
				return;
			}
			drawer[_drawable->GetZOrder()].push_back(_drawable);
		}
		INLINE void RemoveToWindow(IDrawable* _drawable)
		{
			if (!_drawable)
			{
				LOG(VerbosityType::Error, "You try to remove to window an nullptr sprite");
				return;
			} // TODO fix
			//drawer.erase(std::remove(drawer.begin(), drawer.end(), _drawable), drawer.end());
		}

		INLINE void RemoveDrawable(IDrawable* _toDelete)
		{
			std::erase_if(drawer[_toDelete->GetZOrder()], [&](const IDrawable* _drawable)
				{
					return _drawable == _toDelete;
				});
		}

		INLINE void ChangeZOrder(IDrawable* _drawable, const uint8_t& _oldZOrder)
		{
			std::erase_if(drawer[_oldZOrder], [&](const IDrawable* _item)
				{
					return _drawable == _item;
				});

			AddToWindow(_drawable);
		}

		CameraManager() = default;

	public:
		void UpdateView();
		void Render(Window& _window, const bool& splitScreen);
	};

}

