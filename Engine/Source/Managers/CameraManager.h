#pragma once
#include "Components/CameraComponent.h"
#include "Components/SpriteComponent.h"
#include "Actors/Actor.h"
#include "Core/KrampusObject.h"

namespace Krampus
{

	class CameraManager : public KrampusObject
	{
		CameraComponent* current = nullptr;
		std::map<uint8_t, std::vector<IDrawable*>> drawer;
		std::vector<std::shared_ptr<Vertices>> debugs;

	public:
		inline void SetCurrent(CameraComponent* _camera)
		{
			if (!_camera) return;
			current = _camera;
		}
		inline void AddToWindow(IDrawable* _drawable)
		{
			if (!_drawable)
			{
				LOG(VerbosityType::Error, "You try to add to window an invalide sprite");
				return;
			}
			drawer[_drawable->GetZOrder()].push_back(_drawable);
		}
		inline void RemoveToWindow(IDrawable* _drawable)
		{
			if (!_drawable)
			{
				LOG(VerbosityType::Error, "You try to remove to window an nullptr sprite");
				return;
			}
			std::erase_if(drawer[_drawable->GetZOrder()], [&](const IDrawable* _item)
				{
					return _item == _drawable;
				});
		}

		inline void ChangeZOrder(IDrawable* _drawable, const uint8_t& _oldZOrder)
		{
			std::erase_if(drawer[_oldZOrder], [&](const IDrawable* _item)
				{
					return _drawable == _item;
				});

			AddToWindow(_drawable);
		}

		CameraManager(Engine* _engine)
			: KrampusObject(_engine) { }

	public:
		void UpdateView();
		void Render(const bool& splitScreen);

		friend class Debug;
	};

}

