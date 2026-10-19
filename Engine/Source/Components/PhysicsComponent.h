#pragma once
#include "Component.h"
#include "box2d/box2d.h"

namespace Krampus
{
    class PhysicsComponent : public Component
    {
        b2BodyDef   body;
        b2BodyId    id;
        b2ShapeDef  shape;

        Event<>::ListenerHandle collisionResolveHandle;
        Event<FVector2>::ListenerHandle moveHandle;
        Event<Angle>::ListenerHandle rotateHandle;

    public:
        // Require a SpriteComponent
        PhysicsComponent(Actor* _owner);
        PhysicsComponent(Actor* _owner, const Float& _radius);
        PhysicsComponent(Actor* _owner, const FVector2& _size);

        void OnCollisionResolve();

        void SetDynamic();
        void SetStatic();
        void SetKinematic();

    private:
        void Init();
        void MakeCircle(const Float& _radius);
        void MakeRectangle(const FVector2& _size);
    };
}
