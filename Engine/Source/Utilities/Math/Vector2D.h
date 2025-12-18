#pragma once
#include "Math.h"
#include "Utilities/System/Printable.h"

namespace Krampus
{

    template<typename T>
    struct Vector2D : public IPrintable
    {
        static_assert(std::is_arithmetic_v<T>, "Vector2D<T> requires an arithmetic type T");

        using CalcType = std::common_type_t<T, float>;
        using Math = Math<CalcType>;

        T x;
        T y;

        constexpr Vector2D() noexcept
        {
            x = y = CAST(T, 0.0f);
        }
        constexpr Vector2D(T _x, T _y) noexcept
        {
            x = _x;
            y = _y;
        }
        explicit constexpr Vector2D(T _value) noexcept
        {
            x = y = _value;
        }

        template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
        Vector2D(const sf::Vector2<U>& _vector) noexcept
        {
            x = CAST(T, _vector.x);
            y = CAST(T, _vector.y);
        }
        template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
        Vector2D(const Vector2D<U>& _vector) noexcept
        {
            x = CAST(T, _vector.x);
            y = CAST(T, _vector.y);
        }

        static INLINE constexpr Vector2D<T> Up() noexcept
            requires std::is_signed_v<T>
        { 
            return Vector2D<T>(CAST(T, 0), CAST(T, -1));
        };
        static INLINE constexpr Vector2D<T> Down() noexcept
        { 
            return Vector2D<T>(CAST(T, 0), CAST(T, 1));
        };
        static INLINE constexpr Vector2D<T> Right() noexcept
        { 
            return Vector2D<T>(CAST(T, 1), CAST(T, 0));
        };
        static INLINE constexpr Vector2D<T> Left() noexcept
            requires std::is_signed_v<T>
        { 
            return Vector2D<T>(CAST(T, -1), CAST(T, 0));
        };
        static INLINE constexpr Vector2D<T> Zero() noexcept
        { 
            return Vector2D<T>(CAST(T, 0), CAST(T, 0));
        };
        static INLINE constexpr Vector2D<T> One() noexcept
        { 
            return Vector2D<T>(CAST(T, 1), CAST(T, 1));
        };

        INLINE NO_DISCARD CalcType LengthSquared() const noexcept 
        { 
            return CAST(CalcType, x) * CAST(CalcType, x) + CAST(CalcType, y) * CAST(CalcType, y);
        }
        INLINE NO_DISCARD CalcType Length() const noexcept 
        { 
            return Math::Sqrt(LengthSquared());
        }

        INLINE NO_DISCARD Vector2D<CalcType> Normalized(const CalcType& _fallback = 0.0) const noexcept
        {
            CalcType _length = Length();

            if (_length <= Math::epsilon) return Vector2D<CalcType>(_fallback, _fallback);

            return Vector2D<CalcType>(CAST(CalcType, x) / _length, CAST(CalcType, y) / _length);
        }

        INLINE NO_DISCARD void Clamp(const Vector2D& _min, const Vector2D& _max)
        {
            x = FMath::Clamp(x, _min.x, _max.x);
            y = FMath::Clamp(y, _min.y, _max.y);
        }

        static INLINE NO_DISCARD Vector2D<CalcType> Clamp(const Vector2D& _vector, const Vector2D& _min, const Vector2D& _max)
        {
            return Vector2D<CalcType>(FMath::Clamp(_vector.x, _min.x, _max.x), FMath::Clamp(_vector.y, _min.y, _max.y));
        }

        INLINE NO_DISCARD CalcType Dot(const Vector2D& _other) const noexcept
        {
            return CAST(CalcType, x) * CAST(CalcType, _other.x) + CAST(CalcType, y) * CAST(CalcType, _other.y);
        }

        INLINE NO_DISCARD CalcType Cross(const Vector2D& _other) const noexcept
        {
            return CAST(CalcType, x) * CAST(CalcType, _other.y) - CAST(CalcType, y) * CAST(CalcType, _other.x);
        }

