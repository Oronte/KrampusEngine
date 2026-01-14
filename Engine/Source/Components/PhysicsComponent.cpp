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
    if (!info.hit || !info.collision)
        return;

    PhysicsComponent* otherPhysics = info.collision->GetOwner()->GetComponent<PhysicsComponent>();
    if (!otherPhysics) otherPhysics = nullptr;

    // --- 1. Séparation pour corriger la pénétration ---
    // On corrige la position avant d'appliquer les forces pour éviter les rebonds multiples
    const float percent = 0.8f;     // pourcentage de correction
    const float slop = 0.01f;       // tolérance min pour éviter jitter
    FVector2 correction = info.normal * FMath::MaxVal(info.penetration - slop, 0.0f) /
        (inverseMass + (otherPhysics ? otherPhysics->inverseMass : 0.0f)) * percent;

    if (!isKinematic)
        owner->transform.position += correction * inverseMass;

    if (otherPhysics && !otherPhysics->isKinematic)
        otherPhysics->owner->transform.position -= correction * otherPhysics->inverseMass;

    // --- 2. Calcul de la vitesse relative au point de contact ---
    FVector2 rA = info.contactPoint - owner->transform.position;
    FVector2 velA = velocity + FVector2{ -angularVelocity * rA.y, angularVelocity * rA.x }; // v + ω × r

    FVector2 velB = FVector2::Zero();
    FVector2 rB = FVector2::Zero();
    if (otherPhysics)
    {
        rB = info.contactPoint - otherPhysics->owner->transform.position;
        velB = otherPhysics->velocity + FVector2{ -otherPhysics->angularVelocity * rB.y, otherPhysics->angularVelocity * rB.x };
    }

    FVector2 relativeVelocity = velA - velB;

    // --- 3. Calcul de la composante normale ---
    float velAlongNormal = relativeVelocity.Dot(info.normal);
    if (velAlongNormal > 0)
        return; // les objets s'éloignent, pas besoin d'impulsion

    // --- 4. Calcul du moment d'inertie ---
    float invInertiaA = 0.f;
    float invInertiaB = 0.f;

    // Cercle: I = 0.5 * m * r^2 ; Rectangle: I = (1/12) * m * (w^2 + h^2)
    if (owner->GetComponent<CollisionComponent>())
    {
        CollisionComponent* col = owner->GetComponent<CollisionComponent>();
        if (col->GetShapeType() == ShapeType::Circle)
            invInertiaA = mass > 0.f ? 2.0f / (mass * col->GetRadius() * col->GetRadius()) : 0.f;
        else
        {
            FVector2 s = col->GetSize();
            invInertiaA = mass > 0.f ? 12.0f / (mass * (s.x * s.x + s.y * s.y)) : 0.f;
        }
    }

    if (otherPhysics && otherPhysics->owner->GetComponent<CollisionComponent>())
    {
        CollisionComponent* col = otherPhysics->owner->GetComponent<CollisionComponent>();
        if (col->GetShapeType() == ShapeType::Circle)
            invInertiaB = otherPhysics->mass > 0.f ? 2.0f / (otherPhysics->mass * col->GetRadius() * col->GetRadius()) : 0.f;
        else
        {
            FVector2 s = col->GetSize();
            invInertiaB = otherPhysics->mass > 0.f ? 12.0f / (otherPhysics->mass * (s.x * s.x + s.y * s.y)) : 0.f;
        }
    }

    // --- 5. Calcul de l'impulsion normale ---
    float e = std::min(restitution, otherPhysics ? otherPhysics->restitution : restitution); // restitution combinée
    float rAcrossN_A = rA.Cross(info.normal);
    float rAcrossN_B = rB.Cross(info.normal);

    float denom = inverseMass + (otherPhysics ? otherPhysics->inverseMass : 0.0f) +
        rAcrossN_A * rAcrossN_A * invInertiaA +
        rAcrossN_B * rAcrossN_B * invInertiaB;

    float j = -(1.0f + e) * velAlongNormal / denom;

    FVector2 impulse = info.normal * j;

    if (!isKinematic)
    {
        velocity += impulse * inverseMass;
        angularVelocity += rA.Cross(impulse) * invInertiaA;
    }

    if (otherPhysics && !otherPhysics->isKinematic)
    {
        otherPhysics->velocity -= impulse * otherPhysics->inverseMass;
        otherPhysics->angularVelocity -= rB.Cross(impulse) * invInertiaB;
    }

    // --- 6. Impulsion tangentielle pour friction ---
    FVector2 tangent = relativeVelocity - info.normal * velAlongNormal;
    tangent = tangent.Dot(tangent) > 0.0f ? tangent * (1.0f / std::sqrt(tangent.Dot(tangent))) : FVector2::Zero();

    float jt = -relativeVelocity.Dot(tangent) / denom;

    float mu = 0.5f; // coefficient de friction statique/dynamique (à ajuster)
    if (std::abs(jt) > j * mu)
        jt = j * mu * (jt < 0 ? -1.f : 1.f);

    FVector2 frictionImpulse = tangent * jt;

    if (!isKinematic)
    {
        velocity += frictionImpulse * inverseMass;
        angularVelocity += rA.Cross(frictionImpulse) * invInertiaA;
    }

    if (otherPhysics && !otherPhysics->isKinematic)
    {
        otherPhysics->velocity -= frictionImpulse * otherPhysics->inverseMass;
        otherPhysics->angularVelocity -= rB.Cross(frictionImpulse) * invInertiaB;
    }

    // --- 7. Limitation des vitesses pour stabilité ---
    const float maxAngularVelocity = 20.f; // rad/s
    if (angularVelocity > maxAngularVelocity) angularVelocity = maxAngularVelocity;
    if (angularVelocity < -maxAngularVelocity) angularVelocity = -maxAngularVelocity;
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
