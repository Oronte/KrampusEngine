#pragma once
#include "Utilities/Macro.h"

namespace Krampus
{

    template<typename Real>
	class Math
	{
        static_assert(std::is_arithmetic_v<Real>, "Math<Real> requires an arithmetic type Real");

    public:
        static constexpr Real pi = static_cast<Real>(3.14159265358979323846);
        static constexpr Real tau = static_cast<Real>(2.0 * 3.14159265358979323846);
        static constexpr Real halfPi = static_cast<Real>(1.57079632679489661923);
        static constexpr Real degToRad = static_cast<Real>(pi / 180.0);
        static constexpr Real radToDeg = static_cast<Real>(180.0 / pi);
        static constexpr Real epsilon = static_cast<Real>(1e-6);
        static constexpr Real infinity = std::numeric_limits<Real>::infinity();
        static constexpr Real golden = static_cast<Real>(1.6180339887498948);

    
        #pragma region Common Functions

        static Real Sqrt(const Real& _x) noexcept
        {
            assert(_x >= static_cast<Real>(0) && "Sqrt: negative input");
            return static_cast<Real>(std::sqrt(_x));
        }
        static Real InvSqrt(const Real& _x) noexcept
        {
            assert(_x > static_cast<Real>(0) && "InvSqrt: input must be > 0");
            return static_cast<Real>(1) / Sqrt(_x);
        }
        static Real Pow(const Real& _base, const int& _exp) noexcept
        {
            return static_cast<Real>(std::pow(_base, _exp));
        }
        static Real Exp(const Real& _x) noexcept
        {
            return static_cast<Real>(std::exp(_x));
        }
        static Real Log(const Real& _x) noexcept
        {
            assert(_x > static_cast<Real>(0) && "Log: input must be > 0");
            return static_cast<Real>(std::log(_x));
        }
        static Real Log10(const Real& _x) noexcept
        {
            assert(_x > static_cast<Real>(0) && "Log10: input must be > 0");
            return static_cast<Real>(std::log10(_x));
        }

        static Real Sin(const Real& _x) noexcept
        {
            return static_cast<Real>(std::sin(_x));
        }
        static Real Cos(const Real& _x) noexcept
        {
            return static_cast<Real>(std::cos(_x));
        }
        static Real Tan(const Real& _x) noexcept
        {
            return static_cast<Real>(std::tan(_x));
        }

        static Real Asin(const Real& _x) noexcept
        {
            assert(_x >= static_cast<Real>(-1) && _x <= static_cast<Real>(1) && "Asin: input out of [-1, 1]");
            return static_cast<Real>(std::asin(_x));
        }
        static Real Acos(const Real& _x) noexcept
        {
            assert(_x >= static_cast<Real>(-1) && _x <= static_cast<Real>(1) && "Acos: input out of [-1, 1]");
            return static_cast<Real>(std::acos(_x));
        }
        static Real Atan(const Real& _x) noexcept
        {
            return static_cast<Real>(std::atan(_x));
        }
        static Real Atan2(const Real& _y, const Real& _x) noexcept
        {
            return static_cast<Real>(std::atan2(_y, _x));
        }

        static Real Sinh(const Real& _x) noexcept
        {
            return static_cast<Real>(std::sinh(_x));
        }
        static Real Cosh(const Real& _x) noexcept
        {
            return static_cast<Real>(std::cosh(_x));
        }
        static Real Tanh(const Real& _x) noexcept
        {
            return static_cast<Real>(std::tanh(_x));
        }

        static Real Floor(const Real& _x) noexcept
        {
            return static_cast<Real>(std::floor(_x));
        }
        static Real Ceil(const Real& _x) noexcept
        {
            return static_cast<Real>(std::ceil(_x));
        }
        static Real Round(const Real& _x) noexcept
        {
            return static_cast<Real>(std::round(_x));
        }
        static Real Trunc(const Real& _x) noexcept
        {
            return static_cast<Real>(std::trunc(_x));
        }


