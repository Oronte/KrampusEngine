#pragma once
#include "Core/KrampusObject.h"
#include "Managers/ActorManager.h"
#include "Managers/CameraManager.h"
#include "Managers/WorldPhysics.h"
#include "Managers/TimerManager.h"
#include "Managers/HUD.h"

namespace Krampus
{

	class CameraActor;
	class CameraComponent;

	/// <summary>
	/// Represents a game scene or level containing actors, physics simulation, cameras, and UI widgets.
	/// </summary>
	class Level : public KrampusObject
	{	
		Int					frameCount				= 0;

	protected:
		TimerManager		timerManager;
		CameraManager		cameraManager;
		WorldPhysics		worldPhysics;
		ActorManager		actorManager;
		HUD					hud;
		Color				backgroundColor			= Color::Black();
		Bool				isLoaded				= false;

		CameraActor*		mainCamera				= nullptr;

	protected:
		/// <summary>
		/// Constructs a Level associated with the specified Engine instance.
		/// </summary>
		/// <param name="_engine">Pointer to the Engine world.</param>
		Level(Engine* _engine);

	public:
		/// <summary>
		/// Sets the background clear color of the level.
		/// </summary>
		/// <param name="_bgColor">The color used to clear the window before rendering.</param>
		inline void SetBackgroundColor(const Color& _bgColor)
		{
			backgroundColor = _bgColor;
		}

		/// <summary>
		/// Gets a reference to the level's TimerManager.
		/// </summary>
		/// <returns>Reference to TimerManager.</returns>
		inline TimerManager& GetTimerManager()
		{
			return timerManager;
		}

		/// <summary>
		/// Gets a reference to the level's CameraManager.
		/// </summary>
		/// <returns>Reference to CameraManager.</returns>
		inline CameraManager& GetCameraManager()
		{
			return cameraManager;
		}

		/// <summary>
		/// Gets a reference to the level's CollisionManager.
		/// </summary>
		/// <returns>Reference to CollisionManager.</returns>
		inline WorldPhysics& GetWorldPhysics()
		{
			return worldPhysics;
		}

		/// <summary>
		/// Gets a reference to the level's ActorManager.
		/// </summary>
		/// <returns>Reference to ActorManager.</returns>
		inline ActorManager& GetActorManager()
		{
			return actorManager;
		}

		/// <summary>
		/// Gets a reference to the level's heads-up display (HUD).
		/// </summary>
		/// <returns>Reference to HUD.</returns>
		inline HUD& GetHUD()
		{
			return hud;
		}

		/// <summary>
		/// Gets a reference to the engine window.
		/// </summary>
		/// <returns>Reference to Window.</returns>
		inline Window& GetWindow()
		{
			return GetWorld()->GetWindowRef();
		}


	protected:
		/// <summary>
		/// Hook for initializing level-specific content and spawning initial actors.
		/// </summary>
		virtual void InitLevel();

	public:
		/// <summary>
		/// Spawns a new actor of the specified type in the level.
		/// </summary>
		/// <typeparam name="Type">Actor class derived from Actor.</typeparam>
		/// <param name="_args">Arguments forwarded to the actor constructor.</param>
		/// <returns>Pointer to the newly spawned actor.</returns>
		template <typename Type, typename ...Args, IS_BASE_OF(Actor, Type)>
		inline Type* SpawnActor(Args&&... _args)
		{
			return actorManager.CreateActor<Type>(this, std::forward<Args>(_args)...);
		}

		/// <summary>
		/// Spawns a new actor of the specified type with an initial transform.
		/// </summary>
		/// <typeparam name="Type">Actor class derived from Actor.</typeparam>
		/// <param name="_transform">The initial transform (position, rotation, scale).</param>
		/// <param name="_args">Arguments forwarded to the actor constructor.</param>
		/// <returns>Pointer to the newly spawned actor.</returns>
		template <typename Type, typename ...Args, IS_BASE_OF(Actor, Type)>
		inline Type* SpawnActorAt(const Transform& _transform, Args&&... _args)
		{
			return actorManager.CreateActorAt<Type>(_transform, this, std::forward<Args>(_args)...);
		}

		/// <summary>
		/// Spawns a HUD widget of the specified type.
		/// </summary>
		/// <typeparam name="Type">Widget class derived from Widget.</typeparam>
		/// <param name="_args">Arguments forwarded to the widget constructor.</param>
		/// <returns>Pointer to the newly created widget.</returns>
		template <typename Type, typename ...Args, IS_BASE_OF(Widget, Type)>
		inline Type* SpawnWidget(Args&&... _args)
		{
			return hud.CreateWidget<Type>(this, std::forward<Args>(_args)...);
		}

		/// <summary>
		/// Loads the level, initializing physics, cameras, actors, and UI.
		/// </summary>
		virtual void Load();

		/// <summary>
		/// Unloads the level, clearing actors, UI widgets, and releasing resources.
		/// </summary>
		virtual void Unload();

		/// <summary>
		/// Updates the level simulation, physics, managers, and actors for the frame.
		/// </summary>
		/// <param name="_deltaTime">Time elapsed since the previous frame in seconds.</param>
		void Update(const Float& _deltaTime);

		/// <summary>
		/// Clears the window and draws actors, debug shapes and widgets. Does not present the frame.
		/// </summary>
		void Render();

		/// <summary>
		/// Updates the internal frame timer and returns elapsed delta time.
		/// </summary>
		/// <returns>Delta time as Float.</returns>
		Float UpdateTime();

		/// <summary>
		/// Gets the default active camera component in this level.
		/// </summary>
		/// <returns>Pointer to the default CameraComponent.</returns>
		CameraComponent* GetDefaultCamera() const;
	};

}
