#pragma once
#include "Utilities/Debug/Exception.h"

namespace Krampus
{

    template<typename Real>
	class Math
	{
        static_assert(std::is_arithmetic_v<Real>, "Math<Real> requires an arithmetic type Real");

    public:
        static INLINE constexpr Real pi = CAST(Real, 3.14159265358979323846);
        static INLINE constexpr Real tau = CAST(Real, 2.0 * 3.14159265358979323846);
        static INLINE constexpr Real halfPi = CAST(Real, 1.57079632679489661923);
        static INLINE constexpr Real degToRad = CAST(Real, pi / 180.0);
        static INLINE constexpr Real radToDeg = CAST(Real, 180.0 / pi);
        static INLINE constexpr Real epsilon = CAST(Real, 1e-6);
        static INLINE constexpr Real infinity = std::numeric_limits<Real>::infinity();
        static INLINE constexpr Real golden = CAST(Real, 1.6180339887498948);


        #pragma region Common Functions

        static INLINE Real Sqrt(const Real& _x)
        {
            if (_x < CAST(Real, 0))
                THROW_EXCEPTION(std::format("Sqrt: negative input => _x = {} (must be >= 0)", _x));
            return CAST(Real, std::sqrt(_x));
        }
        static INLINE Real InvSqrt(const Real& _x)
        {
            if (_x <= CAST(Real, 0))
                THROW_EXCEPTION(std::format("InvSqrt: non-positive input => _x = {} (must be > 0)", _x));
            return CAST(Real, 1) / Sqrt(_x);
        }
        static INLINE Real Pow(const Real& _base, const int& _exp) noexcept
        {
            return CAST(Real, std::pow(_base, _exp));
        }
        static INLINE Real Exp(const Real& _x) noexcept
        {
            return CAST(Real, std::exp(_x));
        }
        static INLINE Real Log(const Real& _x)
        {
            if (_x <= CAST(Real, 0))
                THROW_EXCEPTION(std::format("Log: non-positive input => _x = {} (must be > 0)", _x));
            return CAST(Real, std::log(_x));
        }
        static INLINE Real Log10(const Real& _x)
        {
            if (_x <= CAST(Real, 0))
                THROW_EXCEPTION(std::format("Log10: non-positive input => _x = {} (must be > 0)", _x));
            return CAST(Real, std::log10(_x));
        }

        static INLINE Real Sin(const Real& _x) noexcept
        {
            return CAST(Real, std::sin(_x));
        }
        static INLINE Real Cos(const Real& _x) noexcept
        {
            return CAST(Real, std::cos(_x));
        }
        static INLINE Real Tan(const Real& _x) noexcept
        {
            return CAST(Real, std::tan(_x));
        }

        static INLINE Real Asin(const Real& _x)
        {
            if (_x < CAST(Real, -1) || _x > CAST(Real, 1))
                THROW_EXCEPTION(std::format("Asin: input out of range => _x = {} (must be in [-1, 1])", _x));
            return CAST(Real, std::asin(_x));
        }
        static INLINE Real Acos(const Real& _x)
        {
            if (_x < CAST(Real, -1) || _x > CAST(Real, 1))
                THROW_EXCEPTION(std::format("Acos: input out of range => _x = {} (must be in [-1, 1])", _x));
            return CAST(Real, std::acos(_x));
        }
        static INLINE Real Atan(const Real& _x) noexcept
        {
            return CAST(Real, std::atan(_x));
        }
        static INLINE Real Atan2(const Real& _y, const Real& _x)
        {
            if (_x == CAST(Real, 0) && _y == CAST(Real, 0))
                THROW_EXCEPTION(std::format("Atan2: undefined for _x = {} and _y = {} (both 0)", _x, _y));
            return CAST(Real, std::atan2(_y, _x));
        }

        static INLINE Real Sinh(const Real& _x) noexcept
        {
            return CAST(Real, std::sinh(_x));
        }
        static INLINE Real Cosh(const Real& _x) noexcept
        {
            return CAST(Real, std::cosh(_x));
        }
        static INLINE Real Tanh(const Real& _x) noexcept
        {
            return CAST(Real, std::tanh(_x));
        }

