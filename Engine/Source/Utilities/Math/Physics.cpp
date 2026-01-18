#include "Physics.h"

bool Krampus::Physics::CircleToCircle(const FVector2& _aPos, const float& _aRadius, const FVector2& _bPos, const float& _bRadius, CollisionInfo& _aInfo, CollisionInfo& _bInfo)
{
    _aInfo = _bInfo = CollisionInfo();

    FVector2 delta = _bPos - _aPos;
    float distSq = delta.LengthSquared();
    float radiusSum = _aRadius + _bRadius;

    if (distSq > radiusSum * radiusSum)
        return false;

    float dist = FMath::Sqrt(distSq);

    FVector2 normal;
    if (dist > FMath::epsilon)
        normal = delta / dist;
    else
        normal = FVector2(0.f, -1.f);

    float penetration = radiusSum - dist;

    FVector2 contact = _aPos + normal * (_aRadius - penetration * 0.5f);

    _aInfo.normal = normal * -1.0f;
    _aInfo.penetration = penetration;
    _aInfo.avrageContactPoint = contact;
    _aInfo.hit = true;

    _bInfo.normal = normal;
    _bInfo.penetration = penetration;
    _bInfo.avrageContactPoint = contact;
    _bInfo.hit = true;

    _aInfo.contacts = _bInfo.contacts =
        CircleCircleIntersections(_aPos, _aRadius, _bPos, _bRadius);

    if (_aInfo.contacts.empty())
        _aInfo.contacts = _bInfo.contacts = { contact };

    return true;
}

bool Krampus::Physics::RectToRectOBB(const FRect& _aRect, const Angle& _aRot, const FRect& _bRect, const Angle& _bRot, CollisionInfo& _aInfo, CollisionInfo& _bInfo)
{
    _aInfo = _bInfo = CollisionInfo();

    const FVector2& _aPos = _aRect.GetPosition();
    const FVector2& _aSize = _aRect.GetSize();
    const FVector2& _bPos = _bRect.GetPosition();
    const FVector2& _bSize = _bRect.GetSize();

    FVector2 _axesA[2], _axesB[2];
    GetAxes(_aRot, _axesA);
    GetAxes(_bRot, _axesB);

    const FVector2& _delta = _bPos - _aPos;
    float _minPen = FLT_MAX;
    FVector2 _normal;

    auto TestOverlap = [&](FVector2 _axis) -> bool
        {
            float _lenSq = _axis.LengthSquared();
            if (_lenSq < 1e-8f)
                return true;

            _axis /= FMath::Sqrt(_lenSq);

            const float _projA = ProjectOBB(_aSize, _aRot, _axis);
            const float _projB = ProjectOBB(_bSize, _bRot, _axis);
            const float _dist = FMath::Abs(_delta.Dot(_axis));

            const float _overlap = _projA + _projB - _dist;
            if (_overlap <= 0.0f)
                return false;

            if (_overlap < _minPen)
            {
                _minPen = _overlap;
                _normal = _axis;
            }
            return true;
        };

    if (!TestOverlap(_axesA[0])) return false;
    if (!TestOverlap(_axesA[1])) return false;
    if (!TestOverlap(_axesB[0])) return false;
    if (!TestOverlap(_axesB[1])) return false;

    if (_delta.Dot(_normal) < 0.0f) _normal *= -1.0f;

    const float _penetration = _minPen / 2.0f;

    _aInfo.normal = _normal * -1.0f;
    _aInfo.penetration = _penetration;
    _bInfo.normal = _normal;
    _bInfo.penetration = _penetration;

    _aInfo.hit = true;
    _bInfo.hit = true;

    std::vector<FVector2> _contacts = CalculateOBBContactPoints(_aRect, _aRot, _bRect, _bRot);
    if (_contacts.empty())
    {
        FVector2 _mid = (_aPos + _bPos) * 0.5f;
        _contacts.push_back(_mid);
    }

    _aInfo.contacts = _bInfo.contacts = _contacts;
    _aInfo.avrageContactPoint = _bInfo.avrageContactPoint = ComputeAverageContactPoint(_contacts);

    return true;
}

