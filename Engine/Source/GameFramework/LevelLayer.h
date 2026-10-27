#pragma once
#include "Core/Layer.h"
#include "Managers/LevelManager.h"

namespace Krampus
{

	/// <summary>
	/// Layer that runs and draws the level currently held by the LevelManager.
	/// </summary>
	class LevelLayer : public Layer
	{
		Engine*		world	= nullptr;

	public:
		LevelLayer(Engine* _engine)
			: world(_engine) { }

		void OnUpdate(const Float& _deltaTime) override
		{
			if (Level* _level = world->GetLevelManager()->GetCurrentLevel())
				_level->Update(_deltaTime);
		}

		void OnRender(Window& _window) override
		{
			if (Level* _level = world->GetLevelManager()->GetCurrentLevel())
				_level->Render();
		}
	};
}