        static INLINE Real Floor(const Real& _x) noexcept
        {
            return CAST(Real, std::floor(_x));
        }
        static INLINE Real Ceil(const Real& _x) noexcept
        {
            return CAST(Real, std::ceil(_x));
        }
        static INLINE Real Round(const Real& _x) noexcept
        {
            return CAST(Real, std::round(_x));
        }
        static INLINE Real Trunc(const Real& _x) noexcept
        {
            return CAST(Real, std::trunc(_x));
        }


        /// <summary>Converts degrees to radians.</summary>
        static INLINE constexpr Real DegToRad(const Real& _degrees) noexcept
        {
            return _degrees * degToRad;
        }

        /// <summary>Converts radians to degrees.</summary>
        static INLINE constexpr Real RadToDeg(const Real& _radian) noexcept
        {
            return _radian * radToDeg;
        }

        /// <summary>Returns the absolute value of a scalar.</summary>
        static INLINE constexpr Real Abs(const Real& _value) noexcept
        {
            return _value < 0 ? -_value : _value;
        }

        /// <summary>Returns the sign of a value (-1, 0, or 1).</summary>
        static INLINE constexpr Real Sign(const Real& _value) noexcept
        {
            return CAST(Real, (_value > 0)) - CAST(Real, (_value < 0));
        }

        static INLINE constexpr Real MinVal(const Real& _a, const Real& _b) noexcept
        {
            return _a < _b ? _a : _b;
        }

        static INLINE constexpr Real MaxVal(const Real& _a, const Real& _b) noexcept
        {
            return _a > _b ? _a : _b;
        }

        /// <summary>Clamps a value between a minimum and maximum range.</summary>
        static INLINE constexpr Real Clamp(const Real& _value, const Real& _min, const Real& _max) noexcept
        {
            return _value < _min ? _min : _value > _max ? _max : _value;
        }

        /// <summary>Clamps a value between 0 and 1.</summary>
        static INLINE constexpr Real Saturate(const Real& _value) noexcept
        {
            return Clamp(_value, CAST(Real, 0), CAST(Real, 1));
        }

        /// <summary>Returns true if two values are approximately equal (within epsilon).</summary>
        static INLINE constexpr bool ApproxEqual(const Real& _a, const Real& _b, const Real& _eps = epsilon) noexcept
        {
            return Abs(_a - _b) <= _eps;
        }

        #pragma endregion


        #pragma region Interpolation

        /// <summary>Linearly interpolates between two values.</summary>
        static INLINE constexpr Real Lerp(const Real& _a, const Real& _b, const Real& _time) noexcept
        {
            return _a + (_b - _a) * _time;
        }

        static INLINE constexpr Real ConstantLerp(const Real& _start, const Real& _end, const Real& _time)
        {
            return (1 - _time) * _start + _time * _end;
        }

        /// <summary>Inverse linear interpolation (returns how far _v is between _a and _b).</summary>
        static INLINE constexpr Real InverseLerp(const Real& _a, const Real& _b, const Real& _v) noexcept
        {
            return (_b - _a) == 0 ? 0 : (_v - _a) / (_b - _a);
        }

        /// <summary>Remaps a value from one range to another.</summary>
        static INLINE constexpr Real Remap(const Real& _inMin, const Real& _inMax, const Real& _outMin, const Real& _outMax, const Real& _v) noexcept
        {
            return Lerp(_outMin, _outMax, Saturate(InverseLerp(_inMin, _inMax, _v)));
        }

        /// <summary>Returns a smooth interpolation between 0 and 1 based on x (Hermite curve).</summary>
        static INLINE Real SmoothStep(const Real& _edge0, const Real& _edge1, Real _x) noexcept
        {
            _x = Saturate((_x - _edge0) / (_edge1 - _edge0));
            return _x * _x * (3 - 2 * _x);
        }

