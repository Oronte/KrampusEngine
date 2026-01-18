#include "PhysicsComponent.h"
#include "Actors/Actor.h"

Krampus::PhysicsComponent::PhysicsComponent(Actor* _owner, const float& _mass)
	: Component(_owner)
{
	mass = FMath::MaxVal(0.001f, _mass);
	inverseMass = 1.0f / mass;
    ComputeInertia();
}

void Krampus::PhysicsComponent::AddForce(const FVector2& _force, const FVector2& _applyPoint)
{
	if (isKinematic) return;
	accumulatedForces += _force;

    if (_applyPoint != FVector2::Zero())
    {
        const FVector2& _r = _applyPoint - owner->transform.position;
        accumulatedTorque += _r.Cross(_force);
    }
}

void Krampus::PhysicsComponent::AddImpulse(const FVector2& _impulse, const FVector2& contactVector)
{
	if (isKinematic) return;
	velocity += _impulse * inverseMass;
    angularVelocity += inverseInertia * contactVector.Cross(_impulse);
}

void Krampus::PhysicsComponent::ClearForces()
{
    accumulatedForces = FVector2::Zero();
    accumulatedTorque = 0.f;
}

void Krampus::PhysicsComponent::Tick(const float& _deltaTime)
{
	if (isKinematic) return;

    if (useGravity)
        accumulatedForces += FVector2(0, gravity * mass);

	Integrate(_deltaTime);
}

void Krampus::PhysicsComponent::BindCollisionResponse()
{
	CollisionComponent* _collision = owner->GetComponent<CollisionComponent>();
	if (!_collision)
	{
		LOG_ERROR("You try to bind a response to the collision component that is nullptr");
		return;
	}
	_collision->onCollision.AddListener(this, &PhysicsComponent::OnCollision);
}

