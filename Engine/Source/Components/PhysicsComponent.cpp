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
    if (isKinematic) return; // les objets kinematic ne réagissent pas

    PhysicsComponent* otherPhys = info.collision ? info.collision->GetOwner()->GetComponent<PhysicsComponent>() : nullptr;

    float invMassA = inverseMass;
    float invInertiaA = inverseInertia;

    CollisionComponent* col = owner->GetComponent<CollisionComponent>();
    bool isCircle = col && col->GetShapeType() == ShapeType::Circle;

    // Traitement de tous les points de contact
    for (const FVector2& contact : info.contacts)
    {
        FVector2 rA = contact - owner->transform.position;

        // Vélocité relative
        FVector2 vA = velocity + FVector2(-angularVelocity * rA.y, angularVelocity * rA.x);
        FVector2 vB = FVector2::Zero();
        if (otherPhys && !otherPhys->isKinematic)
        {
            FVector2 rB = contact - otherPhys->owner->transform.position;
            vB = otherPhys->velocity + FVector2(-otherPhys->angularVelocity * rB.y, otherPhys->angularVelocity * rB.x);
        }

        FVector2 relativeVel = vA - vB;
        float velAlongNormal = relativeVel.Dot(info.normal);

        if (velAlongNormal > 0.f) continue; // objets s'éloignent

        // --- IMPULSION NORMALE ---
        float e = restitution;
        if (otherPhys) e = FMath::MinVal(restitution, otherPhys->restitution);

        float rAcrossN_A = rA.Cross(info.normal);
        float denom = invMassA + rAcrossN_A * rAcrossN_A * invInertiaA;

        float j = -(1.f + e) * velAlongNormal / denom;
        FVector2 impulse = info.normal * j;

        // Appliquer seulement à cet objet
        AddImpulse(impulse, rA);

        // --- FRICTION ---
        FVector2 tangent = relativeVel - (info.normal * velAlongNormal);
        float tangentLen2 = tangent.LengthSquared();

        // Clamp petites valeurs
        if (tangentLen2 > 1e-6f)
            tangent = tangent.Normalized();
        else
            tangent = FVector2::Zero();

        // Pour les cercles, éviter rotation si quasi immobile
        if (isCircle && tangentLen2 < 1e-4f)
            tangent = FVector2::Zero();

        float jt = -relativeVel.Dot(tangent) / denom;
        FVector2 frictionImpulse;
        float mu = dynamicFriction;

        if (FMath::Abs(jt) < j * mu)
            frictionImpulse = tangent * jt; // statique
        else
            frictionImpulse = tangent * -j * mu; // dynamique

        AddImpulse(frictionImpulse, rA);

        // --- CORRECTION DE PÉNÉTRATION ---
        const float percent = 0.8f;
        const float slop = 0.01f;
        FVector2 correction = info.normal * FMath::MaxVal(info.penetration - slop, 0.f) * percent * invMassA;

        if (correction.LengthSquared() < 1e-6f) // clamp très petit déplacement
            correction = FVector2::Zero();

        owner->transform.position += correction;
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
