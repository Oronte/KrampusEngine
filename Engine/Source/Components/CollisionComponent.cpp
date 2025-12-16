#include "CollisionComponent.h"
#include "Actors/Actor.h"
#include "Managers/CollisionManager.h"

Krampus::CollisionComponent::CollisionComponent(Actor* _owner)
	: Component(_owner)
{
	sprite = owner->GetComponent<SpriteComponent>();
	if (!sprite)
	{
		LOG(VerbosityType::Error, "You need to have a sprite component for this Component");
		return;
	}
	
	halfSize = sprite->GetShapeObject()->GetSizeData().size / 2.0f;

	M_COLLISION.Register(this);
}

void Krampus::CollisionComponent::ComputeCollision(CollisionComponent* _other)
{
	//if (type == ShapeType::Circle)
	//{
		/*if (_other->type == ShapeType::Circle)*/ CircleToCircle(_other);
	//}
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