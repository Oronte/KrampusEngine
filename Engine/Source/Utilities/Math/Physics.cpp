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
    const FVector2& aPos = _aRect.GetPosition();
    const FVector2& aSize = _aRect.GetSize();
    const FVector2& bPos = _bRect.GetPosition();
    const FVector2& bSize = _bRect.GetSize();

    FVector2 axesA[2], axesB[2];
    GetAxes(_aRot, axesA);
    GetAxes(_bRot, axesB);

    FVector2 delta = bPos - aPos;
    float minPen = FLT_MAX;
    FVector2 normal;

    // Projection sur un axe et test de chevauchement
    auto testOverlap = [&](const FVector2& axis) -> bool
        {
            float projA = ProjectOBB(aSize, _aRot, axis);
            float projB = ProjectOBB(bSize, _bRot, axis);
            float dist = FMath::Abs(delta.Dot(axis));
            float overlap = projA + projB - dist;
            if (overlap <= 0.0f) return false;
            if (overlap < minPen) { normal = axis; minPen = overlap; }
            return true;
        };

    if (!testOverlap(axesA[0])) return false;
    if (!testOverlap(axesA[1])) return false;
    if (!testOverlap(axesB[0])) return false;
    if (!testOverlap(axesB[1])) return false;

    // sens de la normale (de A vers B)
    if (delta.Dot(normal) < 0.0f) normal *= -1.0f;

    _aInfo.normal = normal * -1;
    _aInfo.penetration = minPen / 2;
    _bInfo.normal = normal;
    _bInfo.penetration = minPen / 2;

    std::vector<FVector2> contacts;

    // Coins de A tournés correctement
    FVector2 halfA = aSize / 2.0f;
    FVector2 cornersA[4] = {
        aPos + FVector2(-halfA.x, -halfA.y).Rotated(_aRot),
        aPos + FVector2(halfA.x, -halfA.y).Rotated(_aRot),
        aPos + FVector2(halfA.x,  halfA.y).Rotated(_aRot),
        aPos + FVector2(-halfA.x,  halfA.y).Rotated(_aRot)
    };

    // Coins de B tournés correctement
    FVector2 halfB = bSize / 2.0f;
    FVector2 cornersB[4] = {
        bPos + FVector2(-halfB.x, -halfB.y).Rotated(_bRot),
        bPos + FVector2(halfB.x, -halfB.y).Rotated(_bRot),
        bPos + FVector2(halfB.x,  halfB.y).Rotated(_bRot),
        bPos + FVector2(-halfB.x,  halfB.y).Rotated(_bRot)
    };

    // Test point dans OBB
    auto pointInOBB = [](const FVector2& p, const FVector2& pos, const FVector2& size, const FVector2 axes[2])
        {
            FVector2 d = p - pos;
            for (int i = 0; i < 2; i++)
            {
                float proj = d.Dot(axes[i]);
                float half = (i == 0 ? size.x : size.y) / 2.0f;
                if (proj < -half || proj > half) return false;
            }
            return true;
        };

    // Coins de A dans B
    for (int i = 0; i < 4; i++)
        if (pointInOBB(cornersA[i], bPos, bSize, axesB))
            contacts.push_back(cornersA[i]);

    // Coins de B dans A
    for (int i = 0; i < 4; i++)
        if (pointInOBB(cornersB[i], aPos, aSize, axesA))
            contacts.push_back(cornersB[i]);

    // filtrer doublons
    std::vector<FVector2> filtered;
    for (auto& p : contacts)
    {
        bool unique = true;
        for (auto& f : filtered)
            if (FMath::Abs(p.x - f.x) < 1e-4f && FMath::Abs(p.y - f.y) < 1e-4f)
            {
                unique = false; break;
            }
        if (unique) filtered.push_back(p);
    }

    _aInfo.contacts = filtered;
    _bInfo.contacts = filtered;

    // POINT DE CONTACT : choisir le plus profond selon la normale
    FVector2 contactPoint = filtered.empty() ? FVector2{ 0,0 } : filtered[0];
    float maxDepth = -FLT_MAX;
    for (auto& p : filtered)
    {
        float depth = (p - aPos).Dot(normal);
        if (depth > maxDepth)
        {
            maxDepth = depth;
            contactPoint = p;
        }
    }

    _aInfo.contactPoint = contactPoint + _aInfo.normal * _aInfo.penetration;
    _bInfo.contactPoint = contactPoint + _bInfo.normal * _bInfo.penetration;

    _aInfo.hit = true;
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
