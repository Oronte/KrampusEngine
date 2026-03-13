#pragma once
#include "Core/KrampusObject.h"
#include "Managers/ActorManager.h"
#include "Managers/CameraManager.h"
#include "Managers/CollisionManager.h"
#include "Managers/HUD.h"

namespace Krampus
{

	class CameraActor;
	class CameraComponent;

	class Level : public KrampusObject
	{	
		Int					frameCount				= 0;

	protected:
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

	protected:
		virtual void InitLevel();

	public:
		template <typename Type, typename ...Args, IS_BASE_OF(Actor, Type), IS_NOT_BASE_OF(Widget, Type)>
		inline Type* SpawnActor(Args&&... _args)
		{
			return actorManager.CreateActor<Type>(this, std::forward<Args>(_args)...);
		}
		template <typename Type, typename ...Args, IS_BASE_OF(Actor, Type), IS_NOT_BASE_OF(Widget, Type)>
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
		CameraComponent* GetDefaultCamera() const;

	private:
		void Render();
	};

}