        /// <summary>Converts degrees to radians.</summary>
        static constexpr Real DegToRad(const Real& _degrees) noexcept
        {
            return _degrees * degToRad;
        }

        /// <summary>Converts radians to degrees.</summary>
        static constexpr Real RadToDeg(const Real& _radian) noexcept
        {
            return _radian * radToDeg;
        }

        /// <summary>Returns the absolute value of a scalar.</summary>
        static constexpr Real Abs(const Real& _value) noexcept
        {
            return _value < 0 ? -_value : _value;
        }

        /// <summary>Returns the sign of a value (-1, 0, or 1).</summary>
        static constexpr Real Sign(const Real& _value) noexcept
        {
            return static_cast<Real>((_value > 0)) - static_cast<Real>((_value < 0));
        }

        static constexpr Real MinVal(const Real& _a, const Real& _b) noexcept
        {
            return _a < _b ? _a : _b;
        }

        static constexpr Real MaxVal(const Real& _a, const Real& _b) noexcept
        {
            return _a > _b ? _a : _b;
        }

        /// <summary>Clamps a value between a minimum and maximum range.</summary>
        static constexpr Real Clamp(const Real& _value, const Real& _min, const Real& _max) noexcept
        {
            return _value < _min ? _min : _value > _max ? _max : _value;
        }

        /// <summary>Clamps a value between 0 and 1.</summary>
        static constexpr Real Saturate(const Real& _value) noexcept
        {
            return Clamp(_value, static_cast<Real>(0), static_cast<Real>(1));
        }

        /// <summary>Returns true if two values are approximately equal (within epsilon).</summary>
        static constexpr bool ApproxEqual(const Real& _a, const Real& _b, const Real& _eps = epsilon) noexcept
        {
            return Abs(_a - _b) <= _eps;
        }

        #pragma endregion


        #pragma region Interpolation

        /// <summary>Linearly interpolates between two values.</summary>
        static constexpr Real Lerp(const Real& _a, const Real& _b, const Real& _time) noexcept
        {
            return _a + (_b - _a) * _time;
        }

        static constexpr Real ConstantLerp(const Real& _start, const Real& _end, const Real& _time)
        {
            return (1 - _time) * _start + _time * _end;
        }

        /// <summary>Inverse linear interpolation (returns how far _v is between _a and _b).</summary>
        static constexpr Real InverseLerp(const Real& _a, const Real& _b, const Real& _v) noexcept
        {
            return (_b - _a) == 0 ? 0 : (_v - _a) / (_b - _a);
        }

        /// <summary>Remaps a value from one range to another.</summary>
        static constexpr Real Remap(const Real& _inMin, const Real& _inMax, const Real& _outMin, const Real& _outMax, const Real& _v) noexcept
        {
            return Lerp(_outMin, _outMax, Saturate(InverseLerp(_inMin, _inMax, _v)));
        }

        /// <summary>Returns a smooth interpolation between 0 and 1 based on x (Hermite curve).</summary>
        static Real SmoothStep(const Real& _edge0, const Real& _edge1, Real _x) noexcept
        {
            _x = Saturate((_x - _edge0) / (_edge1 - _edge0));
            return _x * _x * (3 - 2 * _x);
        }

        /// <summary>Returns a smoother interpolation (cubic curve) between 0 and 1.</summary>
        static Real SmootherStep(const Real& _edge0, const Real& _edge1, Real _x) noexcept
        {
            _x = Saturate((_x - _edge0) / (_edge1 - _edge0));
            return _x * _x * _x * (_x * (_x * 6 - 15) + 10);
        }

        #pragma endregion


        #pragma region Random

