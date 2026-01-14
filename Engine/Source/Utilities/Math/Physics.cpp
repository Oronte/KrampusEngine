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
    _aInfo.contactPoint = CalculateContactPoint(_aRect, _aRot, _bRect, _bRot, _bestAxis); // TODO Contact Point not precise
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

    _aInfo.normal = _normal * -1;
    _aInfo.penetration = FMath::MinVal(_sizeIntersection.x, _sizeIntersection.y);
    _aInfo.contactPoint = _contactPoint; // TODO Contact Point not precise
    _aInfo.hit = true;

    _bInfo.normal = _normal;
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

bool Krampus::Physics::Contains(const FVector2& _point, const FVector2& _circlePos, const float& _radius, CollisionInfo& _info)
{
    const float _distance = _point.Distance(_circlePos);

    if (_distance > _radius) return false;

    const FVector2& _normal = _distance == 0.0f ? FVector2() :
        (_circlePos - _point) / -_distance;

    _info.normal = _normal;
    _info.penetration = _radius - _distance;
    _info.contactPoint = _point;
    _info.hit = true;

    return true;
}

bool Krampus::Physics::Contains(const FVector2& _point, const FVector2& _pos, const FVector2& _size, CollisionInfo& _info)
{
    const FVector2 _halfSize = _size * 0.5f;

    const float _minX = _pos.x - _halfSize.x;
    const float _maxX = _pos.x + _halfSize.x;
    const float _minY = _pos.y - _halfSize.y;
    const float _maxY = _pos.y + _halfSize.y;

    if (_point.x < _minX || _point.x > _maxX ||
        _point.y < _minY || _point.y > _maxY)
        return false;

    const float _left = _point.x - _minX;
    const float _right = _maxX - _point.x;
    const float _top = _point.y - _minY;
    const float _bottom = _maxY - _point.y;

    float _penetration = _left;
    FVector2 _normal(-1.f, 0.f);

    if (_right < _penetration)
    {
        _penetration = _right;
        _normal = FVector2(1.f, 0.f);
    }
    if (_top < _penetration)
    {
        _penetration = _top;
        _normal = FVector2(0.f, -1.f);
    }
    if (_bottom < _penetration)
    {
        _penetration = _bottom;
        _normal = FVector2(0.f, 1.f);
    }

    _info.normal = _normal;
    _info.penetration = _penetration;
    _info.contactPoint = _point;
    _info.hit = true;

    return true;
}