        INLINE NO_DISCARD Vector2D Perp() const noexcept 
        { 
            return Vector2D(CAST(T, -y), CAST(T, x));
        }

        INLINE NO_DISCARD CalcType DistanceSquared(const Vector2D& _other) const noexcept 
        { 
            return (*this - _other).LengthSquared();
        }
        INLINE NO_DISCARD CalcType Distance(const Vector2D& _other) const noexcept 
        { 
            return Math::Sqrt(DistanceSquared(_other));
        }

        INLINE NO_DISCARD Vector2D<CalcType> ClampMagnitude(CalcType _maxLength) const noexcept
        {
            CalcType _lengthSquared = CAST(CalcType, LengthSquared());
            CalcType _maxLengthSquared = _maxLength * _maxLength;

            if (_lengthSquared <= _maxLengthSquared)
                return Vector2D<CalcType>(CAST(CalcType, x), CAST(CalcType, y));

            CalcType _length = Math::Sqrt(_lengthSquared);
            return Vector2D<CalcType>(CAST(CalcType, x) * (_maxLength / _length), CAST(CalcType, y) * (_maxLength / _length));
        }

        INLINE NO_DISCARD Vector2D<CalcType> MoveTowards(const Vector2D& _target, CalcType _maxDelta) const noexcept
        {

            Vector2D<CalcType> _current = Vector2D(CAST(CalcType, x), CAST(CalcType, y));
            Vector2D<CalcType> _tgt = Vector2D(CAST(CalcType, _target.x), CAST(CalcType, _target.y));
            Vector2D<CalcType> _toTarget = _tgt - _current;
            CalcType _dist = _toTarget.Length();

            if (_dist <= CAST(CalcType, _maxDelta) || _dist <= Math::epsilon) return _tgt;

            return _current + _toTarget * (CAST(CalcType, _maxDelta) / _dist);
        }

        INLINE NO_DISCARD Vector2D<CalcType> Reflect(const Vector2D& _normal) const noexcept
        {
            Vector2D<CalcType> _incident = Vector2D(CAST(CalcType, x), CAST(CalcType, y));
            Vector2D<CalcType> _calcTypeNormal = Vector2D(CAST(CalcType, _normal.x), CAST(CalcType, _normal.y));
            CalcType _dotIncidentNormal = _incident.Dot(_calcTypeNormal);
            return _incident - _calcTypeNormal * (CAST(CalcType, 2) * _dotIncidentNormal);
        }

        INLINE NO_DISCARD Vector2D<CalcType> Project(const Vector2D& _axis) const noexcept
        {
            Vector2D<CalcType> _vector{ CAST(CalcType, x), CAST(CalcType, y) };
            Vector2D<CalcType> _direction{ CAST(CalcType, _axis.x), CAST(CalcType, _axis.y) };
            CalcType _denominator = _direction.Dot(_direction);

            if (_denominator <= Math::epsilon) return Vector2D<CalcType>(CAST(CalcType, 0), CAST(CalcType, 0));

            CalcType _projectionScale = _vector.Dot(_direction) / _denominator;
            return _direction * _projectionScale;
        }

        INLINE NO_DISCARD CalcType AtanToRadian() const noexcept 
        { 
            return Math::Atan2(CAST(CalcType, y), CAST(CalcType, x));
        }

        INLINE NO_DISCARD CalcType AngleBetweenRadians(const Vector2D& _other) const noexcept
        {
            CalcType _thisAngle = AtanToRadian();
            CalcType _otherAngle = _other.AtanToRadian();

            CalcType _deltaAngle = _otherAngle - _thisAngle;

            while (_deltaAngle <= -Math::pi) _deltaAngle += Math::tau;
            while (_deltaAngle > Math::pi)  _deltaAngle -= Math::tau;

            return _deltaAngle;
        }

