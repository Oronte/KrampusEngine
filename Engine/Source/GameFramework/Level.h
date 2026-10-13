#pragma once
#include "Core/KrampusObject.h"
#include "Managers/ActorManager.h"
#include "Managers/CameraManager.h"
#include "Managers/CollisionManager.h"
#include "Managers/TimerManager.h"
#include "Managers/HUD.h"
#include "box2d/box2d.h"

namespace Krampus
{

	class CameraActor;
	class CameraComponent;

	class Level : public KrampusObject
	{	
		Int					frameCount				= 0;
		b2WorldDef			physicsWorld;
		b2WorldId			physicsWorldID;

	protected:
		TimerManager		timerManager;
		CameraManager		cameraManager;
		CollisionManager	collisionManager;
		ActorManager		actorManager;
		HUD					hud;
		Color				backgroundColor			= Color::Black();
		Bool				isLoaded				= false;

		CameraActor*		mainCamera				= nullptr;

	protected:
		Level(Engine* _engine);

	public:
		inline void SetBackgroundColor(const Color& _bgColor)
		{
			backgroundColor = _bgColor;
		}
		inline TimerManager& GetTimerManagerRef()
		{
			return timerManager;
		}
		inline CameraManager& GetCameraManagerRef()
		{
			return cameraManager;
		}
		inline CollisionManager& GetCollisionManagerRef()
		{
			return collisionManager;
		}
		inline ActorManager& GetActorManagerRef()
		{
			return actorManager;
		}
		inline HUD& GetHUDRef()
		{
			return hud;
		}
		inline Window& GetWindowRef()
		{
			return GetWorld()->GetWindowRef();
		}
		inline b2WorldDef& GetPhysicsWorldRef()
		{
			return physicsWorld;
		}
		inline b2WorldId GetPhysicsWorldID()
		{
			return physicsWorldID;
		}

	protected:
		virtual void InitLevel();

	public:
		template <typename Type, typename ...Args, IS_BASE_OF(Actor, Type)>
		inline Type* SpawnActor(Args&&... _args)
		{
			return actorManager.CreateActor<Type>(this, std::forward<Args>(_args)...);
		}
		template <typename Type, typename ...Args, IS_BASE_OF(Actor, Type)>
		inline Type* SpawnActorAt(const Transform& _transform, Args&&... _args)
		{
			return actorManager.CreateActorAt<Type>(_transform, this, std::forward<Args>(_args)...);
		}
		template <typename Type, typename ...Args, IS_BASE_OF(Widget, Type)>
		inline Type* SpawnWidget(Args&&... _args)
		{
			return hud.CreateWidget<Type>(this, std::forward<Args>(_args)...);
		}
		virtual void Load();
		virtual void Unload();
		void Update(const Float& _deltaTime);
		Float UpdateTime();
		CameraComponent* GetDefaultCamera() const;
	};

}