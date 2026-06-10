#pragma once
#include "GameFramework/Level.h"

namespace Krampus::Exemple
{

	class CollisionLevel : public Level
	{
		std::vector<Actor*> rbs;
		Event<>::ListenerHandle onResetHandle;

	public:
		CollisionLevel(Engine* _world);

		virtual void InitLevel() override;

	private:
		void ResetSimulation();
	};

}