        INLINE NO_DISCARD CalcType SignedAngleDeg(const Vector2D& _to) const noexcept
        {
            return Math::RadToDeg(AngleBetweenRadians(_to));
        }

        INLINE NO_DISCARD Vector2D<CalcType> Rotated(const CalcType& _angleRad) const noexcept
        {
            CalcType _cos = Math::Cos(_angleRad);
            CalcType _sin = Math::Sin(_angleRad);

            CalcType _x = CAST(CalcType, x);
            CalcType _y = CAST(CalcType, y);

            return Vector2D<CalcType>(_x * _cos - _y * _sin,
                _x * _sin + _y * _cos);
        }

        INLINE NO_DISCARD Vector2D<CalcType> RotatedDeg(const CalcType& _angleDeg) const noexcept
        {
            return Rotated(Math::DegToRad(_angleDeg));
        }

        INLINE NO_DISCARD Vector2D<CalcType> RotateAround(const Vector2D& _pivot, CalcType _angleRad) const noexcept
        {
            Vector2D<CalcType> _calcTypePivot = Vector2D(CAST(CalcType, _pivot.x), CAST(CalcType, _pivot.y));
            Vector2D<CalcType> _self = Vector2D(CAST(CalcType, x), CAST(CalcType, y));
            return _calcTypePivot + (_self - _calcTypePivot).Rotated(_angleRad);
        }

        INLINE NO_DISCARD Vector2D<CalcType> Lerp(const Vector2D& _b, CalcType _time) const noexcept
        {
            CalcType _x = Math::Lerp(CAST(CalcType, x), CAST(CalcType, _b.x), _time);
            CalcType _y = Math::Lerp(CAST(CalcType, y), CAST(CalcType, _b.y), _time);
            return Vector2D<CalcType>(_x, _y);
        }
        INLINE static Vector2D<CalcType> Lerp(const Vector2D& _a, const Vector2D& _b, CalcType _time) noexcept
        {
            CalcType _x = Math::Lerp(CAST(CalcType, _a.x), CAST(CalcType, _b.x), _time);
            CalcType _y = Math::Lerp(CAST(CalcType, _a.y), CAST(CalcType, _b.y), _time);
            return Vector2D<CalcType>(_x, _y);
        }
        
        INLINE NO_DISCARD Vector2D<CalcType> ClosestPointOnSegment(const Vector2D& _a, const Vector2D& _b) const noexcept
        {
            Vector2D<CalcType> _calcTypeA = Vector2D(CAST(CalcType, _a.x), CAST(CalcType, _a.y));
            Vector2D<CalcType> _calcTypeB = Vector2D(CAST(CalcType, _b.x), CAST(CalcType, _b.y));
            Vector2D<CalcType> _thisVector = Vector2D(CAST(CalcType, x), CAST(CalcType, y));
            Vector2D<CalcType> _ab = _calcTypeB - _calcTypeA;
            CalcType _abSquared = _ab.LengthSquared();

            if (_abSquared <= Math::epsilon) return _calcTypeA;

            CalcType _t = (_thisVector - _calcTypeA).Dot(_ab) / _abSquared;
            Math::Saturate(_t);
            return _calcTypeA + _ab * _t;
        }

        INLINE NO_DISCARD CalcType DistanceToSegment(const Vector2D& _a, const Vector2D& _b) const noexcept
        {
            Vector2D<CalcType> _closestPoint = ClosestPointOnSegment(_a, _b);
            Vector2D<CalcType> _thisVector{ CAST(CalcType, x), CAST(CalcType, y) };
            return (_thisVector - _closestPoint).Length();
        }

