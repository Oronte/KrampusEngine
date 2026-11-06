#pragma once
#include "Math.h"

namespace Krampus
{

    template<typename T>
    struct Vector2D : public IPrintable
    {
        static_assert(std::is_arithmetic_v<T>, "Vector2D<T> requires an arithmetic type T");

        using CalcType = std::common_type_t<T, float>;
        using CalcTypeMath = Math<CalcType>;

        T x;
        T y;

        constexpr Vector2D() noexcept
        {
            x = y = static_cast<T>(0.0f);
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
            x = static_cast<T>(_vector.x);
            y = static_cast<T>(_vector.y);
        }
        template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
        Vector2D(const Vector2D<U>& _vector) noexcept
        {
            x = static_cast<T>(_vector.x);
            y = static_cast<T>(_vector.y);
        }

        static constexpr Vector2D<T> Up() noexcept
            requires std::is_signed_v<T>
        { 
            return Vector2D<T>(static_cast<T>(0), static_cast<T>(-1));
        };
        static constexpr Vector2D<T> Down() noexcept
        { 
            return Vector2D<T>(static_cast<T>(0), static_cast<T>(1));
        };
        static constexpr Vector2D<T> Right() noexcept
        { 
            return Vector2D<T>(static_cast<T>(1), static_cast<T>(0));
        };
        static constexpr Vector2D<T> Left() noexcept
            requires std::is_signed_v<T>
        { 
            return Vector2D<T>(static_cast<T>(-1), static_cast<T>(0));
        };
        static constexpr Vector2D<T> Zero() noexcept
        { 
            return Vector2D<T>(static_cast<T>(0), static_cast<T>(0));
        };
        static constexpr Vector2D<T> One() noexcept
        { 
            return Vector2D<T>(static_cast<T>(1), static_cast<T>(1));
        };

        [[nodiscard]] CalcType LengthSquared() const noexcept
        { 
            return static_cast<CalcType>(x) * static_cast<CalcType>(x) + static_cast<CalcType>(y) * static_cast<CalcType>(y);
        }
        [[nodiscard]] CalcType Length() const noexcept
        { 
            return CalcTypeMath::Sqrt(LengthSquared());
        }

        [[nodiscard]] Vector2D<CalcType> Normalized(const CalcType& _fallback = 0.0) const noexcept
        {
            CalcType _length = Length();

            if (_length <= CalcTypeMath::epsilon) return Vector2D<CalcType>(_fallback, _fallback);

            return Vector2D<CalcType>(static_cast<CalcType>(x) / _length, static_cast<CalcType>(y) / _length);
        }

        [[nodiscard]] void Clamp(const Vector2D& _min, const Vector2D& _max)
        {
            x = CalcTypeMath::Clamp(x, _min.x, _max.x);
            y = CalcTypeMath::Clamp(y, _min.y, _max.y);
        }

        [[nodiscard]] void ClampMax(const Float& _max)
        {
            x = x > _max ? _max : x;
            y = y > _max ? _max : y;
        }

        static [[nodiscard]] Vector2D<CalcType> Clamp(const Vector2D& _vector, const Vector2D& _min, const Vector2D& _max)
        {
            return Vector2D<CalcType>(CalcTypeMath::Clamp(_vector.x, _min.x, _max.x), CalcTypeMath::Clamp(_vector.y, _min.y, _max.y));
        }

        [[nodiscard]] CalcType Dot(const Vector2D& _other) const noexcept
        {
            return static_cast<CalcType>(x) * static_cast<CalcType>(_other.x) + static_cast<CalcType>(y) * static_cast<CalcType>(_other.y);
        }
        static [[nodiscard]] CalcType Dot(const Vector2D& _a, const Vector2D& _b) noexcept
        {
            return static_cast<CalcType>(_a.x) * static_cast<CalcType>(_b.x) + static_cast<CalcType>(_a.y) * static_cast<CalcType>(_b.y);
        }

        [[nodiscard]] CalcType Cross(const Vector2D& _other) const noexcept
        {
            return static_cast<CalcType>(x) * static_cast<CalcType>(_other.y) - static_cast<CalcType>(y) * static_cast<CalcType>(_other.x);
        }
        static [[nodiscard]] CalcType Cross(const Vector2D& _a, const Vector2D& _b) noexcept
        {
            return static_cast<CalcType>(_a.x) * static_cast<CalcType>(_b.y) - static_cast<CalcType>(_a.y) * static_cast<CalcType>(_b.x);
        }

        [[nodiscard]] Vector2D Perp() const noexcept
        { 
            return Vector2D(static_cast<T>(-y), static_cast<T>(x));
        }

