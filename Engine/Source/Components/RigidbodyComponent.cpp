#include "RigidbodyComponent.h"
#include "Actors/Actor.h"

Krampus::RigidbodyComponent::RigidbodyComponent(Actor* _owner, const Float& _mass)
	: Component(_owner)
{
    name = NAME_OF(RigidbodyComponent);
	mass = FMath::MaxVal(0.001f, _mass);
	inverseMass = 1.0f / mass;
    ComputeInertia();
}

void Krampus::RigidbodyComponent::AddForce(const FVector2& _force, const FVector2& _applyPoint)
{
	if (isKinematic) return;
	accumulatedForces += _force;

    if (_applyPoint != FVector2::Zero())
    {
        const FVector2& _r = _applyPoint - GetActorPosition();
        accumulatedTorque += _r.Cross(_force);
    }
}

void Krampus::RigidbodyComponent::AddImpulse(const FVector2& _impulse, const FVector2& contactVector)
{
	if (isKinematic) return;
	velocity += _impulse * inverseMass;
    angularVelocity += inverseInertia * contactVector.Cross(_impulse);
}

void Krampus::RigidbodyComponent::ClearForces()
{
    accumulatedForces = FVector2::Zero();
    accumulatedTorque = 0.f;
}

void Krampus::RigidbodyComponent::Tick(const Float& _deltaTime)
{
	if (isKinematic) return;

    if (useGravity)
        accumulatedForces += FVector2(0, gravity * mass);

	Integrate(_deltaTime);
}

void Krampus::RigidbodyComponent::BindCollisionResponse()
{
	CollisionComponent* _collision = owner->GetComponent<CollisionComponent>();
	if (!_collision)
	{
		LOG_ERROR("You try to bind a response to the collision component that is nullptr");
		return;
	}
    onCollisionHandle = _collision->onCollision.AddListener(this, &RigidbodyComponent::OnCollision);
}

void Krampus::RigidbodyComponent::OnCollision(const CollisionInfo& info)
{
    if (isKinematic || !isActive)
        return;

    RigidbodyComponent* _physA = this;
    RigidbodyComponent* _physB = nullptr;

    if (info.collision)
        _physB = info.collision->GetOwner()->GetComponent<RigidbodyComponent>();

    // Masse infinie si pas de PhysicsComponent
    float _invMassA = _physA->inverseMass;
    float _invInertiaA = _physA->inverseInertia;

    float invMassB = _physB && !_physB->isKinematic ? _physB->inverseMass : Float(0.0f);
    float invInertiaB = _physB && !_physB->isKinematic ? _physB->inverseInertia : Float(0.0f);

    const FVector2& normal = info.normal;

    for (const FVector2& contact : info.contacts)
    {
        FVector2 rA = contact - _physA->owner->GetActorPosition();
        FVector2 rB = FVector2::Zero();

        if (_physB)
            rB = contact - _physB->owner->GetActorPosition();

        // Vélocité au point de contact
        FVector2 vA = _physA->velocity + FVector2(-_physA->angularVelocity * rA.y, _physA->angularVelocity * rA.x);
        FVector2 vB = FVector2::Zero();

        if (_physB && !_physB->isKinematic)
            vB = _physB->velocity + FVector2(-_physB->angularVelocity * rB.y, _physB->angularVelocity * rB.x);

        FVector2 relativeVel = vA - vB;
        float velAlongNormal = relativeVel.Dot(normal);

        // Les objets s'éloignent
        if (velAlongNormal > 0.f)
            continue;

        // ---------------------------
        // IMPULSION NORMALE
        // ---------------------------
        float restitution = _physA->restitution;
        if (_physB)
            restitution = FMath::MinVal(restitution, _physB->restitution);

        float rAcrossN_A = rA.Cross(normal);
        float rAcrossN_B = rB.Cross(normal);

        float denom =
            _invMassA + invMassB +
            rAcrossN_A * rAcrossN_A * _invInertiaA +
            rAcrossN_B * rAcrossN_B * invInertiaB;

        if (denom <= 0.f)
            continue;

        float j = -(1.f + restitution) * velAlongNormal;
        j /= denom;

        FVector2 impulse = normal * j;

        _physA->AddImpulse(impulse, rA);
        if (_physB && !_physB->isKinematic)
            _physB->AddImpulse(impulse * -1, rB);

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
            _invMassA + invMassB +
            rAt * rAt * _invInertiaA +
            rBt * rBt * invInertiaB;

        if (denomT <= 0.f)
            continue;

        jt /= denomT;

        float mu_s = _physA->staticFriction;
        float mu_d = _physA->dynamicFriction;

        if (_physB)
        {
            mu_s = FMath::Sqrt(mu_s * _physB->staticFriction);
            mu_d = FMath::Sqrt(mu_d * _physB->dynamicFriction);
        }

        FVector2 frictionImpulse;

        if (FMath::Abs(jt) < j * mu_s)
            frictionImpulse = tangent * jt;               // statique
        else
            frictionImpulse = tangent * -j * mu_d;        // dynamique

        _physA->AddImpulse(frictionImpulse, rA);
        if (_physB && !_physB->isKinematic)
            _physB->AddImpulse(frictionImpulse * -1, rB);

        // ---------------------------
        // CORRECTION DE PÉNÉTRATION
        // ---------------------------
        const float percent = 0.8f;   // 80%
        const float slop = 0.01f;

        float totalInvMass = _invMassA + invMassB;
        if (totalInvMass <= 0.f)
            continue;

        FVector2 correction =
            normal *
            (FMath::MaxVal(info.penetration - slop, 0.f) / totalInvMass) *
            percent;

        _physA->owner->Move(correction * _invMassA);
        if (_physB && !_physB->isKinematic)
            _physB->owner->Move(-(correction * invMassB));
    }
}

void Krampus::RigidbodyComponent::Integrate(const Float& _deltaTime)
{
    velocity += (accumulatedForces * inverseMass) * _deltaTime;
    float _speedSq = velocity.Dot(velocity);
    if (_speedSq > maxLinearVelocity * maxLinearVelocity)
        velocity *= maxLinearVelocity / FMath::Sqrt(_speedSq);

    velocity *= FMath::Clamp(1.f - linearDamping * _deltaTime, 0.f, 1.f);
    const FVector2& _force = FVector2
    (
        freezeMovementX ? 0.0f : velocity.x,
        freezeMovementY ? 0.0f : velocity.y
    );
    owner->Move(_force * _deltaTime);

    if (!freezeRotation)
    {
        angularVelocity += accumulatedTorque * inverseInertia * _deltaTime;
        angularVelocity *= FMath::Clamp(1.f - angularDamping * _deltaTime, 0.f, 1.f);
        angularVelocity = FMath::Clamp(angularVelocity, -maxAngularVelocity, maxAngularVelocity);
        owner->Rotate(angularVelocity * _deltaTime);
    }

    ClearForces();
}

void Krampus::RigidbodyComponent::ComputeInertia()
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
    inverseInertia = inertia > 0.f ? 1.f / inertia : Float(0.0f);
}

std::string Krampus::RigidbodyComponent::ToString() const
{
    return name + " -> Mass = " + mass.ToString() + ", Velocity = " + velocity.ToString();
}