        INLINE NO_DISCARD CalcType SignedDistanceToLine(const Vector2D& _a, const Vector2D& _b) const noexcept
        {
            Vector2D<CalcType> _calcTypeA{ CAST(CalcType, _a.x), CAST(CalcType, _a.y) };
            Vector2D<CalcType> _calcTypeB{ CAST(CalcType, _b.x), CAST(CalcType, _b.y) };
            Vector2D<CalcType> _thisVector{ CAST(CalcType, x), CAST(CalcType, y) };
            Vector2D<CalcType> _ab = _calcTypeB - _calcTypeA;
            CalcType _denominator = Math::Sqrt(_ab.LengthSquared());

            if (_denominator <= Math::epsilon) 
                return CAST(CalcType, 0);

            return CAST(CalcType, _ab.Cross(_thisVector - _calcTypeA) / _denominator);
        }

        INLINE bool IsOnSegment(const Vector2D& _a, const Vector2D& _b, CalcType _epsVal = Math::epsilon) const noexcept
        {
            return DistanceToSegment(_a, _b) <= _epsVal;
        }


        INLINE static bool ComputeLineIntersection(const Vector2D& _lineAOrigin, const Vector2D& _lineADirection,
            const Vector2D& _lineBOrigin, const Vector2D& _lineBDirection,
            CalcType& _outLineAParam, CalcType& _outLineBParam) noexcept
        {
            Vector2D<CalcType> _pointA = Vector2D(CAST(CalcType, _lineAOrigin.x), CAST(CalcType, _lineAOrigin.y));
            Vector2D<CalcType> _dirA = Vector2D(CAST(CalcType, _lineADirection.x), CAST(CalcType, _lineADirection.y));

            Vector2D<CalcType> _pointB = Vector2D(CAST(CalcType, _lineBOrigin.x), CAST(CalcType, _lineBOrigin.y));
            Vector2D<CalcType> _dirB = Vector2D(CAST(CalcType, _lineBDirection.x), CAST(CalcType, _lineBDirection.y));

            CalcType _crossDirADirB = _dirA.Cross(_dirB);
            Vector2D<CalcType> _deltaAB = _pointB - _pointA;
            CalcType _crossDeltaABDirA = _deltaAB.Cross(_dirA);

            if (Math::Abs(_crossDirADirB) <= Math::epsilon)
                return false;

            _outLineAParam = CAST(CalcType, _deltaAB.Cross(_dirB) / _crossDirADirB);
            _outLineBParam = CAST(CalcType, _crossDeltaABDirA / _crossDirADirB);
            return true;
        }

        INLINE static bool ComputeSegmentIntersection(const Vector2D& _segmentAStart, const Vector2D& _segmentADirection,
            const Vector2D& _segmentBStart, const Vector2D& _segmentBDirection,
            Vector2D<double>* _intersectionPoint = nullptr) noexcept
        {
            CalcType _t, _u;
            if (!ComputeLineIntersection(_segmentAStart, _segmentADirection,
                _segmentBStart, _segmentBDirection, _t, _u))
                return false;

            if (_t < 0.0 || _t > 1.0 || _u < 0.0 || _u > 1.0)
                return false;

            if (_intersectionPoint)
            {
                _intersectionPoint->x = CAST(double, _segmentAStart.x) + CAST(double, _segmentADirection.x) * _t;
                _intersectionPoint->y = CAST(double, _segmentAStart.y) + CAST(double, _segmentADirection.y) * _t;
            }
            return true;
        }

        INLINE static bool FindSegmentIntersection(const Vector2D& _segmentAStart, const Vector2D& _segmentAEnd,
            const Vector2D& _segmentBStart, const Vector2D& _segmentBEnd,
            Vector2D<double>* _intersectionPoint = nullptr) noexcept
        {
            Vector2D _directionA = _segmentAEnd - _segmentAStart;
            Vector2D _directionB = _segmentBEnd - _segmentBStart;

            return ComputeSegmentIntersection(_segmentAStart, _directionA,
                _segmentBStart, _directionB,
                _intersectionPoint);
        }


        INLINE virtual std::string ToString() const override
        {
            return std::format("X : {}, Y : {}", x, y);
        }

        #pragma region Operators

