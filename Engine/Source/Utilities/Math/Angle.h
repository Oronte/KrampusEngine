#pragma once
#include "Math.h"

namespace Krampus
{
    class Angle
    {
    private:
        float radians;

        // [0 ; 2pi]
        INLINE static float NormalizeRadiansUnsigned(float _radians) noexcept
        {
            _radians -= FMath::tau * FMath::Floor(_radians / FMath::tau);
            return _radians;
        }

        // [-pi ; pi]
        INLINE static float NormalizeRadiansSigned(float _radians) noexcept
        {
            _radians = NormalizeRadiansUnsigned(_radians);
            if (_radians >= FMath::pi) _radians -= FMath::tau;
            return _radians;
        }

    public:
        INLINE Angle() noexcept
        {
            radians = 0.0f;
        }
        INLINE explicit Angle(const float& _radians, const bool& _isDegrees = false) noexcept
        {
            radians = _isDegrees ? FMath::DegToRad(_radians) : _radians;
        }
        INLINE explicit Angle(const sf::Angle& _angle)
        {
            radians = _angle.asRadians();
        }

        INLINE static Angle FromDegrees(const float& _degrees) noexcept 
        { 
            return Angle(FMath::DegToRad(_degrees));
        }
        INLINE static Angle FromRadians(const float& _radians) noexcept 
        { 
            return Angle(_radians);
        }

        INLINE float ToRadians() const noexcept
        {
            return radians;
        }
        INLINE float ToDegrees() const noexcept 
        { 
            return FMath::RadToDeg(radians);
        }

        INLINE Angle Normalized() const noexcept 
        { 
            return Angle(NormalizeRadiansUnsigned(radians));
        }
        INLINE Angle NormalizedSigned() const noexcept 
        { 
            return Angle(NormalizeRadiansSigned(radians));
        }

        INLINE Angle operator+(const float& _radians) const noexcept 
        { 
            return Angle(radians + _radians);
        }
        INLINE Angle operator-(const float& _radians) const 
        { 
            return Angle(radians - _radians);
        }
        INLINE Angle operator*(const float& _radians) const noexcept 
        { 
            return Angle(radians * _radians);
        }
        INLINE Angle operator/(const float& _radians) const 
        { 
            if (_radians == 0.f)
                THROW_EXCEPTION("You can't divide a angle by 0");
            return Angle(radians / _radians);
        }

        INLINE Angle operator+(const Angle& _other) const noexcept 
        {
            return Angle(radians + _other.radians);
        }
        INLINE Angle operator-(const Angle& _other) const noexcept 
        { 
            return Angle(radians - _other.radians);
        }
        INLINE Angle operator*(const Angle& _other) const noexcept 
        {
            return Angle(radians * _other.radians);
        }
        INLINE Angle operator/(const Angle& _other) const  
        { 
            const float& _otherRad = _other.radians;
            if (_otherRad == 0.f)
                THROW_EXCEPTION("You can't divide a angle by 0");
            return Angle(radians / _otherRad);
        }

        INLINE Angle& operator+=(const float& _radians) noexcept 
        { 
            radians += _radians; 
            return *this;
        }
        INLINE Angle& operator-=(const float& _radians) 
        { 
            radians -= _radians; 
            return *this;
        }
        INLINE Angle& operator*=(const float& _radians) noexcept 
        { 
            radians *= _radians; 
            return *this;
        }
        INLINE Angle& operator/=(const float& _radians) 
        { 
            if (_radians == 0.f)
                THROW_EXCEPTION("You can't divide a angle by 0");
            radians /= _radians; 
            return *this;
        }

        INLINE Angle& operator+=(const Angle& _other) noexcept 
        { 
            radians += _other.radians; 
            return *this;
        }
        INLINE Angle& operator-=(const Angle& _other) noexcept 
        { 
            radians -= _other.radians;
            return *this;
        }
        INLINE Angle& operator*=(const Angle& _other) noexcept 
        { 
            radians *= _other.radians; 
            return *this;
        }
        INLINE Angle& operator/=(const Angle& _other) 
        { 
            const float& _otherRad = _other.radians;
            if (_otherRad == 0.f)
                THROW_EXCEPTION("You can't divide a angle by 0");
            radians /= _otherRad;
            return *this;
        }

        INLINE bool operator==(const Angle& _other) const noexcept 
        { 
            return FMath::ApproxEqual(radians, _other.radians);
        }
        INLINE bool operator!=(const Angle& _other) const noexcept 
        { 
            return !(*this == _other);
        }

        INLINE operator float() const noexcept
        {
            return radians;
        }
        INLINE operator sf::Angle() const noexcept
        {
            return sf::radians(radians);
        }

        INLINE float Sin() const noexcept 
        { 
            return FMath::Sin(radians);
        }
        INLINE float Cos() const noexcept 
        {
            return FMath::Cos(radians);
        }
        INLINE float Tan() const noexcept 
        { 
            return FMath::Tan(radians);
        }

        INLINE void AddDegrees(const float& _degrees) noexcept 
        { 
            radians += FMath::DegToRad(_degrees);
        }
        INLINE void AddRadians(const float& _radians) noexcept 
        { 
            radians += _radians;
        }

        INLINE Angle RotatedDegrees(const float& _degrees) const noexcept 
        { 
            return Angle(radians + FMath::DegToRad(_degrees));
        }
        INLINE Angle RotatedRadians(const float& _radians) const noexcept 
        { 
            return Angle(radians + _radians);
        }

        INLINE float SmallestDifferenceRadians(const Angle& _target) const noexcept
        {
            return NormalizeRadiansSigned(_target.radians - radians);
        }
        INLINE float SmallestDifferenceDegrees(const Angle& _target) const noexcept
        {
            return FMath::RadToDeg(SmallestDifferenceRadians(_target));
        }

        INLINE Angle RotateTowards(const Angle& _target, const float& _time) const noexcept
        {
            return Angle(radians + SmallestDifferenceRadians(_target) * _time);
        }

        INLINE float DistanceToRadians(const Angle& _target) const noexcept
        {
            return FMath::Abs(SmallestDifferenceRadians(_target));
        }
        INLINE float DistanceToDegrees(const Angle& _target) const noexcept
        {
            return FMath::RadToDeg(DistanceToRadians(_target));
        }

        friend INLINE Angle operator*(const float& _scalar, const Angle& _angle) noexcept
        {
            return _angle * _scalar;
        }
    };
}
