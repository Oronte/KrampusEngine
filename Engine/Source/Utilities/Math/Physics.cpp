#include "Physics.h"

bool Krampus::Physics::CircleToCircle(const FVector2& _aPos, const float& _aRadius, const FVector2& _bPos, const float& _bRadius, CollisionInfo& _aInfo, CollisionInfo& _bInfo)
{
	const float _distance = _aPos.Distance(_bPos);
	const float _radiusSum = _aRadius + _bRadius;

	if (_distance > _radiusSum) return false;

	const FVector2& _normal = _distance == 0.0f ? FVector2() :
		(_bPos - _aPos) / -_distance;

	_aInfo.normal = _normal;
	_aInfo.penetration = _radiusSum - _distance;
	_aInfo.contactPoint = _aPos + _normal * (_aRadius - _aInfo.penetration / 2.0f);

	_bInfo.normal = _aInfo.normal * -1.0f;
	_bInfo.penetration = _aInfo.penetration;
	_bInfo.contactPoint = _aInfo.contactPoint;

	return true;
}

bool Krampus::Physics::RectToRectOBB(const FRect& _aRect, const Angle& _aRot, const FRect& _bRect, const Angle& _bRot, CollisionInfo& _aInfo, CollisionInfo& _bInfo)
{
    const FVector2& _aPos = _aRect.GetPosition();
    const FVector2& _aSize = _aRect.GetSize();

    const FVector2& _bPos = _bRect.GetPosition();
    const FVector2& _bSize = _bRect.GetSize();

    // Axes used for the Separating Axis Theorem (2 per OBB)
    FVector2 _axesA[2];
    FVector2 _axesB[2];
    GetAxes(_aRot, _axesA);
    GetAxes(_bRot, _axesB);

    float _minPenetration = FLT_MAX;
    FVector2 _bestAxis;

    FVector2 _delta = _bPos - _aPos;

    const std::function<bool(const FVector2&)> _testOverlap = [&](const FVector2& _axis) -> bool
        {
            float _dist = FMath::Abs(_delta.Dot(_axis));
            float _projA = ProjectOBB(_aSize, _aRot, _axis);
            float _projB = ProjectOBB(_bSize, _bRot, _axis);

            float _overlap = _projA + _projB - _dist;
            if (_overlap < 0.0f)
                return false;

            if (_overlap < _minPenetration)
            {
                _minPenetration = _overlap;
                _bestAxis = _axis;
            }
            return true;
        };

    if (!_testOverlap(_axesA[0])) return false;
    if (!_testOverlap(_axesA[1])) return false;
    if (!_testOverlap(_axesB[0])) return false;
    if (!_testOverlap(_axesB[1])) return false;

    if (_delta.Dot(_bestAxis) > 0.0f)
        _bestAxis *= -1;

    _aInfo.normal = _bestAxis;
    _aInfo.penetration = _minPenetration;
    _aInfo.contactPoint = _aPos + _bestAxis * (_aSize.x * 0.5f);

    _bInfo.normal = _aInfo.normal * -1.0f;
    _bInfo.penetration = _aInfo.penetration;
    _bInfo.contactPoint = _bPos + _bestAxis * (_bSize.x * 0.5f);

    return true;
}

