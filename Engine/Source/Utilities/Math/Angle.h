#pragma once
#include "Math.h"

namespace Krampus
{
    class Angle
    {
    private:
        float radians;

        // [0 ; 2pi]
        inline static float NormalizeRadiansUnsigned(float _radians) noexcept
        {
            _radians -= FMath::tau * FMath::Floor(_radians / FMath::tau);
            return _radians;
        }

        // [-pi ; pi]
        inline static float NormalizeRadiansSigned(float _radians) noexcept
        {
            _radians = NormalizeRadiansUnsigned(_radians);
            if (_radians >= FMath::pi) _radians -= FMath::tau;
            return _radians;
        }

    public:
        inline Angle() noexcept
        {
            radians = 0.0f;
        }
        inline Angle(const float& _radians, const bool& _isDegrees = false) noexcept
        {
            radians = _isDegrees ? FMath::DegToRad(_radians) : _radians;
        }
        inline Angle(const sf::Angle& _angle)
        {
            radians = _angle.asRadians();
        }

        inline static Angle FromDegrees(const float& _degrees) noexcept
        { 
            return Angle(FMath::DegToRad(_degrees));
        }
        inline static Angle FromRadians(const float& _radians) noexcept
        { 
            return Angle(_radians);
        }

        inline float ToRadians() const noexcept
        {
            return radians;
        }
        inline float ToDegrees() const noexcept
        { 
            return FMath::RadToDeg(radians);
        }

        inline Angle Normalized() const noexcept
        { 
            return Angle(NormalizeRadiansUnsigned(radians));
        }
        inline Angle NormalizedSigned() const noexcept
        { 
            return Angle(NormalizeRadiansSigned(radians));
        }

        inline Angle operator+(const float& _radians) const noexcept
        { 
            return Angle(radians + _radians);
        }
        inline Angle operator-(const float& _radians) const
        { 
            return Angle(radians - _radians);
        }
        inline Angle operator*(const float& _radians) const noexcept
        { 
            return Angle(radians * _radians);
        }
        inline Angle operator/(const float& _radians) const
        { 
            if (_radians == 0.f)
                THROW_EXCEPTION("You can't divide a angle by 0");
            return Angle(radians / _radians);
        }

        inline Angle operator+(const Angle& _other) const noexcept
        {
            return Angle(radians + _other.radians);
        }
        inline Angle operator-(const Angle& _other) const noexcept
        { 
            return Angle(radians - _other.radians);
        }
        inline Angle operator*(const Angle& _other) const noexcept
        {
            return Angle(radians * _other.radians);
        }
        inline Angle operator/(const Angle& _other) const
        { 
            const float& _otherRad = _other.radians;
            if (_otherRad == 0.f)
                THROW_EXCEPTION("You can't divide a angle by 0");
            return Angle(radians / _otherRad);
        }

        inline Angle& operator+=(const float& _radians) noexcept
        { 
            radians += _radians; 
            return *this;
        }
        inline Angle& operator-=(const float& _radians)
        { 
            radians -= _radians; 
            return *this;
        }
        inline Angle& operator*=(const float& _radians) noexcept
        { 
            radians *= _radians; 
            return *this;
        }
        inline Angle& operator/=(const float& _radians)
        { 
            if (_radians == 0.f)
                THROW_EXCEPTION("You can't divide a angle by 0");
            radians /= _radians; 
            return *this;
        }

        inline Angle& operator+=(const Angle& _other) noexcept
        { 
            radians += _other.radians; 
            return *this;
        }
        inline Angle& operator-=(const Angle& _other) noexcept
        { 
            radians -= _other.radians;
            return *this;
        }
        inline Angle& operator*=(const Angle& _other) noexcept
        { 
            radians *= _other.radians; 
            return *this;
        }
        inline Angle& operator/=(const Angle& _other)
        { 
            const float& _otherRad = _other.radians;
            if (_otherRad == 0.f)
                THROW_EXCEPTION("You can't divide a angle by 0");
            radians /= _otherRad;
            return *this;
        }

        inline bool operator==(const Angle& _other) const noexcept
        { 
            return FMath::ApproxEqual(radians, _other.radians);
        }
        inline bool operator!=(const Angle& _other) const noexcept
        { 
            return !(*this == _other);
        }

        inline operator float() const noexcept
        {
            return radians;
        }
        inline operator sf::Angle() const noexcept
        {
            return sf::radians(radians);
        }

        inline float Sin() const noexcept
        { 
            return FMath::Sin(radians);
        }
        inline float Cos() const noexcept
        {
            return FMath::Cos(radians);
        }
        inline float Tan() const noexcept
        { 
            return FMath::Tan(radians);
        }

        inline void AddDegrees(const float& _degrees) noexcept
        { 
            radians += FMath::DegToRad(_degrees);
        }
        inline void AddRadians(const float& _radians) noexcept
        { 
            radians += _radians;
        }

        inline Angle RotatedDegrees(const float& _degrees) const noexcept
        { 
            return Angle(radians + FMath::DegToRad(_degrees));
        }
        inline Angle RotatedRadians(const float& _radians) const noexcept
        { 
            return Angle(radians + _radians);
        }

        inline float SmallestDifferenceRadians(const Angle& _target) const noexcept
        {
            return NormalizeRadiansSigned(_target.radians - radians);
        }
        inline float SmallestDifferenceDegrees(const Angle& _target) const noexcept
        {
            return FMath::RadToDeg(SmallestDifferenceRadians(_target));
        }

        inline Angle RotateTowards(const Angle& _target, const float& _time) const noexcept
        {
            return Angle(radians + SmallestDifferenceRadians(_target) * _time);
        }

        inline float DistanceToRadians(const Angle& _target) const noexcept
        {
            return FMath::Abs(SmallestDifferenceRadians(_target));
        }
        inline float DistanceToDegrees(const Angle& _target) const noexcept
        {
            return FMath::RadToDeg(DistanceToRadians(_target));
        }

        friend inline Angle operator*(const float& _scalar, const Angle& _angle) noexcept
        {
            return _angle * _scalar;
        }
    };
}
