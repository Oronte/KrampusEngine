#pragma once
#include "Graphics/Window/Window.h"
#include "Managers/ActorManager.h"
#include "Managers/CameraManager.h"

namespace Krampus
{

	class Level
	{
		std::string name = "Level";
		//Window window;
		CameraManager cameraManager;
		ActorManager actorManager;
		Color backgroundColor = Color::Black();
		bool isLoaded = false;

	public:
		Level(const std::string& _name);

		FORCEINLINE void SetBackgroundColor(const Color& _bgColor)
		{
			backgroundColor = _bgColor;
		}
		FORCEINLINE std::string GetName() const
		{
			return name;
		}
		FORCEINLINE CameraManager& GetCameraManager()
		{
			return cameraManager;
		}
		FORCEINLINE ActorManager& GetActorManager()
		{
			return actorManager;
		}

	protected:
		virtual void InitLevel();

	public:
		virtual void Load();
		virtual void Unload();
		void Update(const float& _deltaTime);
	};

}