bool Krampus::Physics::RectToRectAABB(const FRect& _aRect, const FRect& _bRect, CollisionInfo& _aInfo, CollisionInfo& _bInfo)
{
    std::optional<FRect> _result = _aRect.FindIntersection(_bRect);

    if (!_result.has_value()) return false;

    const FRect& _intersection = _result.value();

    const FVector2& _sizeIntersection = _intersection.GetSize();

    const FVector2& _aPos = _aRect.GetPosition();
    const FVector2& _bPos = _bRect.GetPosition();

    const FVector2& _aHalfSize = _aRect.GetSize() * 0.5f;
    const FVector2& _bHalfSize = _bRect.GetSize() * 0.5f;

    FVector2 _normal;
    FVector2 _contactPoint;

    const FVector2& _delta = _bPos - _aPos;

    if (_sizeIntersection.x < _sizeIntersection.y) // Horizontale
    {
        _normal = (_delta.x < 0.f) ? FVector2{ -1.f, 0.f } : FVector2{ 1.f, 0.f };
        _contactPoint = FVector2(_aPos.x + _aHalfSize.x * _normal.x, _bPos.y);
    }
    else  // Verticale
    {
        _normal = (_delta.y < 0.f) ? FVector2{ 0.f, -1.f } : FVector2{ 0.f,  1.f };
        _contactPoint = FVector2(_bPos.x, _aPos.y + _aHalfSize.y * _normal.y);
    }

    _aInfo.normal = _normal;
    _aInfo.penetration = FMath::MinVal(_sizeIntersection.x, _sizeIntersection.y);
    _aInfo.contactPoint = _contactPoint;
    // TODO contectPoint = _intersection.GetPosition();

    _aInfo.normal = _normal * -1;
    _aInfo.penetration = _aInfo.penetration;
    _aInfo.contactPoint = _contactPoint;

    return true;
}

bool Krampus::Physics::CircleToRect(const FVector2& _circlePos, const float& _radius, const FRect& _rect, const Angle& _rectRot, CollisionInfo& _circleInfo, CollisionInfo& _rectInfo)
{
    const FVector2 _rectPos = _rect.GetPosition();
    const FVector2 _localCircle = _circlePos - _rectPos;

    const float _cosR = FMath::Cos(-_rectRot);
    const float _sinR = FMath::Sin(-_rectRot);

    const FVector2 _rotatedCircle = FVector2(_localCircle.x * _cosR - _localCircle.y * _sinR,
        _localCircle.x * _sinR + _localCircle.y * _cosR);

    const FVector2 _halfSize = _rect.GetSize() * 0.5f;
    const FVector2 _closestPoint = FVector2(FMath::MaxVal(-_halfSize.x, FMath::MinVal(_rotatedCircle.x, _halfSize.x)),
        FMath::MaxVal(-_halfSize.y, FMath::MinVal(_rotatedCircle.y, _halfSize.y)));

    const FVector2 _delta = _rotatedCircle - _closestPoint;
    const float _distanceSquared = _delta.x * _delta.x + _delta.y * _delta.y;


    // HitInfos
    const float _distance = FMath::Sqrt(_distanceSquared);

    FVector2 _normalLocal;
    float _penetration;

    if (_distance > 0.f)
    {
        _normalLocal = _delta / _distance;
        _penetration = _radius - _distance;
    }
    else
    {
        _normalLocal = FVector2(1.f, 0.f);
        _penetration = _radius;
    }

    const FVector2 _normalWorld = FVector2(_normalLocal.x * _cosR - _normalLocal.y * _sinR,
        _normalLocal.x * _sinR + _normalLocal.y * _cosR);

    const FVector2 _contactPointWorld = FVector2(_closestPoint.x * _cosR - _closestPoint.y * _sinR,
        _closestPoint.x * _sinR + _closestPoint.y * _cosR) + _rectPos;

    // TODO strange response
    _circleInfo.normal = _normalWorld;
    _circleInfo.penetration = _penetration;
    _circleInfo.contactPoint = _contactPointWorld;

    _rectInfo.normal = _normalWorld * -1.0f;
    _rectInfo.penetration = _penetration;
    _rectInfo.contactPoint = _contactPointWorld;

    return _distanceSquared <= _radius * _radius;
}

void Krampus::Physics::GetAxes(const Angle& _rotation, FVector2 _axes[2])
{
	const float& _cos = FMath::Cos(_rotation);
	const float& _sin = FMath::Sin(_rotation);

	_axes[0] = FVector2(_cos, _sin);
	_axes[1] = FVector2(-_sin, _cos);
}

float Krampus::Physics::ProjectOBB(const FVector2& _size, const Angle& _rotation, const FVector2& _axis)
{
	FVector2 _axes[2];
	GetAxes(_rotation, _axes);

	const FVector2& _halfSize = _size / 2.0f;

	return
		_halfSize.x * FMath::Abs(_axis.Dot(_axes[0])) +
		_halfSize.y * FMath::Abs(_axis.Dot(_axes[1]));
}