        [[nodiscard]] CalcType DistanceSquared(const Vector2D& _other) const noexcept
        { 
            return (*this - _other).LengthSquared();
        }
        static [[nodiscard]] CalcType DistanceSquared(const Vector2D& _a, const Vector2D& _b) noexcept
        { 
            return (_a - _b).LengthSquared();
        }
        [[nodiscard]] CalcType Distance(const Vector2D& _other) const noexcept
        { 
            return CalcTypeMath::Sqrt(DistanceSquared(_other));
        }
        static [[nodiscard]] CalcType Distance(const Vector2D& _a, const Vector2D& _b) noexcept
        { 
            return CalcTypeMath::Sqrt(_a.DistanceSquared(_b));
        }

        [[nodiscard]] Vector2D<CalcType> DirectionTo(const Vector2D& _target, const CalcType& _fallback = 0.0) const noexcept
        {
            Vector2D<CalcType> _current{ static_cast<CalcType>(x), static_cast<CalcType>(y) };
            Vector2D<CalcType> _tgt{ static_cast<CalcType>(_target.x), static_cast<CalcType>(_target.y) };

            Vector2D<CalcType> _delta = _tgt - _current;
            CalcType _length = _delta.Length();

            if (_length <= CalcTypeMath::epsilon)
                return Vector2D<CalcType>(_fallback);

            return _delta / _length;
        }
        static [[nodiscard]] Vector2D<CalcType> Direction(const Vector2D& _from, const Vector2D& _to, const CalcType& _fallback = 0.0) noexcept
        {
            Vector2D<CalcType> _delta = _to - _from;
            CalcType _length = _delta.Length();

            if (_length <= CalcTypeMath::epsilon)
                return Vector2D<CalcType>(_fallback);

            return _delta / _length;
        }

        [[nodiscard]] Vector2D<CalcType> ClampMagnitude(CalcType _maxLength) const noexcept
        {
            CalcType _lengthSquared = static_cast<CalcType>(LengthSquared());
            CalcType _maxLengthSquared = _maxLength * _maxLength;

            if (_lengthSquared <= _maxLengthSquared)
                return Vector2D<CalcType>(static_cast<CalcType>(x), static_cast<CalcType>(y));

            CalcType _length = CalcTypeMath::Sqrt(_lengthSquared);
            return Vector2D<CalcType>(static_cast<CalcType>(x) * (_maxLength / _length), static_cast<CalcType>(y) * (_maxLength / _length));
        }

        [[nodiscard]] Vector2D<CalcType> MoveTowards(const Vector2D& _target, CalcType _maxDelta) const noexcept
        {

            Vector2D<CalcType> _current = Vector2D(static_cast<CalcType>(x), static_cast<CalcType>(y));
            Vector2D<CalcType> _tgt = Vector2D(static_cast<CalcType>(_target.x), static_cast<CalcType>(_target.y));
            Vector2D<CalcType> _toTarget = _tgt - _current;
            CalcType _dist = _toTarget.Length();

            if (_dist <= static_cast<CalcType>(_maxDelta) || _dist <= CalcTypeMath::epsilon) return _tgt;

            return _current + _toTarget * (static_cast<CalcType>(_maxDelta) / _dist);
        }

        [[nodiscard]] Vector2D<CalcType> Reflect(const Vector2D& _normal) const noexcept
        {
            Vector2D<CalcType> _incident = Vector2D(static_cast<CalcType>(x), static_cast<CalcType>(y));
            Vector2D<CalcType> _calcTypeNormal = Vector2D(static_cast<CalcType>(_normal.x), static_cast<CalcType>(_normal.y));
            CalcType _dotIncidentNormal = _incident.Dot(_calcTypeNormal);
            return _incident - _calcTypeNormal * (static_cast<CalcType>(2) * _dotIncidentNormal);
        }

        [[nodiscard]] Vector2D<CalcType> Project(const Vector2D& _axis) const noexcept
        {
            Vector2D<CalcType> _vector{ static_cast<CalcType>(x), static_cast<CalcType>(y) };
            Vector2D<CalcType> _direction{ static_cast<CalcType>(_axis.x), static_cast<CalcType>(_axis.y) };
            CalcType _denominator = _direction.Dot(_direction);

            if (_denominator <= CalcTypeMath::epsilon) return Vector2D<CalcType>(static_cast<CalcType>(0), static_cast<CalcType>(0));

            CalcType _projectionScale = _vector.Dot(_direction) / _denominator;
            return _direction * _projectionScale;
        }