bool Krampus::Physics::RectToRectAABB(const FRect& _aRect, const FRect& _bRect, CollisionInfo& _aInfo, CollisionInfo& _bInfo)
{
    _aInfo = _bInfo = CollisionInfo();

    std::optional<FRect> _result = _aRect.FindIntersection(_bRect);

    if (!_result.has_value()) return false;

    const FRect& _intersection = _result.value();

    const FVector2& _sizeIntersection = _intersection.GetSize();

    const FVector2& _aPos = _aRect.GetPosition();
    const FVector2& _bPos = _bRect.GetPosition();

    const FVector2& _aHalfSize = _aRect.GetSize() * 0.5f;
    const FVector2& _bHalfSize = _bRect.GetSize() * 0.5f;

    FVector2 _normal;

    const FVector2& _delta = _bPos - _aPos;

    if (_sizeIntersection.x < _sizeIntersection.y) // Horizontale
    {
        _normal = (_delta.x < 0.f) ? FVector2{ -1.f, 0.f } : FVector2{ 1.f, 0.f };
    }
    else  // Verticale
    {
        _normal = (_delta.y < 0.f) ? FVector2{ 0.f, -1.f } : FVector2{ 0.f,  1.f };
    }

    const FVector2& _contactPoint = FVector2(
        (_aPos.x - _aHalfSize.x + _bPos.x - _bHalfSize.x) * 0.5f,
        (_aPos.y - _aHalfSize.y + _bPos.y - _bHalfSize.y) * 0.5f);

    _aInfo.normal = _normal * -1;
    _aInfo.penetration = FMath::MinVal(_sizeIntersection.x, _sizeIntersection.y);
    _aInfo.avrageContactPoint = _contactPoint; 
    _aInfo.hit = true;

    _bInfo.normal = _normal;
    _bInfo.penetration = _aInfo.penetration;
    _bInfo.avrageContactPoint = _contactPoint;
    _bInfo.hit = true;

    _aInfo.contacts = _bInfo.contacts = { _contactPoint };

    return true;
}

bool Krampus::Physics::CircleToRect(const FVector2& _circlePos, const float& _radius, const FRect& _rect, const Angle& _rectRot, CollisionInfo& _circleInfo, CollisionInfo& _rectInfo)
{
    _circleInfo = _rectInfo = CollisionInfo();

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
    if (_delta.LengthSquared() > FMath::epsilon)
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

    const FVector2 normalWorld = _normalLocal.Rotated(_rectRot);
    const FVector2 contactWorld = _contactLocal.Rotated(_rectRot) + _rectPosition;

    _circleInfo.normal = normalWorld;
    _circleInfo.penetration = _penetration;
    _circleInfo.avrageContactPoint = contactWorld;
    _circleInfo.hit = true;

    _rectInfo.normal = normalWorld * -1.0f;
    _rectInfo.penetration = _penetration;
    _rectInfo.avrageContactPoint = contactWorld;
    _rectInfo.hit = true;

    _circleInfo.contacts = _rectInfo.contacts = CircleToRectContacts(_circlePos, _radius, _rect, _rectRot);
    if (_circleInfo.contacts.empty())
        _circleInfo.contacts = _rectInfo.contacts = { contactWorld };

    return true;
}

bool Krampus::Physics::Contains(const FVector2& _point, const FVector2& _circlePos, const float& _radius, CollisionInfo& _info)
{
    _info = CollisionInfo();

    const float _distance = _point.Distance(_circlePos);

    if (_distance > _radius) return false;

    const FVector2& _normal = _distance == 0.0f ? FVector2() :
        (_circlePos - _point) / -_distance;

    _info.normal = _normal;
    _info.penetration = _radius - _distance;
    _info.avrageContactPoint = _point;
    _info.contacts.push_back(_point);
    _info.hit = true;

    return true;
}

bool Krampus::Physics::Contains(const FVector2& _point, const FVector2& _pos, const FVector2& _size, CollisionInfo& _info)
{
    _info = CollisionInfo();

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
    _info.avrageContactPoint = _point;
    _info.contacts.push_back(_point);
    _info.hit = true;

    return true;
}

