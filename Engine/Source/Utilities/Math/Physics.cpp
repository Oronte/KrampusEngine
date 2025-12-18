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
	_aInfo.contactPoint = _bPos + _normal * (_bRadius - _bInfo.penetration / 2.0f);
	_aInfo.hit = true;

	_bInfo.normal = _aInfo.normal * -1.0f;
	_bInfo.penetration = _aInfo.penetration;
	_bInfo.contactPoint = _aInfo.contactPoint;
	_bInfo.hit = true;

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

    const float _minOffset = 0.0001f; // TODO better solution

    _aInfo.normal = _bestAxis;
    _aInfo.penetration = _minPenetration + _minOffset;
    _aInfo.contactPoint = (_aPos + _bPos) * 0.5f - _bestAxis * _minPenetration; // TODO Contact Point not precise
    _aInfo.hit = true;

    _bInfo.normal = _bestAxis * -1.0f;
    _bInfo.penetration = _aInfo.penetration;
    _bInfo.contactPoint = _aInfo.contactPoint;
    _bInfo.hit = true;

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
    _aInfo.contactPoint = _contactPoint; // TODO Contact Point not precise
    _aInfo.hit = true;

    _bInfo.normal = _normal * -1;
    _bInfo.penetration = _aInfo.penetration;
    _bInfo.contactPoint = _contactPoint;
    _bInfo.hit = true;

    return true;
}

bool Krampus::Physics::CircleToRect(const FVector2& _circlePos, const float& _radius, const FRect& _rect, const Angle& _rectRot, CollisionInfo& _circleInfo, CollisionInfo& _rectInfo)
{
    const FVector2 _rectPosition = _rect.GetPosition();
    const FVector2 _rectHalfSize = _rect.GetSize() * 0.5f;

    FVector2 _localPosition = _circlePos - _rectPosition;
    _localPosition = _localPosition.Rotated(-_rectRot);

    const FVector2 _closest = FVector2::Clamp(_localPosition, _rectHalfSize * -1.0f, _rectHalfSize);
    const FVector2 _delta = _localPosition - _closest;

    FVector2 _normalLocal;
    FVector2 _contactLocal;
    float _penetration;

    // Outside
    if (_delta.LengthSquared() > 0.f)
    {
        const float _dist = _delta.Length();
        if (_dist > _radius) return false;

        _normalLocal = _delta / _dist; 
        _penetration = _radius - _dist;
        _contactLocal = _closest;
    }
    // Inside
    else
    {
        const float _deltaX = _rectHalfSize.x - FMath::Abs(_localPosition.x);
        const float _deltaY = _rectHalfSize.y - FMath::Abs(_localPosition.y);

        if (_deltaX < _deltaY)
        {
            _normalLocal = FVector2((_localPosition.x > 0.f) ? 1.f : -1.f, 0.f);
            _penetration = _radius + _deltaX;
            _contactLocal = FVector2(_rectHalfSize.x * _normalLocal.x, _localPosition.y);
        }
        else
        {
            _normalLocal = FVector2(0.f, (_localPosition.y > 0.f) ? 1.f : -1.f);
            _penetration = _radius + _deltaY;
            _contactLocal = FVector2(_localPosition.x, _rectHalfSize.y * _normalLocal.y);
        }
    }
    const float _minOffset = 0.0001f; // TODO better solution
    _penetration += _minOffset;

    const FVector2 normalWorld = _normalLocal.Rotated(_rectRot);
    const FVector2 contactWorld = _contactLocal.Rotated(_rectRot) + _rectPosition;

    _circleInfo.normal = normalWorld;
    _circleInfo.penetration = _penetration;
    _circleInfo.contactPoint = contactWorld;
    _circleInfo.hit = true;

    _rectInfo.normal = normalWorld * -1.0f;
    _rectInfo.penetration = _penetration;
    _rectInfo.contactPoint = contactWorld;
    _rectInfo.hit = true;

    return true;
}

RaycastHitInfo Krampus::Physics::Raycast(const FVector2& _rayOrigin, const FVector2& _rayDir, const FRect& _rect, const float _rectRot)
{
    const FVector2 rayDir = _rayDir.Normalized();

    FVector2 localOrigin = rayOrigin - rect.GetPosition();
    float cosR = std::cos(-rectRot);
    float sinR = std::sin(-rectRot);

    FVector2 ro(
        localOrigin.x * cosR - localOrigin.y * sinR,
        localOrigin.x * sinR + localOrigin.y * cosR
    );

    FVector2 rd(
        rayDir.x * cosR - rayDir.y * sinR,
        rayDir.x * sinR + rayDir.y * cosR
    );

    FVector2 half = rect.GetSize() * 0.5f;

    // --- Slab method (AABB) ---
    float tMin = -FLT_MAX;
    float tMax = FLT_MAX;

    FVector2 normal;

    for (int i = 0; i < 2; ++i)
    {
        float originComp = (i == 0) ? ro.x : ro.y;
        float dirComp = (i == 0) ? rd.x : rd.y;
        float min = (i == 0) ? -half.x : -half.y;
        float max = (i == 0) ? half.x : half.y;

        if (std::abs(dirComp) < 1e-6f)
        {
            // Rayon parallèle à cette face
            if (originComp < min || originComp > max)
                return {}; // Pas d'intersection
        }
        else
        {
            float t1 = (min - originComp) / dirComp;
            float t2 = (max - originComp) / dirComp;

            if (t1 > t2) std::swap(t1, t2);

            if (t1 > tMin)
            {
                tMin = t1;
                // normale correspondante
                normal = (i == 0) ? FVector2((dirComp > 0) ? -1.f : 1.f, 0.f)
                    : FVector2(0.f, (dirComp > 0) ? -1.f : 1.f);
            }

            tMax = std::min(tMax, t2);

            if (tMin > tMax) return {}; // Pas d'intersection
        }
    }

    if (tMin < 0.f) return {}; // L'intersection est derrière le rayon

    // --- Calcul du point de contact en espace monde ---
    FVector2 localHit = ro + rd * tMin;

    FVector2 hitPoint(
        localHit.x * cosR - localHit.y * sinR,
        localHit.x * sinR + localHit.y * cosR
    );
    hitPoint += rect.GetPosition();

    RaycastHit result;
    result.hit = true;
    result.point = hitPoint;
    result.normal = Rotate(normal, rectRot); // normale en monde
    result.distance = tMin;

    return result;
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
