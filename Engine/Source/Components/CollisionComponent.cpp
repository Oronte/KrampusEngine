#include "CollisionComponent.h"
#include "Actors/Actor.h"
#include "Managers/CollisionManager.h"
#include "SpriteComponent.h"

Krampus::CollisionComponent::CollisionComponent(Actor* _owner)
	: Component(_owner)
{
	sprite = owner->GetComponent<SpriteComponent>();
	if (!sprite)
	{
		LOG(VerbosityType::Error, "You need to have a sprite component for this Component");
		return;
	}

	M_COLLISION.Register(this);
}

void Krampus::CollisionComponent::ComputeCollision(CollisionComponent* _other)
{
	const ShapeType& _ownerType = sprite->GetShapeObject()->GetShapeType();
	const ShapeType& _otherType = _other->sprite->GetShapeObject()->GetShapeType();

	if (_ownerType == ShapeType::Circle)
	{
		if (_otherType == ShapeType::Circle) CircleToCircle(_other);
		else /*CircleToRect(this, _other)*/;
	}
	else if (_otherType == ShapeType::Rectangle) RectToRect(_other);
	else /*CircleToRect(_other, this)*/;

}

bool Krampus::CollisionComponent::CircleToCircle(CollisionComponent* _other)
{
	const FVector2& _ownerPosition = owner->transform.position;
	const FVector2& _otherPosition = _other->owner->transform.position;

	const float& _ownerRadius = sprite->GetShapeObject()->GetSizeData().radius;
	const float& _otherRadius = _other->sprite->GetShapeObject()->GetSizeData().radius;

	const float& _distance = _ownerPosition.Distance(_otherPosition);
	const float& _radiusSum = _ownerRadius + _otherRadius;

	if (_distance > _radiusSum) return false;

	const FVector2& _normal = _distance == 0.0f ? FVector2() :
		(_otherPosition - _ownerPosition) / -_distance;

	CollisionInfo _hitInfo;

	_hitInfo.normal = _normal;
	_hitInfo.penetration = _radiusSum - _distance;
	_hitInfo.contactPoint = _ownerPosition + _normal * (_ownerRadius - (_radiusSum - _distance) / 2.0f);
	_hitInfo.collision = _other;

	onCollision.Broadcast(_hitInfo);
	_other->onCollision.Broadcast(_hitInfo);
	return true;
}

bool Krampus::CollisionComponent::RectToRect(CollisionComponent* _other)
{
    const FVector2& _ownerPosition = owner->transform.position;
    const FVector2& _ownerHalfSize = sprite->GetShapeObject()->GetSizeData().size * 0.5f;
    const Angle& _ownerRotation = owner->transform.rotation;

    const FVector2& _otherPosition = _other->owner->transform.position;
    const FVector2& _otherHalfSize = _other->sprite->GetShapeObject()->GetSizeData().size * 0.5f;
    const Angle& _otherRotation = _other->owner->transform.rotation;

    FVector2 axesA[2];
    FVector2 axesB[2];
    GetAxes(this, axesA);
    GetAxes(_other, axesB);

    float minPenetration = FLT_MAX;
    FVector2 bestAxis;

    FVector2 delta = _otherPosition - _ownerPosition;

    auto TestAxis = [&](const FVector2& axis) -> bool
        {
            float dist = fabs(delta.Dot(axis));
            float projA = ProjectOBB(this, axis);
            float projB = ProjectOBB(_other, axis);

            float overlap = projA + projB - dist;
            if (overlap < 0.0f)
                return false;

            if (overlap < minPenetration)
            {
                minPenetration = overlap;
                bestAxis = axis;
            }
            return true;
        };

    // Test des 4 axes
    if (!TestAxis(axesA[0])) return false;
    if (!TestAxis(axesA[1])) return false;
    if (!TestAxis(axesB[0])) return false;
    if (!TestAxis(axesB[1])) return false;

    // Normal orientée correctement
    if (delta.Dot(bestAxis) > 0.0f)
        bestAxis *= -1;

    CollisionInfo hitInfo;
    hitInfo.normal = bestAxis;
    hitInfo.penetration = minPenetration;
    hitInfo.collision = _other;

    // Contact point approximatif (suffisant pour un moteur 2D)
    hitInfo.contactPoint = _ownerPosition + bestAxis * (_ownerHalfSize.x);

    onCollision.Broadcast(hitInfo);
    _other->onCollision.Broadcast(hitInfo);

    return true;
}

void Krampus::CollisionComponent::GetAxes(const CollisionComponent* _component, FVector2 axes[2])
{
	const float& _cos = FMath::Cos(_component->owner->transform.rotation);
	const float& _sin = FMath::Sin(_component->owner->transform.rotation);

	axes[0] = FVector2(_cos, _sin);
	axes[1] = FVector2(-_sin, _cos);
}

float Krampus::CollisionComponent::ProjectOBB(const CollisionComponent* _component, const FVector2& axis)
{
	FVector2 axes[2];
	GetAxes(_component, axes);

    return
        (_component->sprite->GetShapeObject()->GetSizeData().size / 2.0f).x * fabs(axis.Dot(axes[0])) +
        (_component->sprite->GetShapeObject()->GetSizeData().size / 2.0f).y * fabs(axis.Dot(axes[1]));
}
