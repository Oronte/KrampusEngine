#pragma once
#include "Math.h"

namespace Krampus
{
    class Angle : IPrintable
    {
    private:
        Float radians;

        // [0 ; 2pi]
        static Float NormalizeRadiansUnsigned(Float _radians) noexcept
        {
            _radians -= FMath::tau * FMath::Floor(_radians / FMath::tau);
            return _radians;
        }

        // [-pi ; pi]
        static Float NormalizeRadiansSigned(Float _radians) noexcept
        {
            _radians = NormalizeRadiansUnsigned(_radians);
            if (_radians >= FMath::pi) _radians -= FMath::tau;
            return _radians;
        }

    public:
        Angle() noexcept
        {
            radians = 0.0f;
        }
        Angle(const Float& _radians, const Bool& _isDegrees = false) noexcept
        {
            radians = _isDegrees ? Float(FMath::DegToRad(_radians)) : _radians;
        }
        Angle(const sf::Angle& _angle)
        {
            radians = _angle.asRadians();
        }

        static Angle FromDegrees(const Float& _degrees) noexcept
        { 
            return Angle(FMath::DegToRad(_degrees));
        }
        static Angle FromRadians(const Float& _radians) noexcept
        { 
            return Angle(_radians);
        }

        Float ToRadians() const noexcept
        {
            return radians;
        }
        Float ToDegrees() const noexcept
        { 
            return FMath::RadToDeg(radians);
        }

        Angle Normalized() const noexcept
        { 
            return Angle(NormalizeRadiansUnsigned(radians));
        }
        Angle NormalizedSigned() const noexcept
        { 
            return Angle(NormalizeRadiansSigned(radians));
        }

        Angle operator+(const Float& _radians) const noexcept
        { 
            return Angle(radians + _radians);
        }
        Angle operator-(const Float& _radians) const
        { 
            return Angle(radians - _radians);
        }
        Angle operator*(const Float& _radians) const noexcept
        { 
            return Angle(radians * _radians);
        }
        Angle operator/(const Float& _radians) const noexcept
        { 
            assert(_radians != 0.f && "Angle: division by zero");
            return Angle(radians / _radians);
        }

        Angle operator+(const Angle& _other) const noexcept
        {
            return Angle(radians + _other.radians);
        }
        Angle operator-(const Angle& _other) const noexcept
        { 
            return Angle(radians - _other.radians);
        }
        Angle operator*(const Angle& _other) const noexcept
        {
            return Angle(radians * _other.radians);
        }
        Angle operator/(const Angle& _other) const noexcept
        { 
            const Float& _otherRad = _other.radians;
            assert(_otherRad != 0.f && "Angle: division by zero");
            return Angle(radians / _otherRad);
        }

        inline Angle& operator+=(const Float& _radians) noexcept
        { 
            radians += _radians; 
            return *this;
        }
        inline Angle& operator-=(const Float& _radians)
        { 
            radians -= _radians; 
            return *this;
        }
        inline Angle& operator*=(const Float& _radians) noexcept
        { 
            radians *= _radians; 
            return *this;
        }
        inline Angle& operator/=(const Float& _radians) noexcept
        { 
            assert(_radians != 0.f && "Angle: division by zero");
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
        inline Angle& operator/=(const Angle& _other) noexcept
        { 
            const Float& _otherRad = _other.radians;
            assert(_otherRad != 0.f && "Angle: division by zero");
            radians /= _otherRad;
            return *this;
        }

        inline Bool operator==(const Angle& _other) const noexcept
        { 
            return FMath::ApproxEqual(radians, _other.radians);
        }
        inline Bool operator!=(const Angle& _other) const noexcept
        { 
            return !(*this == _other);
        }

        operator float() const noexcept
        {
            return (float)radians;
        }
        operator sf::Angle() const noexcept
        {
            return sf::radians(radians);
        }

        Float Sin() const noexcept
        { 
            return FMath::Sin(radians);
        }
        Float Cos() const noexcept
        {
            return FMath::Cos(radians);
        }
        Float Tan() const noexcept
        { 
            return FMath::Tan(radians);
        }

        void AddDegrees(const Float& _degrees) noexcept
        { 
            radians += FMath::DegToRad(_degrees);
        }
        void AddRadians(const Float& _radians) noexcept
        { 
            radians += _radians;
        }

        Angle RotatedDegrees(const Float& _degrees) const noexcept
        { 
            return Angle(radians + FMath::DegToRad(_degrees));
        }
        Angle RotatedRadians(const Float& _radians) const noexcept
        { 
            return Angle(radians + _radians);
        }

        Float SmallestDifferenceRadians(const Angle& _target) const noexcept
        {
            return NormalizeRadiansSigned(_target.radians - radians);
        }
        Float SmallestDifferenceDegrees(const Angle& _target) const noexcept
        {
            return FMath::RadToDeg(SmallestDifferenceRadians(_target));
        }

        Angle RotateTowards(const Angle& _target, const Float& _time) const noexcept
        {
            return Angle(radians + SmallestDifferenceRadians(_target) * _time);
        }

        Float DistanceToRadians(const Angle& _target) const noexcept
        {
            return FMath::Abs(SmallestDifferenceRadians(_target));
        }
        Float DistanceToDegrees(const Angle& _target) const noexcept
        {
            return FMath::RadToDeg(DistanceToRadians(_target));
        }

        friend Angle operator*(const Float& _scalar, const Angle& _angle) noexcept
        {
            return _angle * _scalar;
        }

        std::string ToString() const override
        {
            return std::format("Radians : {}, Degrees : {}", radians, ToDegrees());
        }
    };
}