        /// <summary>Returns a smoother interpolation (cubic curve) between 0 and 1.</summary>
        static INLINE Real SmootherStep(const Real& _edge0, const Real& _edge1, Real _x) noexcept
        {
            _x = Saturate((_x - _edge0) / (_edge1 - _edge0));
            return _x * _x * _x * (_x * (_x * 6 - 15) + 10);
        }

        #pragma endregion


        #pragma region Random

        static NO_DISCARD Real RandomRange(const Real& _min, const Real& _max)
        {
            std::random_device _rSeed; // Get a random seed
            std::mt19937 _gen(_rSeed()); // Init generator with the seed
            std::uniform_real_distribution<Real> _distr(_min, _max); // Define limits

            return _distr(_gen); // Generate number
        }

        #pragma endregion


        #pragma region Easing curves


        static NO_DISCARD INLINE Real Linear(const Real& _x) noexcept
        {
            return _x;
        }

        static NO_DISCARD INLINE Real EaseInSine(const Real& _x) noexcept
        {
            return CAST(Real, 1.0) - Cos((_x * pi) / CAST(Real, 2.0));
        }
        static NO_DISCARD INLINE Real EaseOutSine(const Real& _x) noexcept
        {
            return Sin((_x * pi) / CAST(Real, 2.0));
        }
        static NO_DISCARD INLINE Real EaseInOutSine(const Real& _x) noexcept
        {
            return -CAST(Real, 0.5) * (Cos(pi * _x) - CAST(Real, 1.0));
        }

        static NO_DISCARD INLINE Real EaseInQuad(const Real& _x) noexcept
        {
            return _x * _x;
        }
        static NO_DISCARD INLINE Real EaseOutQuad(const Real& _x) noexcept
        {
            constexpr Real _one = CAST(Real, 1.0);
            return _one - (_one - _x) * (_one - _x);
        }
        static NO_DISCARD INLINE Real EaseInOutQuad(const Real& _x) noexcept
        {
            constexpr Real _two = CAST(Real, 2.0);
            return (_x < CAST(Real, 0.5)) ?
                _two * _x * _x :
                CAST(Real, 1.0) - Pow(-_two * _x + _two, 2) / _two;
        }

        static NO_DISCARD INLINE Real EaseInCubic(const Real& _x) noexcept
        {
            return _x * _x * _x;
        }
        static NO_DISCARD INLINE Real EaseOutCubic(const Real& _x) noexcept
        {
            constexpr Real _one = CAST(Real, 1.0);
            return _one - Pow(_one - _x, 3);
        }
        static NO_DISCARD INLINE Real EaseInOutCubic(const Real& _x) noexcept
        {
            constexpr Real _two = CAST(Real, 2.0);
            return (_x < CAST(Real, 0.5)) ?
                CAST(Real, 4.0) * _x * _x * _x :
                CAST(Real, 1.0) - Pow(-_two * _x + _two, 3) / _two;
        }

        static NO_DISCARD INLINE Real EaseInQuart(const Real& _x) noexcept
        {
            return _x * _x * _x * _x;
        }
        static NO_DISCARD INLINE Real EaseOutQuart(const Real& _x) noexcept
        {
            constexpr Real _one = CAST(Real, 1.0);
            return _one - Pow(_one - _x, 4);
        }
        static NO_DISCARD INLINE Real EaseInOutQuart(const Real& _x) noexcept
        {
            constexpr Real _two = CAST(Real, 2.0);
            return (_x < CAST(Real, 0.5)) ?
                CAST(Real, 8.0) * _x * _x * _x * _x :
                CAST(Real, 1.0) - Pow(-_two * _x + _two, 4) / _two;
        }

        static NO_DISCARD INLINE Real EaseInQuint(const Real& _x) noexcept
        {
            return _x * _x * _x * _x * _x;
        }
        static NO_DISCARD INLINE Real EaseOutQuint(const Real& _x) noexcept
        {
            constexpr Real _one = CAST(Real, 1.0);
            return _one - Pow(_one - _x, 5);
        }
        static NO_DISCARD INLINE Real EaseInOutQuint(const Real& _x) noexcept
        {
            constexpr Real _two = CAST(Real, 2.0);
            return (_x < CAST(Real, 0.5)) ?
                CAST(Real, 16.0) * _x * _x * _x * _x * _x :
                CAST(Real, 1.0) - Pow(-_two * _x + _two, 5) / _two;
        }

