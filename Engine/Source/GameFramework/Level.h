#pragma once
#include "Graphics/Window/Window.h"
#include "Managers/ActorManager.h"
#include "Managers/CameraManager.h"
#include "Managers/HUD.h"

namespace Krampus
{

	class Level
	{
	protected:
		CameraManager cameraManager;
		ActorManager actorManager;
		HUD hud;
		Color backgroundColor = Color::Black();
		bool isLoaded = false;

	public:
		Level() = default;

		INLINE void SetBackgroundColor(const Color& _bgColor)
		{
			backgroundColor = _bgColor;
		}
		INLINE CameraManager& GetCameraManagerRef()
		{
			return cameraManager;
		}
		INLINE ActorManager& GetActorManagerRef()
		{
			return actorManager;
		}
		INLINE HUD& GetHUDRef()
		{
			return hud;
		}

	protected:
		virtual void InitLevel() {}

	public:
		template <typename Type, typename ...Args, IS_BASE_OF(Actor, Type), IS_NOT_BASE_OF(Widget, Type)>
		INLINE Type* SpawnActor(Args&&... _args)
		{
			return actorManager.CreateActor<Type>(this, std::forward<Args>(_args)...);
		}
		template <typename Type, typename ...Args, IS_BASE_OF(Widget, Type)>
		INLINE Type* SpawnWidget(Args&&... _args)
		{
			return hud.CreateWidget<Type>(this, std::forward<Args>(_args)...);
		}
		virtual void Load();
		virtual void Unload();
		void Update(const float& _deltaTime);

	private:
		void Render();
	};

}