        operator sf::Vector2<T>() const noexcept
        {
            return { x, y };
        }

        constexpr Vector2D operator + (const Vector2D& _other) const noexcept 
        { 
            return Vector2D(CAST(T, x + _other.x), CAST(T, y + _other.y));
        }
        constexpr Vector2D operator - (const Vector2D& _other) const noexcept 
        { 
            return Vector2D(CAST(T, x - _other.x), CAST(T, y - _other.y));
        }
        constexpr Vector2D operator * (const Vector2D& _other) const
        {
            return Vector2D(CAST(T, x * _other.x), CAST(T, y * _other.y));
        }
        constexpr Vector2D operator / (const Vector2D& _other) const
        {
            if (_other.x == 0 || _other.y == 0) THROW_EXCEPTION("Can't divide by zero !");
            return Vector2D(CAST(T, x / _other.x), CAST(T, y / _other.y));
        }

        Vector2D& operator += (const Vector2D& _other) noexcept 
        { 
            x = CAST(T, x + _other.x); 
            y = CAST(T, y + _other.y);
            return *this;
        }
        Vector2D& operator -= (const Vector2D& _other) noexcept 
        { 
            x = CAST(T, x - _other.x);
            y = CAST(T, y - _other.y);
            return *this;
        }
        Vector2D& operator *= (const Vector2D& _other) noexcept 
        { 
            x = CAST(T, x * _other.x);
            y = CAST(T, y * _other.y);
            return *this;
        }
        Vector2D& operator /= (const Vector2D& _other) 
        { 
            if (_other.x == 0 || _other.y == 0) THROW_EXCEPTION("Can't divide by zero !");
            x = CAST(T, x / _other.x);
            y = CAST(T, y / _other.y);
            return *this;
        }

        template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
        constexpr auto operator * (U _scalar) const noexcept
        {
            using R = std::common_type_t<T, U>;
            return Vector2D<R>(CAST(R, x * _scalar), (CAST(R, y * _scalar)));
        }
        template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
        constexpr auto operator/(U _scalar) const
        {
            if (_scalar == 0) THROW_EXCEPTION("Can't divide by zero !");

            using R = std::common_type_t<T, U>;
            return Vector2D<R>(CAST(R, x / _scalar), CAST(R, y / _scalar));
        }
        template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
        Vector2D& operator*=(U _scalar) noexcept
        {
            using R = std::common_type_t<T, U>;
            x = CAST(T, CAST(R, x) * CAST(R, _scalar));
            y = CAST(T, CAST(R, y) * CAST(R, _scalar));
            return *this;
        }
        template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
        Vector2D& operator/=(U _scalar)
        {
            if (_scalar == 0) THROW_EXCEPTION("Can't divide by zero !");

            using R = std::common_type_t<T, U>;
            x = CAST(T, CAST(R, x) / CAST(R, _scalar));
            y = CAST(T, CAST(R, y) / CAST(R, _scalar));
            return *this;
        }

        bool operator == (const Vector2D& _other) const noexcept
        {
            return x == _other.x && y == _other.y;
        }
        bool operator != (const Vector2D& _other) const noexcept
        {
            return !(*this == _other);
        }

        #pragma endregion
    };

    using FVector2 = Krampus::Vector2D<float>;
    using IVector2 = Krampus::Vector2D<int>;
    using UVector2 = Krampus::Vector2D<unsigned int>;
    using DVector2 = Krampus::Vector2D<double>;
    using LVector2 = Krampus::Vector2D<long>;
    using SVector2 = Krampus::Vector2D<short>;

    //Vector3D for sound TODO move somewhere else
    using IVector3 = sf::Vector3<int>;
    using FVector3 = sf::Vector3f;

}

///////////////////////////////////////////////////////////
// 
//  Debug : 
//  
//  FVector2 _vector = FVector2(1, 2);
//  LOG(engine::VerbosityType::Warning, _vector);
// 
///////////////////////////////////////////////////////////