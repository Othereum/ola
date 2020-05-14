#pragma once
#include "Vector.hpp"

namespace otm
{
	template <std::floating_point T>
	struct Quaternion
	{
		union
		{
			struct { Vector<T, 3> v; T s; };
			struct { T x, y, z, w; };
			Vector<T, 4> v4;
			T data[4];
		};

		constexpr Quaternion() noexcept: v{}, s{1} {}
		constexpr Quaternion(const Vector<T, 3>& v, T s) noexcept: v{v}, s{s} {}
		constexpr Quaternion(T x, T y, T z, T w) noexcept: x{x}, y{y}, z{z}, w{w} {}
		explicit constexpr Quaternion(const Vector<T, 4>& v4) noexcept: v4{v4} {}
		
		Quaternion(const UnitVec<T, 3>& axis, Angle<RadR, T> angle) noexcept
			:Quaternion{axis.Get() * Sin(angle / 2), Cos(angle / 2)}
		{
		}

		constexpr Quaternion& operator*=(const Quaternion& q) noexcept { return *this = *this * q; }
		constexpr Quaternion operator*(const Quaternion& q) const noexcept
		{
			return {s*q.v + q.s*v + (v^q.v), s*q.s - (v|q.v)};
		}

		constexpr void Invert() noexcept { v.Negate(); }
		constexpr Quaternion operator~() const noexcept { return {-v, s}; }
	};

	template <std::floating_point T>
	Vector<T, 3> detail::VecBase2<T, 3>::Rotated(const Quaternion<T>& q) const noexcept
	{
		return (q * Quaternion<T>{static_cast<const Vector<T, 3>&>(*this), 0} * ~q).v;
	}

	template <std::floating_point T>
	void detail::VecBase2<T, 3>::Rotate(const Quaternion<T>& q) noexcept
	{
		return *this = Rotated(q);
	}
}
