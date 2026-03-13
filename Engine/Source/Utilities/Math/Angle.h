#pragma once
#include "Math.h"

namespace Krampus
{
    class Angle : IPrintable
    {
    private:
        Float radians;

        // [0 ; 2pi]
        inline static Float NormalizeRadiansUnsigned(Float _radians) noexcept
        {
            _radians -= FMath::tau * FMath::Floor(_radians / FMath::tau);
            return _radians;
        }

        // [-pi ; pi]
        inline static Float NormalizeRadiansSigned(Float _radians) noexcept
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
        inline Angle(const Float& _radians, const Bool& _isDegrees = false) noexcept
        {
            radians = _isDegrees ? Float(FMath::DegToRad(_radians)) : _radians;
        }
        inline Angle(const sf::Angle& _angle)
        {
            radians = _angle.asRadians();
        }

        inline static Angle FromDegrees(const Float& _degrees) noexcept
        { 
            return Angle(FMath::DegToRad(_degrees));
        }
        inline static Angle FromRadians(const Float& _radians) noexcept
        { 
            return Angle(_radians);
        }

        inline Float ToRadians() const noexcept
        {
            return radians;
        }
        inline Float ToDegrees() const noexcept
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

        inline Angle operator+(const Float& _radians) const noexcept
        { 
            return Angle(radians + _radians);
        }
        inline Angle operator-(const Float& _radians) const
        { 
            return Angle(radians - _radians);
        }
        inline Angle operator*(const Float& _radians) const noexcept
        { 
            return Angle(radians * _radians);
        }
        inline Angle operator/(const Float& _radians) const
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
            const Float& _otherRad = _other.radians;
            if (_otherRad.IsZero())
                THROW_EXCEPTION("You can't divide a angle by 0");
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
        inline Angle& operator/=(const Float& _radians)
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
            const Float& _otherRad = _other.radians;
            if (_otherRad.IsZero())
                THROW_EXCEPTION("You can't divide a angle by 0");
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

        inline operator float() const noexcept
        {
            return (float)radians;
        }
        inline operator Float() const noexcept
        {
            return radians;
        }
        inline operator sf::Angle() const noexcept
        {
            return sf::radians(radians);
        }

        inline Float Sin() const noexcept
        { 
            return FMath::Sin(radians);
        }
        inline Float Cos() const noexcept
        {
            return FMath::Cos(radians);
        }
        inline Float Tan() const noexcept
        { 
            return FMath::Tan(radians);
        }

        inline void AddDegrees(const Float& _degrees) noexcept
        { 
            radians += FMath::DegToRad(_degrees);
        }
        inline void AddRadians(const Float& _radians) noexcept
        { 
            radians += _radians;
        }

        inline Angle RotatedDegrees(const Float& _degrees) const noexcept
        { 
            return Angle(radians + FMath::DegToRad(_degrees));
        }
        inline Angle RotatedRadians(const Float& _radians) const noexcept
        { 
            return Angle(radians + _radians);
        }

        inline Float SmallestDifferenceRadians(const Angle& _target) const noexcept
        {
            return NormalizeRadiansSigned(_target.radians - radians);
        }
        inline Float SmallestDifferenceDegrees(const Angle& _target) const noexcept
        {
            return FMath::RadToDeg(SmallestDifferenceRadians(_target));
        }

        inline Angle RotateTowards(const Angle& _target, const Float& _time) const noexcept
        {
            return Angle(radians + SmallestDifferenceRadians(_target) * _time);
        }

        inline Float DistanceToRadians(const Angle& _target) const noexcept
        {
            return FMath::Abs(SmallestDifferenceRadians(_target));
        }
        inline Float DistanceToDegrees(const Angle& _target) const noexcept
        {
            return FMath::RadToDeg(DistanceToRadians(_target));
        }

        friend inline Angle operator*(const Float& _scalar, const Angle& _angle) noexcept
        {
            return _angle * _scalar;
        }

        virtual std::string ToString() const override
        {
            return std::format("Radians : {}, Degrees : {}", radians.ToString(), ToDegrees().ToString());
        }
    };
}
