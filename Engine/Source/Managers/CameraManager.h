#pragma once
#include "Components/CameraComponent.h"
#include "Components/SpriteComponent.h"
#include "Actors/Actor.h"
#include "Core/KrampusObject.h"

namespace Krampus
{

	/// <summary>
	/// Manages the active camera, viewport rendering order (Z-order) for actors and UI widgets, and debug overlays.
	/// </summary>
	class CameraManager : public KrampusObject
	{
		CameraComponent*							current		= nullptr;
		std::map<uint8_t, std::vector<IDrawable*>>	actors;
		std::map<uint8_t, std::vector<IDrawable*>>	widgets;
		std::vector<std::shared_ptr<Vertices>>		debugs;

	public:
		/// <summary>
		/// Sets the currently active rendering camera.
		/// </summary>
		/// <param name="_camera">Pointer to CameraComponent.</param>
		inline void SetCurrent(CameraComponent* _camera)
		{
			current = _camera;
		}

		/// <summary>
		/// Adds a drawable object into the world rendering layer sorted by its Z-order.
		/// </summary>
		/// <param name="_drawable">Pointer to IDrawable.</param>
		inline void AddToWorld(IDrawable* _drawable)
		{
			if (!_drawable)
			{
				LOG(VerbosityType::Error, "You try to add to window an invalide sprite");
				return;
			}
			actors[_drawable->GetZOrder()].push_back(_drawable);
		}

		/// <summary>
		/// Adds a drawable object into the UI screen rendering layer sorted by its Z-order.
		/// </summary>
		/// <param name="_drawable">Pointer to IDrawable.</param>
		inline void AddToScreen(IDrawable* _drawable)
		{
			if (!_drawable)
			{
				LOG(VerbosityType::Error, "You try to add to window an invalide sprite");
				return;
			}
			widgets[_drawable->GetZOrder()].push_back(_drawable);
		}

		/// <summary>
		/// Removes a drawable object from the world rendering layer.
		/// </summary>
		/// <param name="_drawable">Pointer to IDrawable.</param>
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

		/// <summary>
		/// Removes a drawable object from the UI screen rendering layer.
		/// </summary>
		/// <param name="_drawable">Pointer to IDrawable.</param>
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

		/// <summary>
		/// Re-sorts a world drawable after its Z-order layer has changed.
		/// </summary>
		/// <param name="_drawable">Pointer to IDrawable.</param>
		/// <param name="_oldZOrder">Previous Z-order layer value.</param>
		inline void ChangeWorldZOrder(IDrawable* _drawable, const uint8_t& _oldZOrder)
		{
			std::erase_if(actors[_oldZOrder], [&](const IDrawable* _item)
				{
					return _drawable == _item;
				});

			AddToWorld(_drawable);
		}

		/// <summary>
		/// Re-sorts a screen drawable after its Z-order layer has changed.
		/// </summary>
		/// <param name="_drawable">Pointer to IDrawable.</param>
		/// <param name="_oldZOrder">Previous Z-order layer value.</param>
		inline void ChangeScreenZOrder(IDrawable* _drawable, const uint8_t& _oldZOrder)
		{
			std::erase_if(widgets[_oldZOrder], [&](const IDrawable* _item)
				{
					return _drawable == _item;
				});

			AddToScreen(_drawable);
		}

		/// <summary>
		/// Constructs a CameraManager associated with the engine world.
		/// </summary>
		/// <param name="_engine">Pointer to Engine world.</param>
		CameraManager(Engine* _engine)
			: KrampusObject(_engine) { }

	public:
		/// <summary>
		/// Updates the window rendering view to match the active camera.
		/// </summary>
		void UpdateView();

		/// <summary>
		/// Renders all world actors, debug shapes, and HUD widgets into the window.
		/// </summary>
		/// <param name="splitScreen">Whether split-screen multi-view rendering is active.</param>
		void Render(const Bool& splitScreen);

	private:
		void RenderActors(Window& _window, const Bool& splitScreen);
		void RenderDebugs(Window& _window, const Bool& splitScreen);
		void RenderWidgets(Window& _window, const Bool& splitScreen);

		friend class Debug;
	};

}
