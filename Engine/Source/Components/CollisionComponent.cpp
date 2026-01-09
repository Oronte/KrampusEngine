#include "CollisionComponent.h"
#include "Actors/Actor.h"
#include "Managers/CollisionManager.h"
#include "SpriteComponent.h"

Krampus::CollisionComponent::CollisionComponent(Actor* _owner, const CollisionChannel& _channel, const CollisionChannel& _mask)
	: Component(_owner)
{
	channel = _channel;
	mask = _mask;

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
	if (!CanCollide(_other)) return;

	const ShapeType& _ownerType = sprite->GetShapeObject()->GetShapeType();
	const ShapeType& _otherType = _other->sprite->GetShapeObject()->GetShapeType();

	if (_ownerType == ShapeType::Circle)
	{
		if (_otherType == ShapeType::Circle) CircleToCircle(_other);
		else CircleToRect(this, _other);
	}
	else if (_otherType == ShapeType::Rectangle)
	{
		if (FMath::Abs(transform.rotation) > 0.02f || FMath::Abs(_other->transform.rotation) > 0.02f)
			RectToRectOBB(_other);
		else RectToRectAABB(_other);
	}
	else CircleToRect(_other, this);

}

bool Krampus::CollisionComponent::CanCollide(const CollisionComponent* _other) const
{
	return CAST(uint32_t, mask & _other->channel) != 0 &&
		CAST(uint32_t, _other->mask & channel) != 0;
}

bool Krampus::CollisionComponent::CircleToCircle(CollisionComponent* _other)
{
	CollisionInfo _hitInfo, _otherHitInfo;

    const bool _collision = Physics::CircleToCircle(transform.position, sprite->GetShapeObject()->GetSizeData().radius,
        _other->transform.position, _other->sprite->GetShapeObject()->GetSizeData().radius,
        _hitInfo, _otherHitInfo);

	if (!_collision)
	{
		if (collidingComponents.contains(_other))
		{
			collidingComponents.erase(_other);
			onCollisionExit.Broadcast(_other);
			_other->onCollisionExit.Broadcast(this);
		}
		return false;
	}

	_hitInfo.collision = _other;
    _otherHitInfo.collision = this;

	if (!collidingComponents.contains(_other))
	{
		collidingComponents.insert(_other);
		onCollisionEnter.Broadcast(_hitInfo);
		_other->onCollisionEnter.Broadcast(_otherHitInfo);

	}

	onCollision.Broadcast(_hitInfo);
	_other->onCollision.Broadcast(_otherHitInfo);

	return true;
}

bool Krampus::CollisionComponent::RectToRectOBB(CollisionComponent* _other)
{
	CollisionInfo _hitInfo, _otherHitInfo;

    const bool _collision = Physics::RectToRectOBB(FRect(transform.position, sprite->GetShapeObject()->GetSizeData().size), transform.rotation,
        FRect(_other->transform.position, _other->sprite->GetShapeObject()->GetSizeData().size), _other->transform.rotation,
        _hitInfo, _otherHitInfo);

	if (!_collision)
	{
		if (collidingComponents.contains(_other))
		{
			collidingComponents.erase(_other);
			onCollisionExit.Broadcast(_other);
			_other->onCollisionExit.Broadcast(this);
		}
		return false;
	}


    _hitInfo.collision = _other;
	_otherHitInfo.collision = this;

	if (!collidingComponents.contains(_other))
	{
		collidingComponents.insert(_other);
		onCollisionEnter.Broadcast(_hitInfo);
		_other->onCollisionEnter.Broadcast(_otherHitInfo);
	}

	onCollision.Broadcast(_hitInfo);
	_other->onCollision.Broadcast(_otherHitInfo);

    return true;
}

bool Krampus::CollisionComponent::RectToRectAABB(CollisionComponent* _other)
{
	CollisionInfo _hitInfo, _otherHitInfo;

	const bool _collision = Physics::RectToRectAABB(FRect(transform.position, sprite->GetShapeObject()->GetSizeData().size),
		FRect(_other->transform.position, _other->sprite->GetShapeObject()->GetSizeData().size),
		_hitInfo, _otherHitInfo);

	if (!_collision)
	{
		if (collidingComponents.contains(_other))
		{
			collidingComponents.erase(_other);
			onCollisionExit.Broadcast(_other);
			_other->onCollisionExit.Broadcast(this);
		}
		return false;
	}

	_hitInfo.collision = _other;
	_otherHitInfo.collision = this;

	if (!collidingComponents.contains(_other))
	{
		collidingComponents.insert(_other);
		onCollisionEnter.Broadcast(_hitInfo);
		_other->onCollisionEnter.Broadcast(_otherHitInfo);
	}

	onCollision.Broadcast(_hitInfo);
	_other->onCollision.Broadcast(_otherHitInfo);

	return true;
}

bool Krampus::CollisionComponent::CircleToRect(CollisionComponent* _circle, CollisionComponent* _rect)
{
	CollisionInfo _circleInfo, _rectInfo;

	const bool _collision = Physics::CircleToRect(_circle->transform.position, _circle->sprite->GetShapeObject()->GetSizeData().radius,
		FRect(_rect->transform.position, _rect->sprite->GetShapeObject()->GetSizeData().size), _rect->transform.rotation,
		_circleInfo, _rectInfo);

	CollisionComponent* _other = _circle == this ? _rect : _circle;
	if (!_collision)
	{
		if (collidingComponents.contains(_other))
		{
			collidingComponents.erase(_other);
			onCollisionExit.Broadcast(_other);
			_other->onCollisionExit.Broadcast(this);
		}
		return false;
	}

	_circleInfo.collision = _rect;
	_rectInfo.collision = _circle;

	if (!collidingComponents.contains(_other))
	{
		collidingComponents.insert(_other);
		_circle->onCollisionEnter.Broadcast(_circleInfo);
		_rect->onCollisionEnter.Broadcast(_rectInfo);
	}

	_circle->onCollision.Broadcast(_circleInfo);
	_rect->onCollision.Broadcast(_rectInfo);

	return true;
}

void Krampus::CollisionComponent::BeginDestroy()
{
	Component::BeginDestroy();

	M_COLLISION.RemoveComponent(this);
}
