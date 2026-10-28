#pragma once
#include "Reflection/Reflection.h"

namespace Krampus
{

	class Test : public Actor
	{
		KACTOR(Test, Actor)

	public:
		Test(Level* _level);

		// Shown in the Inspector and saved in scenes.
		KPROPERTY(Float, speed) = 100.f;

		void BeginPlay() override;
		void Tick(const Float& _deltaTime) override;
	};

}