        static [[nodiscard]] Real RandomRange(const Real& _min, const Real& _max)
        {
            static thread_local std::mt19937 _generator{
                    []()
                    {
                        std::random_device _rd;
                        std::seed_seq _seed{
                            _rd(),
                            _rd(),
                            _rd(),
                            _rd()
                        };
                        return std::mt19937(_seed);
                    }()
            };

            if constexpr (std::is_integral_v<Real>)
            {
                std::uniform_int_distribution<Real> _dist(_min, _max);
                return _dist(_generator);
            }
            else
            {
                std::uniform_real_distribution<Real> _dist(_min, _max);
                return _dist(_generator);
            }
        }

        static [[nodiscard]] Real RandomRange(const Real& _min, const Real& _max, uint32_t seed)
        {
            thread_local std::mt19937 _generator(seed);
            if constexpr (std::is_integral_v<Real>)
            {
                std::uniform_int_distribution<Real> _dist(_min, _max);
                return _dist(_generator);
            }
            else
            {
                std::uniform_real_distribution<Real> _dist(_min, _max);
                return _dist(_generator);
            }
        }

        #pragma endregion


        #pragma region Easing curves


        [[nodiscard]] static Real Linear(const Real& _x) noexcept
        {
            return _x;
        }

        [[nodiscard]] static Real EaseInSine(const Real& _x) noexcept
        {
            return static_cast<Real>(1.0) - Cos((_x * pi) / static_cast<Real>(2.0));
        }
        [[nodiscard]] static Real EaseOutSine(const Real& _x) noexcept
        {
            return Sin((_x * pi) / static_cast<Real>(2.0));
        }
        [[nodiscard]] static Real EaseInOutSine(const Real& _x) noexcept
        {
            return -static_cast<Real>(0.5) * (Cos(pi * _x) - static_cast<Real>(1.0));
        }

        [[nodiscard]] static Real EaseInQuad(const Real& _x) noexcept
        {
            return _x * _x;
        }
        [[nodiscard]] static Real EaseOutQuad(const Real& _x) noexcept
        {
            constexpr Real _one = static_cast<Real>(1.0);
            return _one - (_one - _x) * (_one - _x);
        }
        [[nodiscard]] static Real EaseInOutQuad(const Real& _x) noexcept
        {
            constexpr Real _two = static_cast<Real>(2.0);
            return (_x < static_cast<Real>(0.5)) ?
                _two * _x * _x :
                static_cast<Real>(1.0) - Pow(-_two * _x + _two, 2) / _two;
        }

        [[nodiscard]] static Real EaseInCubic(const Real& _x) noexcept
        {
            return _x * _x * _x;
        }
        [[nodiscard]] static Real EaseOutCubic(const Real& _x) noexcept
        {
            constexpr Real _one = static_cast<Real>(1.0);
            return _one - Pow(_one - _x, 3);
        }
        [[nodiscard]] static Real EaseInOutCubic(const Real& _x) noexcept
        {
            constexpr Real _two = static_cast<Real>(2.0);
            return (_x < static_cast<Real>(0.5)) ?
                static_cast<Real>(4.0) * _x * _x * _x :
                static_cast<Real>(1.0) - Pow(-_two * _x + _two, 3) / _two;
        }

        [[nodiscard]] static Real EaseInQuart(const Real& _x) noexcept
        {
            return _x * _x * _x * _x;
        }
        [[nodiscard]] static Real EaseOutQuart(const Real& _x) noexcept
        {
            constexpr Real _one = static_cast<Real>(1.0);
            return _one - Pow(_one - _x, 4);
        }
        [[nodiscard]] static Real EaseInOutQuart(const Real& _x) noexcept
        {
            constexpr Real _two = static_cast<Real>(2.0);
            return (_x < static_cast<Real>(0.5)) ?
                static_cast<Real>(8.0) * _x * _x * _x * _x :
                static_cast<Real>(1.0) - Pow(-_two * _x + _two, 4) / _two;
        }