        static NO_DISCARD INLINE Real EaseInExpo(const Real& _x) noexcept
        {
            constexpr Real _zero = CAST(Real, 0.0);
            constexpr Real _ten = CAST(Real, 10.0);
            return (_x == _zero) ? _zero : Pow(2.0, CAST(int, _ten * _x - _ten));
        }
        static NO_DISCARD INLINE Real EaseOutExpo(const Real& _x) noexcept
        {
            constexpr Real _one = CAST(Real, 1.0);
            return (_x == _one) ? _one : _one - Pow(2.0, CAST(int, -CAST(Real, 10.0) * _x));
        }
        static NO_DISCARD INLINE Real EaseInOutExpo(const Real& _x) noexcept
        {
            constexpr Real _zero = CAST(Real, 0.0);
            constexpr Real _one = CAST(Real, 1.0);
            constexpr Real _two = CAST(Real, 2.0);
            constexpr Real _ten = CAST(Real, 10.0);

            if (_x == _zero) return _zero;
            if (_x == _one) return _one;
            return (_x < 0.5) ? Pow(2.0, CAST(int, _two * _ten * _x - _ten)) / _two
                : (_two - Pow(2.0, CAST(int, -_two * _ten * _x + _ten))) / _two;
        }

        static NO_DISCARD INLINE Real EaseInCirc(const Real& _x) noexcept
        {
            constexpr Real _one = CAST(Real, 1.0);
            return _one - Sqrt(_one - _x * _x);
        }
        static NO_DISCARD INLINE Real EaseOutCirc(const Real& _x) noexcept
        {
            constexpr Real _one = CAST(Real, 1.0);
            return Sqrt(_one - Pow(_x - _one, 2));
        }
        static NO_DISCARD INLINE Real EaseInOutCirc(const Real& _x) noexcept
        {
            constexpr Real _one = CAST(Real, 1.0);
            constexpr Real _two = CAST(Real, 2.0);

            return (_x < CAST(Real, 0.5)) ? (_one - Sqrt(_one - Pow(_two * _x, 2))) / _two
                : (Sqrt(_one - Pow(-_two * _x + _two, 2)) + _one) / _two;
        }

        static NO_DISCARD INLINE Real EaseInBack(const Real& _x) noexcept
        {
            constexpr Real _c1 = CAST(Real, 1.70158);
            constexpr Real _c3 = _c1 + CAST(Real, 1.0);
            return _c3 * _x * _x * _x - _c1 * _x * _x;
        }
        static NO_DISCARD INLINE Real EaseOutBack(const Real& _x) noexcept
        {
            constexpr Real _one = CAST(Real, 1.0);

            constexpr Real _c1 = CAST(Real, 1.70158);
            constexpr Real _c3 = _c1 + _one;
            return _one + _c3 * Pow(_x - _one, 3) + _c1 * Pow(_x - _one, 2);
        }
        static NO_DISCARD INLINE Real EaseInOutBack(const Real& _x) noexcept
        {
            constexpr Real _one = CAST(Real, 1.0);
            constexpr Real _two = CAST(Real, 2.0);

            constexpr Real _c1 = CAST(Real, 1.70158);
            constexpr Real _c2 = _c1 * CAST(Real, 1.525);

            return (_x < CAST(Real, 0.5))
                ? (Pow(_two * _x, 2) * ((_c2 + _one) * _two * _x - _c2)) / _two
                : (Pow(_two * _x - _two, 2) * ((_c2 + _one) * (_two * _x - _two) + _c2) + _two) / _two;
        }