void Krampus::PhysicsComponent::OnCollision(const CollisionInfo& info)
{
    if (isKinematic)
        return;

    PhysicsComponent* physA = this;
    PhysicsComponent* physB = nullptr;

    if (info.collision)
        physB = info.collision->GetOwner()->GetComponent<PhysicsComponent>();

    // Masse infinie si pas de PhysicsComponent
    float invMassA = physA->inverseMass;
    float invInertiaA = physA->inverseInertia;

    float invMassB = physB && !physB->isKinematic ? physB->inverseMass : 0.f;
    float invInertiaB = physB && !physB->isKinematic ? physB->inverseInertia : 0.f;

    const FVector2& normal = info.normal;

    for (const FVector2& contact : info.contacts)
    {
        FVector2 rA = contact - physA->owner->transform.position;
        FVector2 rB = FVector2::Zero();

        if (physB)
            rB = contact - physB->owner->transform.position;

        // Vélocité au point de contact
        FVector2 vA = physA->velocity + FVector2(-physA->angularVelocity * rA.y, physA->angularVelocity * rA.x);
        FVector2 vB = FVector2::Zero();

        if (physB && !physB->isKinematic)
            vB = physB->velocity + FVector2(-physB->angularVelocity * rB.y, physB->angularVelocity * rB.x);

        FVector2 relativeVel = vA - vB;
        float velAlongNormal = relativeVel.Dot(normal);

        // Les objets s'éloignent
        if (velAlongNormal > 0.f)
            continue;

        // ---------------------------
        // IMPULSION NORMALE
        // ---------------------------
        float restitution = physA->restitution;
        if (physB)
            restitution = FMath::MinVal(restitution, physB->restitution);

        float rAcrossN_A = rA.Cross(normal);
        float rAcrossN_B = rB.Cross(normal);

        float denom =
            invMassA + invMassB +
            rAcrossN_A * rAcrossN_A * invInertiaA +
            rAcrossN_B * rAcrossN_B * invInertiaB;

        if (denom <= 0.f)
            continue;

        float j = -(1.f + restitution) * velAlongNormal;
        j /= denom;

        FVector2 impulse = normal * j;

        physA->AddImpulse(impulse, rA);
        if (physB && !physB->isKinematic)
            physB->AddImpulse(impulse * -1, rB);

        // ---------------------------
        // FRICTION
        // ---------------------------
        FVector2 tangent = relativeVel - normal * velAlongNormal;
        float tangentLenSq = tangent.LengthSquared();

        if (tangentLenSq > 1e-6f)
            tangent /= FMath::Sqrt(tangentLenSq);
        else
            continue;

        float jt = -relativeVel.Dot(tangent);

        float rAt = rA.Cross(tangent);
        float rBt = rB.Cross(tangent);

        float denomT =
            invMassA + invMassB +
            rAt * rAt * invInertiaA +
            rBt * rBt * invInertiaB;

        if (denomT <= 0.f)
            continue;

        jt /= denomT;

        float mu_s = physA->staticFriction;
        float mu_d = physA->dynamicFriction;

        if (physB)
        {
            mu_s = FMath::Sqrt(mu_s * physB->staticFriction);
            mu_d = FMath::Sqrt(mu_d * physB->dynamicFriction);
        }

        FVector2 frictionImpulse;

        if (FMath::Abs(jt) < j * mu_s)
            frictionImpulse = tangent * jt;               // statique
        else
            frictionImpulse = tangent * -j * mu_d;        // dynamique

        physA->AddImpulse(frictionImpulse, rA);
        if (physB && !physB->isKinematic)
            physB->AddImpulse(frictionImpulse * -1, rB);

        // ---------------------------
        // CORRECTION DE PÉNÉTRATION
        // ---------------------------
        const float percent = 0.8f;   // 80%
        const float slop = 0.01f;

        float totalInvMass = invMassA + invMassB;
        if (totalInvMass <= 0.f)
            continue;

        FVector2 correction =
            normal *
            (FMath::MaxVal(info.penetration - slop, 0.f) / totalInvMass) *
            percent;

        physA->owner->transform.position += correction * invMassA;
        if (physB && !physB->isKinematic)
            physB->owner->transform.position -= correction * invMassB;
    }
}

void Krampus::PhysicsComponent::Integrate(const float& _deltaTime)
{
    velocity += (accumulatedForces * inverseMass) * _deltaTime;
    float _speedSq = velocity.Dot(velocity);
    if (_speedSq > maxLinearVelocity * maxLinearVelocity)
        velocity *= maxLinearVelocity / FMath::Sqrt(_speedSq);

    velocity *= FMath::Clamp(1.f - linearDamping * _deltaTime, 0.f, 1.f);
    owner->transform.position += velocity * _deltaTime;

    angularVelocity += accumulatedTorque * inverseInertia * _deltaTime;
    angularVelocity *= FMath::Clamp(1.f - angularDamping * _deltaTime, 0.f, 1.f);
    angularVelocity = FMath::Clamp(angularVelocity, -maxAngularVelocity, maxAngularVelocity);
    owner->transform.rotation += angularVelocity * _deltaTime;

    ClearForces();
}

void Krampus::PhysicsComponent::ComputeInertia()
{
    CollisionComponent* _collision = owner->GetComponent<CollisionComponent>();
    if (!_collision)
    {
        inertia = 1.f;
        inverseInertia = 1.f;
        LOG_WARNING("You have a PhysicsComponent but not a CollisionComponent");
        return;
    }

    if (_collision->GetShapeType() == ShapeType::Circle)
    {
        const float _radius = _collision->GetRadius();
        inertia = 0.5f * mass * _radius * _radius;
    }
    else // Rectangle
    {
        const FVector2& _size = _collision->GetSize();
        inertia = (1.f / 12.f) * mass * (_size.x * _size.x + _size.y * _size.y);
    }
    inverseInertia = inertia > 0.f ? 1.f / inertia : 0.f;
}
