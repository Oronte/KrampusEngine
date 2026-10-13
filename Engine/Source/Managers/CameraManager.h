#pragma once
#include "Components/CameraComponent.h"
#include "Components/SpriteComponent.h"
#include "Actors/Actor.h"
#include "Core/KrampusObject.h"

namespace Krampus
{

	class CameraManager : public KrampusObject
	{
		CameraComponent*							current		= nullptr;
		std::map<uint8_t, std::vector<IDrawable*>>	actors;
		std::map<uint8_t, std::vector<IDrawable*>>	widgets;
		std::vector<std::shared_ptr<Vertices>>		debugs;

	public:
		inline void SetCurrent(CameraComponent* _camera)
		{
			current = _camera;
		}
		inline void AddToWorld(IDrawable* _drawable)
		{
			if (!_drawable)
			{
				LOG(VerbosityType::Error, "You try to add to window an invalide sprite");
				return;
			}
			actors[_drawable->GetZOrder()].push_back(_drawable);
		}
		inline void AddToScreen(IDrawable* _drawable)
		{
			if (!_drawable)
			{
				LOG(VerbosityType::Error, "You try to add to window an invalide sprite");
				return;
			}
			widgets[_drawable->GetZOrder()].push_back(_drawable);
		}
		inline void RemoveToWorld(IDrawable* _drawable)
		{
			if (!_drawable)
			{
				LOG(VerbosityType::Error, "You try to remove to window an nullptr sprite");
				return;
			}
			std::erase_if(actors[_drawable->GetZOrder()], [&](const IDrawable* _item)
				{
					return _item == _drawable;
				});
		}
		inline void RemoveToScreen(IDrawable* _drawable)
		{
			if (!_drawable)
			{
				LOG(VerbosityType::Error, "You try to remove to window an nullptr sprite");
				return;
			}
			std::erase_if(widgets[_drawable->GetZOrder()], [&](const IDrawable* _item)
				{
					return _item == _drawable;
				});
		}

		inline void ChangeWorldZOrder(IDrawable* _drawable, const uint8_t& _oldZOrder)
		{
			std::erase_if(actors[_oldZOrder], [&](const IDrawable* _item)
				{
					return _drawable == _item;
				});

			AddToWorld(_drawable);
		}
		inline void ChangeScreenZOrder(IDrawable* _drawable, const uint8_t& _oldZOrder)
		{
			std::erase_if(widgets[_oldZOrder], [&](const IDrawable* _item)
				{
					return _drawable == _item;
				});

			AddToScreen(_drawable);
		}

		CameraManager(Engine* _engine)
			: KrampusObject(_engine) { }

	public:
		void UpdateView();
		void Render(const Bool& splitScreen);

	private:
		void RenderActors(Window& _window, const Bool& splitScreen);
		void RenderDebugs(Window& _window, const Bool& splitScreen);
		void RenderWidgets(Window& _window, const Bool& splitScreen);

		friend class Debug;
	};

}

