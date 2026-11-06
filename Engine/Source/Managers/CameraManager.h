#pragma once
#include "Components/CameraComponent.h"
#include "Components/SpriteComponent.h"
#include "Actors/Actor.h"
#include "Core/KrampusObject.h"
#include "Graphics/Lighting/LightRenderer.h"

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
		std::vector<sf::Vertex>						debugTriangles;	// lines and points made thick, drawn at once (reused)
		LightRenderer								lights;			// 2D lights over the world (a level without lights is not touched)
		std::vector<CameraComponent*>				splitCameras;	// split screen: each one draws the world in its part of the window
		bool										drawingScreenDebug	= false;

	public:
		/// <summary>
		/// Gets the camera the level is drawn with (nullptr until the first render).
		/// </summary>
		CameraComponent* GetCurrent() const
		{
			return current;
		}

		/// <summary>
		/// Adds an outline to what is drawn this frame: in the HUD view while the debug outlines of widgets are drawn, in the world otherwise.
		/// </summary>
		void PushDebug(const std::shared_ptr<Vertices>& _vertices)
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
		void SetViewOverride(const sf::View& _view)
		{
			viewOverride = _view;
		}

		/// <summary>
		/// Goes back to the cameras of the level.
		/// </summary>
		void ClearViewOverride()
		{
			viewOverride.reset();
		}

		/// <summary>
		/// Sets the currently active rendering camera.
		/// </summary>
		/// <param name="_camera">Pointer to CameraComponent.</param>
		/// <summary>
		/// Split screen: the world is drawn by each camera in its part of the window, the HUD once over the whole window.
		/// 2 cameras: side by side (one over the other when _stacked); 3: two at the top, one at the bottom; 4: a 2 x 2 grid.
		/// Each camera keeps its zoom. An empty list (or ClearSplitScreen) goes back to the current camera alone.
		/// </summary>
		void SetSplitScreen(const std::vector<CameraComponent*>& _cameras, const bool& _stacked = false);

		/// <summary>Back to one camera on the whole window.</summary>
		void ClearSplitScreen();

		bool IsSplitScreen() const
		{
			return splitCameras.size() > 1;
		}

		const std::vector<CameraComponent*>& GetSplitCameras() const
		{
			return splitCameras;
		}

		/// <summary>A camera that is destroyed leaves the split screen (the others share the window again).</summary>
		void RemoveSplitCamera(CameraComponent* _camera);

		void SetCurrent(CameraComponent* _camera)
		{
			LOG(VerbosityType::Verbose, std::string("Camera: current camera changed to ") + (_camera && _camera->GetOwner() ? _camera->GetOwner()->GetName() : std::string("none")));
			current = _camera;
		}

		/// <summary>
		/// Adds a drawable object into the world rendering layer sorted by its Z-order.
		/// </summary>
		/// <param name="_drawable">Pointer to IDrawable.</param>
		void AddToWorld(IDrawable* _drawable)
		{
			if (!_drawable)
			{
				LOG(VerbosityType::Error, "You try to add to window an invalide sprite");
				return;
			}
			// Never twice in the list: a second entry would draw the sprite twice.
			std::vector<IDrawable*>& _layer = actors[_drawable->GetZOrder()];
			if (std::find(_layer.begin(), _layer.end(), _drawable) == _layer.end())
				_layer.push_back(_drawable);
		}

		/// <summary>
		/// Adds a drawable object into the UI screen rendering layer sorted by its Z-order.
		/// </summary>
		/// <param name="_drawable">Pointer to IDrawable.</param>
		void AddToScreen(IDrawable* _drawable)
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
		void RemoveToWorld(IDrawable* _drawable)
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
		void RemoveToScreen(IDrawable* _drawable)
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
		void ChangeWorldZOrder(IDrawable* _drawable, const uint8_t& _oldZOrder)
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
		void ChangeScreenZOrder(IDrawable* _drawable, const uint8_t& _oldZOrder)
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
		void RenderPostProcess(Window& _window);
		sf::Texture									screenCopy;		// the picture of the world, for the post-process effects
		void RenderDebugs(Window& _window, const Bool& splitScreen);
		void DrawDebugList(Window& _window, const std::vector<std::shared_ptr<Vertices>>& _list);
		void RenderWidgets(Window& _window, const Bool& splitScreen);

		friend class Debug;
	};

}