        [[nodiscard]] CalcType AtanToRadian() const noexcept
        { 
            return CalcTypeMath::Atan2(static_cast<CalcType>(y), static_cast<CalcType>(x));
        }

        [[nodiscard]] CalcType AngleBetweenRadians(const Vector2D& _other) const noexcept
        {
            CalcType _thisAngle = AtanToRadian();
            CalcType _otherAngle = _other.AtanToRadian();

            CalcType _deltaAngle = _otherAngle - _thisAngle;

            while (_deltaAngle <= -CalcTypeMath::pi) _deltaAngle += CalcTypeMath::tau;
            while (_deltaAngle > CalcTypeMath::pi)  _deltaAngle -= CalcTypeMath::tau;

            return _deltaAngle;
        }
        static [[nodiscard]] CalcType AngleBetweenRadians(const Vector2D& _a, const Vector2D& _b) noexcept
        {
            CalcType _thisAngle = _a.AtanToRadian();
            CalcType _otherAngle = _b.AtanToRadian();

            CalcType _deltaAngle = _otherAngle - _thisAngle;

            while (_deltaAngle <= -CalcTypeMath::pi) _deltaAngle += CalcTypeMath::tau;
            while (_deltaAngle > CalcTypeMath::pi)  _deltaAngle -= CalcTypeMath::tau;

            return _deltaAngle;
        }

        [[nodiscard]] CalcType SignedAngleDeg(const Vector2D& _to) const noexcept
        {
            return CalcTypeMath::RadToDeg(AngleBetweenRadians(_to));
        }

        [[nodiscard]] Vector2D<CalcType> Rotated(const CalcType& _angleRad) const noexcept
        {
            CalcType _cos = CalcTypeMath::Cos(_angleRad);
            CalcType _sin = CalcTypeMath::Sin(_angleRad);

            CalcType _x = static_cast<CalcType>(x);
            CalcType _y = static_cast<CalcType>(y);

            return Vector2D<CalcType>(_x * _cos - _y * _sin,
                _x * _sin + _y * _cos);
        }

        [[nodiscard]] Vector2D<CalcType> RotatedDeg(const CalcType& _angleDeg) const noexcept
        {
            return Rotated(CalcTypeMath::DegToRad(_angleDeg));
        }

        [[nodiscard]] Vector2D<CalcType> RotateAround(const Vector2D& _pivot, CalcType _angleRad) const noexcept
        {
            Vector2D<CalcType> _calcTypePivot = Vector2D(static_cast<CalcType>(_pivot.x), static_cast<CalcType>(_pivot.y));
            Vector2D<CalcType> _self = Vector2D(static_cast<CalcType>(x), static_cast<CalcType>(y));
            return _calcTypePivot + (_self - _calcTypePivot).Rotated(_angleRad);
        }

        [[nodiscard]] Vector2D<CalcType> Lerp(const Vector2D& _b, CalcType _time) const noexcept
        {
            CalcType _x = CalcTypeMath::Lerp(static_cast<CalcType>(x), static_cast<CalcType>(_b.x), _time);
            CalcType _y = CalcTypeMath::Lerp(static_cast<CalcType>(y), static_cast<CalcType>(_b.y), _time);
            return Vector2D<CalcType>(_x, _y);
        }
        static Vector2D<CalcType> Lerp(const Vector2D& _a, const Vector2D& _b, CalcType _time) noexcept
        {
            CalcType _x = CalcTypeMath::Lerp(static_cast<CalcType>(_a.x), static_cast<CalcType>(_b.x), _time);
            CalcType _y = CalcTypeMath::Lerp(static_cast<CalcType>(_a.y), static_cast<CalcType>(_b.y), _time);
            return Vector2D<CalcType>(_x, _y);
        }
        
        [[nodiscard]] Vector2D<CalcType> ClosestPointOnSegment(const Vector2D& _a, const Vector2D& _b) const noexcept
        {
            Vector2D<CalcType> _calcTypeA = Vector2D(static_cast<CalcType>(_a.x), static_cast<CalcType>(_a.y));
            Vector2D<CalcType> _calcTypeB = Vector2D(static_cast<CalcType>(_b.x), static_cast<CalcType>(_b.y));
            Vector2D<CalcType> _thisVector = Vector2D(static_cast<CalcType>(x), static_cast<CalcType>(y));
            Vector2D<CalcType> _ab = _calcTypeB - _calcTypeA;
            CalcType _abSquared = _ab.LengthSquared();

            if (_abSquared <= CalcTypeMath::epsilon) return _calcTypeA;

            CalcType _t = (_thisVector - _calcTypeA).Dot(_ab) / _abSquared;
            CalcTypeMath::Saturate(_t);
            return _calcTypeA + _ab * _t;
        }