        [[nodiscard]] static Real EaseInQuint(const Real& _x) noexcept
        {
            return _x * _x * _x * _x * _x;
        }
        [[nodiscard]] static Real EaseOutQuint(const Real& _x) noexcept
        {
            constexpr Real _one = static_cast<Real>(1.0);
            return _one - Pow(_one - _x, 5);
        }
        [[nodiscard]] static Real EaseInOutQuint(const Real& _x) noexcept
        {
            constexpr Real _two = static_cast<Real>(2.0);
            return (_x < static_cast<Real>(0.5)) ?
                static_cast<Real>(16.0) * _x * _x * _x * _x * _x :
                static_cast<Real>(1.0) - Pow(-_two * _x + _two, 5) / _two;
        }

        [[nodiscard]] static Real EaseInExpo(const Real& _x) noexcept
        {
            constexpr Real _zero = static_cast<Real>(0.0);
            constexpr Real _ten = static_cast<Real>(10.0);
            return (_x == _zero) ? _zero : Pow(2.0, static_cast<int>(_ten * _x - _ten));
        }
        [[nodiscard]] static Real EaseOutExpo(const Real& _x) noexcept
        {
            constexpr Real _one = static_cast<Real>(1.0);
            return (_x == _one) ? _one : _one - Pow(2.0, static_cast<int>(-static_cast<Real>(10.0) * _x));
        }
        [[nodiscard]] static Real EaseInOutExpo(const Real& _x) noexcept
        {
            constexpr Real _zero = static_cast<Real>(0.0);
            constexpr Real _one = static_cast<Real>(1.0);
            constexpr Real _two = static_cast<Real>(2.0);
            constexpr Real _ten = static_cast<Real>(10.0);

            if (_x == _zero) return _zero;
            if (_x == _one) return _one;
            return (_x < 0.5) ? Pow(2.0, static_cast<int>(_two * _ten * _x - _ten)) / _two
                : (_two - Pow(2.0, static_cast<int>(-_two * _ten * _x + _ten))) / _two;
        }

        [[nodiscard]] static Real EaseInCirc(const Real& _x) noexcept
        {
            constexpr Real _one = static_cast<Real>(1.0);
            return _one - Sqrt(_one - _x * _x);
        }
        [[nodiscard]] static Real EaseOutCirc(const Real& _x) noexcept
        {
            constexpr Real _one = static_cast<Real>(1.0);
            return Sqrt(_one - Pow(_x - _one, 2));
        }
        [[nodiscard]] static Real EaseInOutCirc(const Real& _x) noexcept
        {
            constexpr Real _one = static_cast<Real>(1.0);
            constexpr Real _two = static_cast<Real>(2.0);

            return (_x < static_cast<Real>(0.5)) ? (_one - Sqrt(_one - Pow(_two * _x, 2))) / _two
                : (Sqrt(_one - Pow(-_two * _x + _two, 2)) + _one) / _two;
        }

        [[nodiscard]] static Real EaseInBack(const Real& _x) noexcept
        {
            constexpr Real _c1 = static_cast<Real>(1.70158);
            constexpr Real _c3 = _c1 + static_cast<Real>(1.0);
            return _c3 * _x * _x * _x - _c1 * _x * _x;
        }
        [[nodiscard]] static Real EaseOutBack(const Real& _x) noexcept
        {
            constexpr Real _one = static_cast<Real>(1.0);

            constexpr Real _c1 = static_cast<Real>(1.70158);
            constexpr Real _c3 = _c1 + _one;
            return _one + _c3 * Pow(_x - _one, 3) + _c1 * Pow(_x - _one, 2);
        }
        [[nodiscard]] static Real EaseInOutBack(const Real& _x) noexcept
        {
            constexpr Real _one = static_cast<Real>(1.0);
            constexpr Real _two = static_cast<Real>(2.0);

            constexpr Real _c1 = static_cast<Real>(1.70158);
            constexpr Real _c2 = _c1 * static_cast<Real>(1.525);

            return (_x < static_cast<Real>(0.5))
                ? (Pow(_two * _x, 2) * ((_c2 + _one) * _two * _x - _c2)) / _two
                : (Pow(_two * _x - _two, 2) * ((_c2 + _one) * (_two * _x - _two) + _c2) + _two) / _two;
        }

