#include "CollisionComponent.h"
#include "Actors/Actor.h"
#include "GameFramework/Level.h"
#include "Managers/CollisionManager.h"
#include "SpriteComponent.h"

Krampus::CollisionComponent::CollisionComponent(Actor* _owner, const CollisionChannel& _channel, const CollisionChannel& _mask)
	: Component(_owner)
{
	channel = _channel;
	mask = _mask;

	SpriteComponent* _sprite = owner->GetComponent<SpriteComponent>();
	if (!_sprite)
	{
		LOG(VerbosityType::Error, "You need to have a sprite component for this Component");
		return;
	}

	shapeType = _sprite->GetShapeType();
	if (shapeType == ShapeType::Circle) sizeData.radius = _sprite->GetShapeSizeData().radius;
	else sizeData.size = _sprite->GetShapeSizeData().size;

	level->GetCollisionManagerRef().Register(this);
}

Krampus::CollisionComponent::CollisionComponent(Actor* _owner, const float& _radius, const CollisionChannel& _channel, const CollisionChannel& _mask)
	: Component(_owner)
{
	channel = _channel;
	mask = _mask;

	sizeData.radius = _radius;
	shapeType = ShapeType::Circle;

	level->GetCollisionManagerRef().Register(this);
}

Krampus::CollisionComponent::CollisionComponent(Actor* _owner, const FVector2& _size, const CollisionChannel& _channel, const CollisionChannel& _mask)
	: Component(_owner)
{
	channel = _channel;
	mask = _mask;

	sizeData.size = _size;
	shapeType = ShapeType::Circle;

	level->GetCollisionManagerRef().Register(this);
}

void Krampus::CollisionComponent::ResolveCollision(const CollisionInfo& _info)
{
	transform.position += _info.normal * _info.penetration;
}

void Krampus::CollisionComponent::BindCollisionResolution()
{
	onCollisionHandle = onCollision.AddListener(this, &CollisionComponent::ResolveCollision);
}

void Krampus::CollisionComponent::ComputeCollision(CollisionComponent* _other)
{
	if (!CanCollide(_other)) return;

	const ShapeType& _otherType = _other->shapeType;

	if (shapeType == ShapeType::Circle)
	{
		if (_otherType == ShapeType::Circle) CircleToCircle(_other);
		else CircleToRect(this, _other);
	}
	else if (_otherType == ShapeType::Rectangle)
	{
		// TODO Remove
		//if (FMath::Abs(transform.rotation) > 0.02f || FMath::Abs(_other->transform.rotation) > 0.02f)
		//	RectToRectOBB(_other);
		//else RectToRectAABB(_other);
		RectToRectOBB(_other);
	}
	else CircleToRect(_other, this);

}

void Krampus::CollisionComponent::Tick(const float& _deltaTime)
{
	Component::Tick(_deltaTime);

	DrawDebug();
}

bool Krampus::CollisionComponent::CanCollide(const CollisionComponent* _other) const
{
	return CAST(uint32_t, mask & _other->channel) != 0 &&
		CAST(uint32_t, _other->mask & channel) != 0;
}

bool Krampus::CollisionComponent::CircleToCircle(CollisionComponent* _other)
{
	CollisionInfo _hitInfo, _otherHitInfo;

    const bool _collision = Physics::CircleToCircle(transform.position, sizeData.radius,
        _other->transform.position, _other->sizeData.radius,
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

    const bool _collision = Physics::RectToRectOBB(FRect(transform.position, sizeData.size), transform.rotation,
        FRect(_other->transform.position, _other->sizeData.size), _other->transform.rotation,
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

	const bool _collision = Physics::RectToRectAABB(FRect(transform.position, sizeData.size),
		FRect(_other->transform.position, _other->sizeData.size),
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

	const bool _collision = Physics::CircleToRect(_circle->transform.position, _circle->sizeData.radius,
		FRect(_rect->transform.position, _rect->sizeData.size), _rect->transform.rotation,
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

	level->GetCollisionManagerRef().RemoveComponent(this);
}

void Krampus::CollisionComponent::DrawDebug()
{
#ifdef DEBUG
	if (!useDebug) return;

	switch (shapeType)
	{
	case ShapeType::Circle:
		Debug::DrawDebugCircle(level, transform.position, sizeData.radius, 15, Color::Red());
		break;

	case ShapeType::Rectangle:
		Debug::DrawDebugRect(level, transform.position, sizeData.size, transform.rotation, Color::Red());
		break;
	};
#endif
}
