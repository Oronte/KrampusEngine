#pragma once
#include "Components/PhysicsComponent.h"
#include "Actors/Actor.h"

namespace Krampus::Exemple
{
    class PhysicsActorExemple : public Actor
    {
        PhysicsComponent* component = nullptr;

    public:
        PhysicsActorExemple(Level* _level);

        void SetDynamic();
        void SetStatic();
    };
}
