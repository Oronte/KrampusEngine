#pragma once
#include "Math.h"
#include "Utilities/System/Printable.h"

namespace Krampus
{
	template<typename T>
	struct Vector3D : public IPrintable
	{
		static_assert(std::is_arithmetic_v<T>, "Vector2D<T> requires an arithmetic type T");

		using CalcType = std::common_type_t<T, float>;
		using Math = Math<CalcType>;

		T x;
		T y;
		T z;

        constexpr Vector3D()
            : x(0.0f), y(0.0f), z(0.0f) { }

        constexpr explicit Vector3D(T _value)
            : x(_value), y(_value), z(_value) { }

        constexpr Vector3D(T _x, T _y, T _z)
            : x(_x), y(_y), z(_z) { }

        constexpr Vector3D(sf::Vector3<T> _vector)
            : x(_vector.x), y(_vector.y), z(_vector.z) { }

        template<typename U>
        constexpr explicit Vector3D(const Vector3D<U>& _other)
            : x(CAST(T, _other.x)), y(CAST(T, _other.y)), z(CAST(T, _other.z)) { }


        static constexpr Vector3D Zero() { return { 0, 0, 0 }; }
        static constexpr Vector3D One() { return { 1, 1, 1 }; }

        static constexpr Vector3D Right() { return { 1, 0, 0 }; }
        static constexpr Vector3D Left() { return { -1, 0, 0 }; }
        static constexpr Vector3D Up() { return { 0, 1, 0 }; }
        static constexpr Vector3D Down() { return { 0, -1, 0 }; }
        static constexpr Vector3D Forward() { return { 0, 0, 1 }; }
        static constexpr Vector3D Back() { return { 0, 0, -1 }; }


        NO_DISCARD CalcType LengthSquared() const
        {
            return CAST(CalcType, x) * x
                + CAST(CalcType, y) * y
                + CAST(CalcType, z) * z;
        }

        NO_DISCARD CalcType Length() const
        {
            return Math::Sqrt(LengthSquared());
        }

        NO_DISCARD Vector3D Normalized() const
        {
            CalcType _len = Length();
            if (_len == CalcType(0))
                return Vector3D{};
            return Vector3D
            (
                CAST(T, x / _len),
                CAST(T, y / _len),
                CAST(T, z / _len)
            );
        }

        void Normalize()
        {
            *this = Normalized();
        }


        NO_DISCARD static CalcType Dot(const Vector3D& _a, const Vector3D& _b)
        {
            return CAST(CalcType, _a.x) * _b.x
                + CAST(CalcType, _a.y) * _b.y
                + CAST(CalcType, _a.z) * _b.z;
        }

        NO_DISCARD static Vector3D Cross(const Vector3D& _a, const Vector3D& _b)
        {
            return 
            {
                _a.y * _b.z - _a.z * _b.y,
                _a.z * _b.x - _a.x * _b.z,
                _a.x * _b.y - _a.y * _b.x
            };
        }

        NO_DISCARD static CalcType DistanceSquared(const Vector3D& _a, const Vector3D& _b)
        {
            return (_a - _b).LengthSquared();
        }

        NO_DISCARD static CalcType Distance(const Vector3D& _a, const Vector3D& _b)
        {
            return Math::Sqrt(DistanceSquared(_a, _b));
        }


        static Vector3D Clamp(const Vector3D& _vector, const Vector3D& _min, const Vector3D& _max)
        {
            return 
            {
                Math::Clamp(_vector.x, _min.x, _max.x),
                Math::Clamp(_vector.y, _min.y, _max.y),
                Math::Clamp(_vector.z, _min.z, _max.z)
            };
        }
    

        constexpr Vector3D ComponentWiseMul(const Vector3D& _vector) const
        {
            return 
            {
                x * _vector.x,
                y * _vector.y,
                z * _vector.z
            };
        }

        constexpr Vector3D ComponentWiseDiv(const Vector3D& _vector) const
        {
            return 
            {
                x / _vector.x,
                y / _vector.y,
                z / _vector.z
            };
        }

        static constexpr Vector3D ComponentWiseMul(const Vector3D& _a, const Vector3D& _b)
        {
            return 
            {
                _a.x * _b.x,
                _a.y * _b.y,
                _a.z * _b.z
            };
        }

        static constexpr Vector3D ComponentWiseDiv(const Vector3D& _a, const Vector3D& _b)
        {
            return 
            {
                _a.x / _b.x,
                _a.y / _b.y,
                _a.z / _b.z
            };
        }


        #pragma region operator

        constexpr Vector3D operator+(const Vector3D& _vector) const
        {
            return { x + _vector.x, y + _vector.y, z + _vector.z };
        }

        constexpr Vector3D operator-(const Vector3D& _vector) const
        {
            return { x - _vector.x, y - _vector.y, z - _vector.z };
        }

        constexpr Vector3D operator*(T _scalar) const
        {
            return { x * _scalar, y * _scalar, z * _scalar };
        }

        constexpr Vector3D operator/(T _scalar) const
        {
            return { x / _scalar, y / _scalar, z / _scalar };
        }

        constexpr Vector3D& operator+=(const Vector3D& _vector)
        {
            x += _vector.x; y += _vector.y; z += _vector.z;
            return *this;
        }

        constexpr Vector3D& operator-=(const Vector3D& _vector)
        {
            x -= _vector.x; y -= _vector.y; z -= _vector.z;
            return *this;
        }

        constexpr Vector3D& operator*=(T _scalar)
        {
            x *= _scalar; y *= _scalar; z *= _scalar;
            return *this;
        }

        constexpr Vector3D& operator/=(T _scalar)
        {
            x /= _scalar; y /= _scalar; z /= _scalar;
            return *this;
        }

        constexpr Vector3D operator-() const
        {
            return { -x, -y, -z };
        }

        constexpr bool operator==(const Vector3D& _vector) const
        {
            return x == _vector.x && y == _vector.y && z == _vector.z;
        }

        constexpr bool operator!=(const Vector3D& _vector) const
        {
            return !(*this == _vector);
        }

        operator sf::Vector3<T>() const noexcept
        {
            return { x, y, z };
        }

#pragma endregion

        std::string ToString() const override
        {
            return std::format("X : {}, Y : {}, Z : {}", x, y, z);
        }
};

    template<typename T>
    constexpr Vector3D<T> operator*(T _scalar, const Vector3D<T>& _vector)
    {
        return _vector * _scalar;
    }

    using FVector3 = Vector3D<float>;
    using IVector3 = Vector3D<int>;
    using UVector3 = Vector3D<unsigned int>;
    using SVector3 = Vector3D<short>;
    using DVector3 = Vector3D<double>;
}