bool Krampus::Physics::Contains(const FVector2& _point, const FVector2& _pos, const FVector2& _size, const Angle& _rot, CollisionInfo& _info)
{
    _info = CollisionInfo();

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
    _info.avrageContactPoint = _point;
    _info.contacts.push_back(_point);
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

std::array<Krampus::FVector2, 4> Krampus::Physics::GetCorners(const FRect& _rect, const Angle& _rot)
{
    const FVector2& _pos = _rect.GetPosition();
    const FVector2 _size = _rect.GetSize() * 0.5f;
    FVector2 _axes[2];
    GetAxes(_rot, _axes);

    return {
        _pos + _axes[0] * _size.x + _axes[1] * _size.y,
        _pos - _axes[0] * _size.x + _axes[1] * _size.y,
        _pos - _axes[0] * _size.x - _axes[1] * _size.y,
        _pos + _axes[0] * _size.x - _axes[1] * _size.y
    };
}

bool Krampus::Physics::SegmentIntersect(const FVector2& _p, const FVector2& _r, const FVector2& _q, const FVector2& _s, FVector2& _outPoint)
{
    const float _rxs = _r.Cross(_s);

    if (FMath::Abs(_rxs) < FMath::epsilon)
        return false;

    FVector2 _qp = _q - _p;
    float _t = _qp.Cross(_s) / _rxs;
    float _u = _qp.Cross(_r) / _rxs;

    if (_t >= 0.f && _t <= 1.f && _u >= 0.f && _u <= 1.f)
    {
        _outPoint = _p + _r * _t;
        return true;
    }
    return false;
}

std::vector<Krampus::FVector2> Krampus::Physics::CalculateOBBContactPoints(const FRect& _aRect, const Angle& _aRot, const FRect& _bRect, const Angle& _bRot)
{
    std::vector<FVector2> _contacts;

    auto _aCorners = GetCorners(_aRect, _aRot);
    auto _bCorners = GetCorners(_bRect, _bRot);

    std::array<std::pair<FVector2, FVector2>, 4> _aEdges = { {
        {_aCorners[0], _aCorners[1]},
        {_aCorners[1], _aCorners[2]},
        {_aCorners[2], _aCorners[3]},
        {_aCorners[3], _aCorners[0]}
    } };
    std::array<std::pair<FVector2, FVector2>, 4> _bEdges = { {
        {_bCorners[0], _bCorners[1]},
        {_bCorners[1], _bCorners[2]},
        {_bCorners[2], _bCorners[3]},
        {_bCorners[3], _bCorners[0]}
    } };

    for (auto& _aE : _aEdges)
        for (auto& _bE : _bEdges)
        {
            FVector2 _pt;
            if (SegmentIntersect(_aE.first, _aE.second - _aE.first,
                _bE.first, _bE.second - _bE.first, _pt))
                _contacts.push_back(_pt);
        }

    return _contacts;
}

Krampus::FVector2 Krampus::Physics::ComputeAverageContactPoint(const std::vector<FVector2>& _contacts)
{
    if (_contacts.empty())
        return FVector2::Zero();

    FVector2 _sum;
    for (const FVector2& _p : _contacts)
        _sum += _p;

    return _sum / CAST(float, _contacts.size());
}

std::vector<Krampus::FVector2> Krampus::Physics::CircleToRectContacts(const FVector2& _circlePos, float _radius, const FRect& _rect, float _rectRot)
{
    std::vector<FVector2> _contacts;

    // Cercle en repère local
    FVector2 _localCircle = (_circlePos - _rect.GetPosition()).Rotated(-_rectRot);
    FVector2 hw = _rect.GetSize() * 0.5f;

    // Les 4 segments du rectangle (en local)
    FVector2 corners[4] = {
        {-hw.x, -hw.y}, {hw.x, -hw.y}, {hw.x, hw.y}, {-hw.x, hw.y}
    };

    for (int i = 0; i < 4; ++i)
    {
        FVector2 p0 = corners[i];
        FVector2 p1 = corners[(i + 1) % 4];

        FVector2 d = p1 - p0;
        FVector2 f = p0 - _localCircle;

        float a = d.Dot(d);
        if (a < FMath::epsilon)
            continue;

        float b = 2.f * f.Dot(d);
        float c = f.Dot(f) - _radius * _radius;

        float discriminant = b * b - 4.f * a * c;
        if (discriminant < 0.f)
            continue;

        discriminant = FMath::Sqrt(discriminant);

        float inv2a = 1.f / (2.f * a);
        float t1 = (-b - discriminant) * inv2a;
        float t2 = (-b + discriminant) * inv2a;

        if (t1 >= 0.f && t1 <= 1.f)
            _contacts.push_back(p0 + d * t1);
        if (t2 >= 0.f && t2 <= 1.f)
            _contacts.push_back(p0 + d * t2);
    }

    // Transformer en coordonnées monde
    for (auto& pt : _contacts) pt = pt.Rotated(_rectRot) + _rect.GetPosition();

    return _contacts;
}

std::vector<Krampus::FVector2> Krampus::Physics::CircleCircleIntersections(const FVector2& _c1, float _r1, const FVector2& _c2, float _r2)
{
    std::vector<FVector2> contacts;

    FVector2 v = _c2 - _c1;
    float d = v.Length();

    // Pas d'intersection
    if (d > _r1 + _r2 || d < FMath::Abs(_r1 - _r2) || d < FMath::epsilon) return contacts;

    float a = (_r1 * _r1 - _r2 * _r2 + d * d) / (2 * d);
    FVector2 p0 = _c1 + v * (a / d);

    float h = FMath::Sqrt(_r1 * _r1 - a * a);

    // Perpendiculaire
    FVector2 perp = FVector2(-v.y, v.x) / d;

    contacts.push_back(p0 + perp * h);
    contacts.push_back(p0 - perp * h);

    return contacts;
}