        [[nodiscard]] static Real EaseInElastic(const Real& _x) noexcept
        {
            constexpr Real _zero = static_cast<Real>(0.0);
            constexpr Real _one = static_cast<Real>(1.0);
            constexpr Real _ten = static_cast<Real>(10.0);
            constexpr Real _twoPiOverThree = (static_cast<Real>(2.0) * pi) / static_cast<Real>(3.0);

            if (_x == _zero) return _zero;
            if (_x == _one)  return _one;
            return -Pow(static_cast<Real>(2.0), static_cast<int>(_ten * _x - _ten)) * Sin((_x * _ten - static_cast<Real>(10.75)) * _twoPiOverThree);
        }
        [[nodiscard]] static Real EaseOutElastic(const Real& _x) noexcept
        {
            constexpr Real _zero = static_cast<Real>(0.0);
            constexpr Real _one = static_cast<Real>(1.0);
            constexpr Real _ten = static_cast<Real>(10.0);
            constexpr Real _twoPiOverThree = (static_cast<Real>(2.0) * pi) / static_cast<Real>(3.0);

            if (_x == _zero) return _zero;
            if (_x == _one)  return _one;
            return Pow(static_cast<Real>(2.0), static_cast<int>(-_ten * _x)) * Sin((_x * _ten - static_cast<Real>(0.75)) * _twoPiOverThree) + _one;
        }
        [[nodiscard]] static Real EaseInOutElastic(const Real& _x) noexcept
        {
            constexpr Real _zero = static_cast<Real>(0.0);
            constexpr Real _one = static_cast<Real>(1.0);
            constexpr Real _two = static_cast<Real>(2.0);
            constexpr Real _ten = static_cast<Real>(10.0);
            constexpr Real _twoPiOver4_5 = (static_cast<Real>(2.0) * pi) / static_cast<Real>(4.5);

            if (_x == _zero) return _zero;
            if (_x == _one)  return _one;
            if (_x < static_cast<Real>(0.5))
                return -(Pow(_two, static_cast<int>(_two * _ten * _x - _ten)) * Sin((_two * _x - static_cast<Real>(1.1125)) * _twoPiOver4_5)) / _two;
            else
                return (Pow(_two, static_cast<int>(-_two * _ten * _x + _ten)) * Sin((_two * _x - static_cast<Real>(1.1125)) * _twoPiOver4_5)) / _two + _one;
        }

        [[nodiscard]] static Real EaseOutBounce(const Real& _x) noexcept
        {
            constexpr Real _n1 = static_cast<Real>(7.5625);
            constexpr Real _d1 = static_cast<Real>(2.75);
            constexpr Real _threeHalves = static_cast<Real>(1.5);
            constexpr Real _twoPoint625 = static_cast<Real>(2.625);

            if (_x < static_cast<Real>(1.0) / _d1) return _n1 * _x * _x;
            else if (_x < static_cast<Real>(2.0) / _d1) return _n1 * (_x - _threeHalves / _d1) * (_x - _threeHalves / _d1) + static_cast<Real>(0.75);
            else if (_x < static_cast<Real>(2.5) / _d1) return _n1 * (_x - _twoPoint625 / _d1) * (_x - _twoPoint625 / _d1) + static_cast<Real>(0.9375);
            else return _n1 * (_x - _twoPoint625 / _d1) * (_x - _twoPoint625 / _d1) + static_cast<Real>(0.984375);
        }
        [[nodiscard]] static Real EaseInBounce(const Real& _x) noexcept
        {
            constexpr Real _one = static_cast<Real>(1.0);
            return _one - EaseOutBounce(_one - _x);
        }
        [[nodiscard]] static Real EaseInOutBounce(const Real& _x) noexcept
        {
            constexpr Real _one = static_cast<Real>(1.0);
            constexpr Real _two = static_cast<Real>(2.0);
            return (_x < static_cast<Real>(0.5))
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