        [[nodiscard]] CalcType DistanceToSegment(const Vector2D& _a, const Vector2D& _b) const noexcept
        {
            Vector2D<CalcType> _closestPoint = ClosestPointOnSegment(_a, _b);
            Vector2D<CalcType> _thisVector{ static_cast<CalcType>(x), static_cast<CalcType>(y) };
            return (_thisVector - _closestPoint).Length();
        }

        [[nodiscard]] CalcType SignedDistanceToLine(const Vector2D& _a, const Vector2D& _b) const noexcept
        {
            Vector2D<CalcType> _calcTypeA{ static_cast<CalcType>(_a.x), static_cast<CalcType>(_a.y) };
            Vector2D<CalcType> _calcTypeB{ static_cast<CalcType>(_b.x), static_cast<CalcType>(_b.y) };
            Vector2D<CalcType> _thisVector{ static_cast<CalcType>(x), static_cast<CalcType>(y) };
            Vector2D<CalcType> _ab = _calcTypeB - _calcTypeA;
            CalcType _denominator = CalcTypeMath::Sqrt(_ab.LengthSquared());

            if (_denominator <= CalcTypeMath::epsilon)
                return static_cast<CalcType>(0);

            return static_cast<CalcType>(_ab.Cross(_thisVector - _calcTypeA) / _denominator);
        }

        Bool IsOnSegment(const Vector2D& _a, const Vector2D& _b, CalcType _epsVal = Math::epsilon) const noexcept
        {
            return DistanceToSegment(_a, _b) <= _epsVal;
        }


        static Bool ComputeLineIntersection(const Vector2D& _lineAOrigin, const Vector2D& _lineADirection,
            const Vector2D& _lineBOrigin, const Vector2D& _lineBDirection,
            CalcType& _outLineAParam, CalcType& _outLineBParam) noexcept
        {
            Vector2D<CalcType> _pointA = Vector2D(static_cast<CalcType>(_lineAOrigin.x), static_cast<CalcType>(_lineAOrigin.y));
            Vector2D<CalcType> _dirA = Vector2D(static_cast<CalcType>(_lineADirection.x), static_cast<CalcType>(_lineADirection.y));

            Vector2D<CalcType> _pointB = Vector2D(static_cast<CalcType>(_lineBOrigin.x), static_cast<CalcType>(_lineBOrigin.y));
            Vector2D<CalcType> _dirB = Vector2D(static_cast<CalcType>(_lineBDirection.x), static_cast<CalcType>(_lineBDirection.y));

            CalcType _crossDirADirB = _dirA.Cross(_dirB);
            Vector2D<CalcType> _deltaAB = _pointB - _pointA;
            CalcType _crossDeltaABDirA = _deltaAB.Cross(_dirA);

            if (CalcTypeMath::Abs(_crossDirADirB) <= CalcTypeMath::epsilon)
                return false;

            _outLineAParam = static_cast<CalcType>(_deltaAB.Cross(_dirB) / _crossDirADirB);
            _outLineBParam = static_cast<CalcType>(_crossDeltaABDirA / _crossDirADirB);
            return true;
        }

        static Bool ComputeSegmentIntersection(const Vector2D& _segmentAStart, const Vector2D& _segmentADirection,
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
                _intersectionPoint->x = static_cast<double>(_segmentAStart.x) + static_cast<double>(_segmentADirection.x) * _t;
                _intersectionPoint->y = static_cast<double>(_segmentAStart.y) + static_cast<double>(_segmentADirection.y) * _t;
            }
            return true;
        }

        static Bool FindSegmentIntersection(const Vector2D& _segmentAStart, const Vector2D& _segmentAEnd,
            const Vector2D& _segmentBStart, const Vector2D& _segmentBEnd,
            Vector2D<double>* _intersectionPoint = nullptr) noexcept
        {
            Vector2D _directionA = _segmentAEnd - _segmentAStart;
            Vector2D _directionB = _segmentBEnd - _segmentBStart;

            return ComputeSegmentIntersection(_segmentAStart, _directionA,
                _segmentBStart, _directionB,
                _intersectionPoint);
        }

        static Vector2D<T> RandomRange(T _min, T _max)
        {
            return Vector2D<T>(Math<T>::RandomRange(_min, _max), Math<T>::RandomRange(_min, _max));
        }