bool Krampus::Physics::Contains(const FVector2& _point, const FVector2& _pos, const FVector2& _size, const Angle& _rot, CollisionInfo& _info)
{
    const FVector2 _halfSize = _size * 0.5f;

    const float _cosA = FMath::Cos(_rot);
    const float _sinA = FMath::Sin(_rot);

    FVector2 _local;
    FVector2 _delta = _point - _pos;

    _local.x = _delta.x * _cosA + _delta.y * _sinA;
    _local.y = -_delta.x * _sinA + _delta.y * _cosA;

    if (_local.x < -_halfSize.x || _local.x > _halfSize.x ||
        _local.y < -_halfSize.y || _local.y > _halfSize.y)
        return false;

    const float _left = _local.x + _halfSize.x;
    const float _right = _halfSize.x - _local.x;
    const float _top = _local.y + _halfSize.y;
    const float _bottom = _halfSize.y - _local.y;

    float _penetration = _left;
    FVector2 _localNormal(-1.f, 0.f);

    if (_right < _penetration)
    {
        _penetration = _right;
        _localNormal = FVector2(1.f, 0.f);
    }
    if (_top < _penetration)
    {
        _penetration = _top;
        _localNormal = FVector2(0.f, -1.f);
    }
    if (_bottom < _penetration)
    {
        _penetration = _bottom;
        _localNormal = FVector2(0.f, 1.f);
    }

    FVector2 _worldNormal;
    _worldNormal.x = _localNormal.x * _cosA - _localNormal.y * _sinA;
    _worldNormal.y = _localNormal.x * _sinA + _localNormal.y * _cosA;

    _info.normal = _worldNormal;
    _info.penetration = _penetration;
    _info.contactPoint = _point;
    _info.hit = true;

    return true;
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

void Krampus::Physics::GetRectCorners(const FVector2& pos, const FVector2& size, const Angle& rot, FVector2 outCorners[4])
{
    FVector2 half = size * 0.5f;
    FVector2 local[4] = {
        FVector2(-half.x, -half.y),
        FVector2(half.x, -half.y),
        FVector2(half.x,  half.y),
        FVector2(-half.x,  half.y)
    };

    float c = FMath::Cos(rot);
    float s = FMath::Sin(rot);

    for (int i = 0; i < 4; ++i)
    {
        FVector2 r(
            local[i].x * c - local[i].y * s,
            local[i].x * s + local[i].y * c
        );
        outCorners[i] = pos + r;
    }
}

bool Krampus::Physics::ClipSegment(FVector2& p1, FVector2& p2, float min, float max, const FVector2& axis)
{
    float d1 = p1.Dot(axis) - min;
    float d2 = p2.Dot(axis) - min;

    if (d1 < 0.f && d2 < 0.f) return false;
    if (d1 > 0.f && d2 > 0.f) return false;

    if (d1 < 0.f)
        p1 = p1 + (p2 - p1) * (d1 / (d1 - d2));
    else if (d2 < 0.f)
        p2 = p1 + (p2 - p1) * (d1 / (d1 - d2));

    return true;
}

Krampus::FVector2 Krampus::Physics::CalculateContactPoint(const FRect& aRect, const Angle& aRot, const FRect& bRect, const Angle& bRot, const FVector2& normal)
{
    FVector2 aCorners[4], bCorners[4];
    GetRectCorners(aRect.GetPosition(), aRect.GetSize(), aRot, aCorners);
    GetRectCorners(bRect.GetPosition(), bRect.GetSize(), bRot, bCorners);

    // Determine reference face (rectangle whose face is most aligned with normal)
    FVector2 refAxis = normal;
    FVector2 refMin = aCorners[0], refMax = aCorners[0];

    for (int i = 1; i < 4; ++i)
    {
        float proj = aCorners[i].Dot(refAxis);
        float minProj = refMin.Dot(refAxis);
        float maxProj = refMax.Dot(refAxis);
        if (proj < minProj) refMin = aCorners[i];
        if (proj > maxProj) refMax = aCorners[i];
    }

    // Clip incident rectangle edges against reference face
    FVector2 contactSum(0, 0);
    int contactCount = 0;

    for (int i = 0; i < 4; ++i)
    {
        FVector2 p1 = bCorners[i];
        FVector2 p2 = bCorners[(i + 1) % 4];
        if (ClipSegment(p1, p2, refMin.Dot(refAxis), refMax.Dot(refAxis), refAxis))
        {
            contactSum += p1;
            contactCount++;
            contactSum += p2;
            contactCount++;
        }
    }

    if (contactCount == 0)
        return (aRect.GetPosition() + bRect.GetPosition()) * 0.5f; // fallback

    FVector2 contactPoint = contactSum / float(contactCount);

    // --- Special case: small rectangle side fully touches the reference face ---
    float aArea = aRect.GetSize().x * aRect.GetSize().y;
    float bArea = bRect.GetSize().x * bRect.GetSize().y;

    const FRect* smallRect = (aArea < bArea) ? &aRect : &bRect;
    Angle smallRot = (aArea < bArea) ? aRot : bRot;

    FVector2 smallCorners[4];
    GetRectCorners(smallRect->GetPosition(), smallRect->GetSize(), smallRot, smallCorners);

    // Find the side of the small rectangle most facing the normal
    int sideIndex = 0;
    float maxDot = -FLT_MAX;
    for (int i = 0; i < 4; ++i)
    {
        FVector2 edge = smallCorners[(i + 1) % 4] - smallCorners[i];
        FVector2 edgeNormal(-edge.y, edge.x);
        edgeNormal.Normalized();
        float dotVal = edgeNormal.Dot(normal);
        if (dotVal > maxDot)
        {
            maxDot = dotVal;
            sideIndex = i;
        }
    }

    // Check if the side is fully within reference face projection
    FVector2 sideP1 = smallCorners[sideIndex];
    FVector2 sideP2 = smallCorners[(sideIndex + 1) % 4];

    // Project the side onto the reference normal axis
    float p1Proj = sideP1.Dot(refAxis);
    float p2Proj = sideP2.Dot(refAxis);
    float refMinProj = refMin.Dot(refAxis);
    float refMaxProj = refMax.Dot(refAxis);
    const float epsilon = 0.001f;

    if ((p1Proj >= refMinProj - epsilon && p1Proj <= refMaxProj + epsilon) &&
        (p2Proj >= refMinProj - epsilon && p2Proj <= refMaxProj + epsilon))
    {
        // Use the midpoint of the side as contact point
        contactPoint = (sideP1 + sideP2) * 0.5f;
    }

    return contactPoint;
}
