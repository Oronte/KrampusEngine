#pragma once
#include "Math.h"

namespace Krampus
{
    class Angle
    {
    private:
        float radians;

        // [0 ; 2pi]
        inlin static float NormalizeRadiansUnsigned(float _radians) noexcept
        {
            _radians -= FMath::tau * FMath::Floor(_radians / FMath::tau);
            return _radians;
        }

        // [-pi ; pi]
        inlin static float NormalizeRadiansSigned(float _radians) noexcept
        {
            _radians = NormalizeRadiansUnsigned(_radians);
            if (_radians >= FMath::pi) _radians -= FMath::tau;
            return _radians;
        }

    public:
        inlin Angle() noexcept
        {
            radians = 0.0f;
        }
        inlin explicit Angle(const float& _radians, const bool& _isDegrees = false) noexcept
        {
            radians = _isDegrees ? FMath::DegToRad(_radians) : _radians;
        }
        inlin explicit Angle(const sf::Angle& _angle)
        {
            radians = _angle.asRadians();
        }

        inlin static Angle FromDegrees(const float& _degrees) noexcept 
        { 
            return Angle(FMath::DegToRad(_degrees));
        }
        inlin static Angle FromRadians(const float& _radians) noexcept 
        { 
            return Angle(_radians);
        }

        inlin float ToRadians() const noexcept
        {
            return radians;
        }
        inlin float ToDegrees() const noexcept 
        { 
            return FMath::RadToDeg(radians);
        }

        inlin Angle Normalized() const noexcept 
        { 
            return Angle(NormalizeRadiansUnsigned(radians));
        }
        inlin Angle NormalizedSigned() const noexcept 
        { 
            return Angle(NormalizeRadiansSigned(radians));
        }

        inlin Angle operator+(const float& _radians) const noexcept 
        { 
            return Angle(radians + _radians);
        }
        inlin Angle operator-(const float& _radians) const 
        { 
            return Angle(radians - _radians);
        }
        inlin Angle operator*(const float& _radians) const noexcept 
        { 
            return Angle(radians * _radians);
        }
        inlin Angle operator/(const float& _radians) const 
        { 
            if (_radians == 0.f)
                THROW_EXCEPTION("You can't divide a angle by 0");
            return Angle(radians / _radians);
        }

        inlin Angle operator+(const Angle& _other) const noexcept 
        {
            return Angle(radians + _other.radians);
        }
        inlin Angle operator-(const Angle& _other) const noexcept 
        { 
            return Angle(radians - _other.radians);
        }
        inlin Angle operator*(const Angle& _other) const noexcept 
        {
            return Angle(radians * _other.radians);
        }
        inlin Angle operator/(const Angle& _other) const  
        { 
            const float& _otherRad = _other.radians;
            if (_otherRad == 0.f)
                THROW_EXCEPTION("You can't divide a angle by 0");
            return Angle(radians / _otherRad);
        }

        inlin Angle& operator+=(const float& _radians) noexcept 
        { 
            radians += _radians; 
            return *this;
        }
        inlin Angle& operator-=(const float& _radians) 
        { 
            radians -= _radians; 
            return *this;
        }
        inlin Angle& operator*=(const float& _radians) noexcept 
        { 
            radians *= _radians; 
            return *this;
        }
        inlin Angle& operator/=(const float& _radians) 
        { 
            if (_radians == 0.f)
                THROW_EXCEPTION("You can't divide a angle by 0");
            radians /= _radians; 
            return *this;
        }

        inlin Angle& operator+=(const Angle& _other) noexcept 
        { 
            radians += _other.radians; 
            return *this;
        }
        inlin Angle& operator-=(const Angle& _other) noexcept 
        { 
            radians -= _other.radians;
            return *this;
        }
        inlin Angle& operator*=(const Angle& _other) noexcept 
        { 
            radians *= _other.radians; 
            return *this;
        }
        inlin Angle& operator/=(const Angle& _other) 
        { 
            const float& _otherRad = _other.radians;
            if (_otherRad == 0.f)
                THROW_EXCEPTION("You can't divide a angle by 0");
            radians /= _otherRad;
            return *this;
        }

        inlin bool operator==(const Angle& _other) const noexcept 
        { 
            return FMath::ApproxEqual(radians, _other.radians);
        }
        inlin bool operator!=(const Angle& _other) const noexcept 
        { 
            return !(*this == _other);
        }

        inlin operator float() const noexcept
        {
            return radians;
        }
        inlin operator sf::Angle() const noexcept
        {
            return sf::radians(radians);
        }

        inlin float Sin() const noexcept 
        { 
            return FMath::Sin(radians);
        }
        inlin float Cos() const noexcept 
        {
            return FMath::Cos(radians);
        }
        inlin float Tan() const noexcept 
        { 
            return FMath::Tan(radians);
        }

        inlin void AddDegrees(const float& _degrees) noexcept 
        { 
            radians += FMath::DegToRad(_degrees);
        }
        inlin void AddRadians(const float& _radians) noexcept 
        { 
            radians += _radians;
        }

        inlin Angle RotatedDegrees(const float& _degrees) const noexcept 
        { 
            return Angle(radians + FMath::DegToRad(_degrees));
        }
        inlin Angle RotatedRadians(const float& _radians) const noexcept 
        { 
            return Angle(radians + _radians);
        }

        inlin float SmallestDifferenceRadians(const Angle& _target) const noexcept
        {
            return NormalizeRadiansSigned(_target.radians - radians);
        }
        inlin float SmallestDifferenceDegrees(const Angle& _target) const noexcept
        {
            return FMath::RadToDeg(SmallestDifferenceRadians(_target));
        }

        inlin Angle RotateTowards(const Angle& _target, const float& _time) const noexcept
        {
            return Angle(radians + SmallestDifferenceRadians(_target) * _time);
        }

        inlin float DistanceToRadians(const Angle& _target) const noexcept
        {
            return FMath::Abs(SmallestDifferenceRadians(_target));
        }
        inlin float DistanceToDegrees(const Angle& _target) const noexcept
        {
            return FMath::RadToDeg(DistanceToRadians(_target));
        }

        friend inlin Angle operator*(const float& _scalar, const Angle& _angle) noexcept
        {
            return _angle * _scalar;
        }
    };
}
