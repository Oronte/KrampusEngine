#pragma once
#include "Actor.h"
#include "Components/SoftBodyComponent.h"

namespace Krampus
{

	class SoftBodyActor : public Actor
	{
		SoftBodyComponent* softBody;

	public:
		INLINE SoftBodyComponent* GetSoftBody() const
		{
			return softBody;
		}

		// Construit une membrane pleine (grid) centrée sur la position de l'acteur.
		// gridW/gridH : nombre de points horizontal/vertical.
		SoftBodyActor(Level* _level, const FVector2& _size,
			int gridW = 8, int gridH = 6,
			float kstruct = 900.f, float kshear = 700.f);

		// Construit un anneau (ring) avec pointCount particules et rayon.
		SoftBodyActor(Level* _level, const float& _radius,
			int pointCount = 24,
			float kstruct = 1200.f, float kshear = 800.f, float kradial = 700.f);
	};

}
