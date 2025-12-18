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
		else CircleToRect(this, _other);
	}
	else if (_otherType == ShapeType::Rectangle)
	{
		if (FMath::Abs(owner->transform.rotation) > 0.02f || FMath::Abs(_other->owner->transform.rotation) > 0.02f)
			RectToRectOBB(_other);
		else RectToRectAABB(_other);
	}
	else CircleToRect(_other, this);

}

bool Krampus::CollisionComponent::CircleToCircle(CollisionComponent* _other)
{
	CollisionInfo _hitInfo, _otherHitInfo;

    const bool _collision = Physics::CircleToCircle(owner->transform.position, sprite->GetShapeObject()->GetSizeData().radius,
        _other->owner->transform.position, _other->sprite->GetShapeObject()->GetSizeData().radius,
        _hitInfo, _otherHitInfo);

    if (!_collision) return false;

	_hitInfo.collision = _other;
    _otherHitInfo.collision = this;

	onCollision.Broadcast(_hitInfo);
	_other->onCollision.Broadcast(_otherHitInfo);

	return true;
}

bool Krampus::CollisionComponent::RectToRectOBB(CollisionComponent* _other)
{
    CollisionInfo _hitInfo, _otherHitInfo;

    const bool _collision = Physics::RectToRectOBB(FRect(owner->transform.position, sprite->GetShapeObject()->GetSizeData().size), owner->transform.rotation,
        FRect(_other->owner->transform.position, _other->sprite->GetShapeObject()->GetSizeData().size), _other->owner->transform.rotation,
        _hitInfo, _otherHitInfo);

	if (!_collision) return false;

    _hitInfo.collision = _other;
	_otherHitInfo.collision = this;

    onCollision.Broadcast(_hitInfo);
    _other->onCollision.Broadcast(_otherHitInfo);

    return true;
}

bool Krampus::CollisionComponent::RectToRectAABB(CollisionComponent* _other)
{
	CollisionInfo _hitInfo, _otherHitInfo;

	const bool _collision = Physics::RectToRectAABB(FRect(owner->transform.position, sprite->GetShapeObject()->GetSizeData().size),
		FRect(_other->owner->transform.position, _other->sprite->GetShapeObject()->GetSizeData().size),
		_hitInfo, _otherHitInfo);

	if (!_collision) return false;

	_hitInfo.collision = _other;
	_otherHitInfo.collision = this;

	onCollision.Broadcast(_hitInfo);
	_other->onCollision.Broadcast(_otherHitInfo);

	return true;
}

bool Krampus::CollisionComponent::CircleToRect(CollisionComponent* _circle, CollisionComponent* _rect)
{
	CollisionInfo _circleInfo, _rectInfo;

	const bool _collision = Physics::CircleToRect(_circle->owner->transform.position, _circle->sprite->GetShapeObject()->GetSizeData().radius,
		FRect(_rect->owner->transform.position, _rect->sprite->GetShapeObject()->GetSizeData().size), _rect->owner->transform.rotation,
		_circleInfo, _rectInfo);

	if (!_collision) return false;

	_circleInfo.collision = _rect;
	_rectInfo.collision = _circle;

	_circle->onCollision.Broadcast(_circleInfo);
	_rect->onCollision.Broadcast(_rectInfo);

	return true;
}
