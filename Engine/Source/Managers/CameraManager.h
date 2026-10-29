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
		std::optional<sf::View>						viewOverride;	// when set, the world is drawn through this view instead of the current camera
		std::map<uint8_t, std::vector<IDrawable*>>	actors;
		std::map<uint8_t, std::vector<IDrawable*>>	widgets;
		std::vector<std::shared_ptr<Vertices>>		debugs;			// outlines drawn in the world, through the camera
		std::vector<std::shared_ptr<Vertices>>		screenDebugs;	// outlines of HUD widgets, drawn in the HUD view
		bool										drawingScreenDebug	= false;

	public:
		/// <summary>
		/// Gets the camera the level is drawn with (nullptr until the first render).
		/// </summary>
		inline CameraComponent* GetCurrent() const
		{
			return current;
		}

		/// <summary>
		/// Adds an outline to what is drawn this frame: in the HUD view while the debug outlines of widgets are drawn, in the world otherwise.
		/// </summary>
		inline void PushDebug(const std::shared_ptr<Vertices>& _vertices)
		{
			(drawingScreenDebug ? screenDebugs : debugs).push_back(_vertices);
		}

		/// <summary>
		/// Draws the outline of every component that uses debug (IDebuggable): of the actors in the world (_screen false), of the HUD widgets (_screen true).
		/// </summary>
		void RenderDebuggables(Window& _window, const Bool& _screen);

		/// <summary>
		/// Draws the world through this view, whatever the cameras of the level are (the editor's free camera). Call it every frame it must hold.
		/// </summary>
		inline void SetViewOverride(const sf::View& _view)
		{
			viewOverride = _view;
		}

		/// <summary>
		/// Goes back to the cameras of the level.
		/// </summary>
		inline void ClearViewOverride()
		{
			viewOverride.reset();
		}

		/// <summary>
		/// Sets the currently active rendering camera.
		/// </summary>
		/// <param name="_camera">Pointer to CameraComponent.</param>
		inline void SetCurrent(CameraComponent* _camera)
		{
			LOG(VerbosityType::Verbose, std::string("Camera: current camera changed to ") + (_camera && _camera->GetOwner() ? _camera->GetOwner()->GetName().StdString() : std::string("none")));
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
			// Construct runs twice for the components created in an actor's constructor (CreateComponent, then the actor's Construct):
			// a second entry would draw the sprite twice.
			std::vector<IDrawable*>& _layer = actors[_drawable->GetZOrder()];
			if (std::find(_layer.begin(), _layer.end(), _drawable) == _layer.end())
				_layer.push_back(_drawable);
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
			std::vector<IDrawable*>& _layer = widgets[_drawable->GetZOrder()];
			if (std::find(_layer.begin(), _layer.end(), _drawable) == _layer.end())
				_layer.push_back(_drawable);
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
