#pragma once
#include "Core/KrampusObject.h"
#include "Managers/ActorManager.h"
#include "Managers/CameraManager.h"
#include "Managers/WorldPhysics.h"
#include "Managers/TimerManager.h"
#include "Managers/HUD.h"

namespace Krampus
{
	class ParticleComponent;
	class NavigationSystem;
	enum class ParticlePreset;

	class CameraActor;
	class CameraComponent;

	/// <summary>
	/// Represents a game scene or level containing actors, physics simulation, cameras, and UI widgets.
	/// </summary>
	class Level : public KrampusObject
	{	
		Int					frameCount				= 0;

	protected:
		std::unique_ptr<NavigationSystem>	navigation;		// first member: destroyed last, after the actors that use it
		TimerManager		timerManager;
		CameraManager		cameraManager;
		WorldPhysics		worldPhysics;
		ActorManager		actorManager;
		HUD					hud;
		Color				backgroundColor			= Color::Black();
		Bool				isLoaded				= false;
		Bool				hasBegunPlay			= false;

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

		/// <returns>The color the window is cleared with before the level is drawn.</returns>
		inline const Color& GetBackgroundColor() const
		{
			return backgroundColor;
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

		/// <summary>The navigation of the level: meshes of its NavMesh Bounds Volumes, paths, agents (made the first time it is asked).</summary>
		NavigationSystem& GetNavigation();

		virtual ~Level();

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
		/// A particle effect at a place of the world, in an actor of its own: SpawnParticles(ParticlePreset::Explosion, _hitPosition).
		/// A one-shot effect (not looping) removes its actor once its last particle is gone.
		/// </summary>
		ParticleComponent* SpawnParticles(const ParticlePreset& _preset, const FVector2& _position);

		/// <summary>
		/// Spawns a diegetic widget kept at _screenPosition (window pixels): SpawnDiegeticWidget&lt;DiegeticLabelWidget&gt;(FVector2(40, 40), TextData("Score")).
		/// </summary>
		template <typename Type, typename ...Args, IS_BASE_OF(DiegeticWidget, Type)>
		inline Type* SpawnDiegeticWidget(const FVector2& _screenPosition, Args&&... _args)
		{
			Type* _widget = actorManager.CreateActor<Type>(this, std::forward<Args>(_args)...);
			_widget->SetScreenPosition(_screenPosition);
			return _widget;
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
		/// Starts the game in this level: BeginPlay of every actor and widget, once. Called when the level loads and the game runs;
		/// in the editor (level paused) it waits for Play, so placing or spawning an actor only runs its constructor and Construct.
		/// </summary>
		void StartPlay();

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
		/// Gets the camera the level is seen through: the current one, else the first CameraComponent of any actor.
		/// A level that starts without any camera gets one (a CameraActor at the center of the window); it is an actor like another and can be deleted.
		/// </summary>
		/// <returns>Pointer to a CameraComponent, or nullptr if the level has none.</returns>
		CameraComponent* GetDefaultCamera() const;
	};

}