        static NO_DISCARD INLINE Real EaseInElastic(const Real& _x) noexcept
        {
            constexpr Real _zero = CAST(Real, 0.0);
            constexpr Real _one = CAST(Real, 1.0);
            constexpr Real _ten = CAST(Real, 10.0);
            constexpr Real _twoPiOverThree = (CAST(Real, 2.0) * pi) / CAST(Real, 3.0);

            if (_x == _zero) return _zero;
            if (_x == _one)  return _one;
            return -Pow(CAST(Real, 2.0), CAST(int, _ten * _x - _ten)) * Sin((_x * _ten - CAST(Real, 10.75)) * _twoPiOverThree);
        }
        static NO_DISCARD INLINE Real EaseOutElastic(const Real& _x) noexcept
        {
            constexpr Real _zero = CAST(Real, 0.0);
            constexpr Real _one = CAST(Real, 1.0);
            constexpr Real _ten = CAST(Real, 10.0);
            constexpr Real _twoPiOverThree = (CAST(Real, 2.0) * pi) / CAST(Real, 3.0);

            if (_x == _zero) return _zero;
            if (_x == _one)  return _one;
            return Pow(CAST(Real, 2.0), CAST(int, -_ten * _x)) * Sin((_x * _ten - CAST(Real, 0.75)) * _twoPiOverThree) + _one;
        }
        static NO_DISCARD INLINE Real EaseInOutElastic(const Real& _x) noexcept
        {
            constexpr Real _zero = CAST(Real, 0.0);
            constexpr Real _one = CAST(Real, 1.0);
            constexpr Real _two = CAST(Real, 2.0);
            constexpr Real _ten = CAST(Real, 10.0);
            constexpr Real _twoPiOver4_5 = (CAST(Real, 2.0) * pi) / CAST(Real, 4.5);

            if (_x == _zero) return _zero;
            if (_x == _one)  return _one;
            if (_x < CAST(Real, 0.5))
                return -(Pow(_two, CAST(int, _two * _ten * _x - _ten)) * Sin((_two * _x - CAST(Real, 1.1125)) * _twoPiOver4_5)) / _two;
            else
                return (Pow(_two, CAST(int, -_two * _ten * _x + _ten)) * Sin((_two * _x - CAST(Real, 1.1125)) * _twoPiOver4_5)) / _two + _one;
        }

        static NO_DISCARD INLINE Real EaseOutBounce(const Real& _x) noexcept
        {
            constexpr Real _n1 = CAST(Real, 7.5625);
            constexpr Real _d1 = CAST(Real, 2.75);
            constexpr Real _threeHalves = CAST(Real, 1.5);
            constexpr Real _twoPoint625 = CAST(Real, 2.625);

            if (_x < CAST(Real, 1.0) / _d1) return _n1 * _x * _x;
            else if (_x < CAST(Real, 2.0) / _d1) return _n1 * (_x - _threeHalves / _d1) * (_x - _threeHalves / _d1) + CAST(Real, 0.75);
            else if (_x < CAST(Real, 2.5) / _d1) return _n1 * (_x - _twoPoint625 / _d1) * (_x - _twoPoint625 / _d1) + CAST(Real, 0.9375);
            else return _n1 * (_x - _twoPoint625 / _d1) * (_x - _twoPoint625 / _d1) + CAST(Real, 0.984375);
        }
        static NO_DISCARD INLINE Real EaseInBounce(const Real& _x) noexcept
        {
            constexpr Real _one = CAST(Real, 1.0);
            return _one - EaseOutBounce(_one - _x);
        }
        static NO_DISCARD INLINE Real EaseInOutBounce(const Real& _x) noexcept
        {
            constexpr Real _one = CAST(Real, 1.0);
            constexpr Real _two = CAST(Real, 2.0);
            return (_x < CAST(Real, 0.5))
                ? (_one - EaseOutBounce(_one - _two * _x)) / _two
                : (_one + EaseOutBounce(_two * _x - _one)) / _two;
        }


        #pragma endregion
	};

    using FMath = Krampus::Math<float>;
    using IMath = Krampus::Math<int>;
    using UMath = Krampus::Math<unsigned int>;
    using DMath = Krampus::Math<double>;
    using LMath = Krampus::Math<long>;
    using SMath = Krampus::Math<short>;
}