        std::string ToString() const override
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
            return Vector2D(static_cast<T>(x + _other.x), static_cast<T>(y + _other.y));
        }
        constexpr Vector2D operator - (const Vector2D& _other) const noexcept 
        { 
            return Vector2D(static_cast<T>(x - _other.x), static_cast<T>(y - _other.y));
        }
        constexpr Vector2D operator * (const Vector2D& _other) const
        {
            return Vector2D(static_cast<T>(x * _other.x), static_cast<T>(y * _other.y));
        }
        constexpr Vector2D operator / (const Vector2D& _other) const noexcept
        {
            assert(_other.x != 0 && _other.y != 0 && "Vector2D: division by zero");
            return Vector2D(static_cast<T>(x / _other.x), static_cast<T>(y / _other.y));
        }

        Vector2D& operator += (const Vector2D& _other) noexcept 
        { 
            x = static_cast<T>(x + _other.x); 
            y = static_cast<T>(y + _other.y);
            return *this;
        }
        Vector2D& operator -= (const Vector2D& _other) noexcept 
        { 
            x = static_cast<T>(x - _other.x);
            y = static_cast<T>(y - _other.y);
            return *this;
        }
        Vector2D& operator *= (const Vector2D& _other) noexcept 
        { 
            x = static_cast<T>(x * _other.x);
            y = static_cast<T>(y * _other.y);
            return *this;
        }
        Vector2D& operator /= (const Vector2D& _other) noexcept
        { 
            assert(_other.x != 0 && _other.y != 0 && "Vector2D: division by zero");
            x = static_cast<T>(x / _other.x);
            y = static_cast<T>(y / _other.y);
            return *this;
        }

        template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
        constexpr auto operator * (U _scalar) const noexcept
        {
            using R = std::common_type_t<T, U>;
            return Vector2D<R>(static_cast<R>(x * _scalar), (static_cast<R>(y * _scalar)));
        }
        template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
        constexpr auto operator/(U _scalar) const noexcept
        {
            assert(_scalar != 0 && "Vector2D: division by zero");

            using R = std::common_type_t<T, U>;
            return Vector2D<R>(static_cast<R>(x / _scalar), static_cast<R>(y / _scalar));
        }
        template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
        Vector2D& operator*=(U _scalar) noexcept
        {
            using R = std::common_type_t<T, U>;
            x = static_cast<T>(static_cast<R>(x) * static_cast<R>(_scalar));
            y = static_cast<T>(static_cast<R>(y) * static_cast<R>(_scalar));
            return *this;
        }
        template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
        Vector2D& operator/=(U _scalar) noexcept
        {
            assert(_scalar != 0 && "Vector2D: division by zero");

            using R = std::common_type_t<T, U>;
            x = static_cast<T>(static_cast<R>(x) / static_cast<R>(_scalar));
            y = static_cast<T>(static_cast<R>(y) / static_cast<R>(_scalar));
            return *this;
        }

        constexpr Vector2D<T> operator * (const Float& _scalar) const noexcept
        {
            return Vector2D<T>(static_cast<T>(x * _scalar), (static_cast<T>(y * _scalar)));
        }
        constexpr Vector2D<T> operator/(const Float& _scalar) const noexcept
        {
            assert(_scalar != 0 && "Vector2D: division by zero");

            return Vector2D<T>(static_cast<T>(x / _scalar), static_cast<T>(y / _scalar));
        }
        Vector2D& operator*=(const Float& _scalar) noexcept
        {
            x = x * static_cast<T>(_scalar);
            y = y * static_cast<T>(_scalar);
            return *this;
        }
        Vector2D& operator/=(const Float& _scalar) noexcept
        {
            assert(_scalar != 0 && "Vector2D: division by zero");

            x = x / static_cast<T>(_scalar);
            y = y / static_cast<T>(_scalar);
            return *this;
        }

        Bool operator == (const Vector2D& _other) const noexcept
        {
            return x == _other.x && y == _other.y;
        }
        Bool operator != (const Vector2D& _other) const noexcept
        {
            return !(*this == _other);
        }

        constexpr Vector2D operator-() const
        {
            return { -x, -y };
        }

        #pragma endregion
    };

    using FVector2 = Krampus::Vector2D<float>;
    using IVector2 = Krampus::Vector2D<int>;
    using UVector2 = Krampus::Vector2D<unsigned int>;
    using DVector2 = Krampus::Vector2D<double>;
    using LVector2 = Krampus::Vector2D<long>;
    using SVector2 = Krampus::Vector2D<short>;
}

///////////////////////////////////////////////////////////
// 
//  Debug : 
//  
//  FVector2 _vector = FVector2(1, 2);
//  LOG(engine::VerbosityType::Warning, _vector);
// 
///////////////////////////////////////////////////////////