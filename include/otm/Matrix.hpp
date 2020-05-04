#pragma once
#include "Vector.hpp"
#include "Quat.hpp"

namespace otm
{
	template <class T, size_t R, size_t C = R>
	struct Matrix;

	using Mat2 = Matrix<float, 2>;
	using Mat3 = Matrix<float, 3>;
	using Mat4 = Matrix<float, 4>;

	namespace detail
	{
		template <class T, size_t R, size_t C>
		struct MatrixBase {};

		template <>
		struct MatrixBase<float, 4, 4>
		{
			static constexpr Mat4 Translation(const Vec3& pos) noexcept;
			static constexpr Mat4 Rotation(const Quat& rot) noexcept;
			static constexpr Mat4 Scale(const Vec3& scale) noexcept;
		};
	}

	template <class T, size_t R, size_t C>
	struct Matrix : detail::MatrixBase<T, R, C>
	{
		static constexpr Matrix Identity() noexcept
		{
			Matrix matrix;
			for (size_t i = 0; i < std::min(R, C); ++i)
				matrix[i][i] = 1;
			return matrix;
		}

		static constexpr Matrix SimpleViewProj(const Vec2& scr) noexcept
		{
			auto proj = Mat4::Identity();
			proj[0][0] = 2 / scr.x;
			proj[1][1] = 2 / scr.y;
			return proj;
		}

		template <class... Args>
		constexpr Matrix(Args... args) noexcept
			: arr{static_cast<T>(args)...}
		{
		}

		constexpr bool operator==(const Matrix&) const noexcept = default;

		constexpr auto& operator[](size_t i) noexcept { return m[i]; }
		constexpr auto& operator[](size_t i) const noexcept { return m[i]; }
		constexpr auto& Row(size_t i) noexcept { return m[i]; }
		[[nodiscard]] constexpr auto& Row(size_t i) const noexcept { return m[i]; }

		[[nodiscard]] constexpr auto Col(size_t c) const noexcept
		{
			Vector<T, R> v;
			for (size_t r = 0; r < R; ++r) v[r] = m[r][c];
			return v;
		}

		[[nodiscard]] constexpr auto& AsVectors() noexcept { return m; }
		[[nodiscard]] constexpr auto& AsVectors() const noexcept { return m; }
		[[nodiscard]] constexpr auto& AsFloats() noexcept { return flat; }
		[[nodiscard]] constexpr auto& AsFloats() const noexcept { return flat; }

		constexpr Matrix operator+(const Matrix& b) const noexcept
		{
			auto c = *this;
			return c += b;
		}

		constexpr Matrix& operator+=(const Matrix& b) noexcept
		{
			for (auto i = 0; i < R; ++i) m[i] += b[i];
			return *this;
		}

		constexpr Matrix operator-(const Matrix& b) const noexcept
		{
			auto c = *this;
			return c -= b;
		}

		constexpr Matrix& operator-=(const Matrix& b) noexcept
		{
			for (auto i = 0; i < R; ++i) m[i] -= b[i];
			return *this;
		}

		constexpr Matrix operator*(T f) const noexcept
		{
			auto c = *this;
			return c *= f;
		}

		constexpr Matrix& operator*=(T f) noexcept
		{
			for (auto i = 0; i < R; ++i) m[i] *= f;
			return *this;
		}

		constexpr Matrix operator/(T f) const noexcept
		{
			auto c = *this;
			return c /= f;
		}

		constexpr Matrix& operator/=(T f) noexcept
		{
			for (auto i = 0; i < R; ++i) m[i] /= f;
			return *this;
		}

		template <size_t C2>
		constexpr Matrix<T, R, C2> operator*(const Matrix<T, C, C2>& b) const noexcept
		{
			Matrix<T, R, C2> c;
			for (size_t i = 0; i < R; ++i)
				for (size_t j = 0; j < C2; ++j)
					c[i][j] = Row(i) | b.Col(j);
			return c;
		}

		constexpr Matrix& operator*=(const Matrix& b) noexcept { return *this = *this * b; }

		[[nodiscard]] constexpr Matrix<T, C, R> Transpose() const noexcept
		{
			Matrix<T, C, R> t;
			for (auto i = 0; i < R; ++i)
				for (auto j = 0; j < C; ++j)
					t[j][i] = m[i][j];
			return t;
		}

	private:
		union
		{
			Vector<T, C> m[R];
			T arr[R][C];
			T flat[R*C];
		};
	};

	template <class F, class T, size_t R, size_t C>
	constexpr auto operator*(F f, const Matrix<T, R, C>& m) noexcept { return m * f; }

	template <class T, size_t R, size_t C>
	std::ostream& operator<<(std::ostream& os, const Matrix<T, R, C>& m)
	{
		os << m[0];
		for (size_t i = 1; i < R; ++i)
			os << '\n' << m[i];
		return os;
	}

	namespace detail
	{
		constexpr Mat4 MatrixBase<float, 4, 4>::Translation(const Vec3& pos) noexcept
		{
			auto t = Mat4::Identity();
			t[3] << pos.x << pos.y << pos.z;
			return t;
		}

		constexpr Mat4 MatrixBase<float, 4, 4>::Rotation(const Quat& rot) noexcept
		{
			
		}

		constexpr Mat4 MatrixBase<float, 4, 4>::Scale(const Vec3& scale) noexcept
		{
			auto s = Mat4::Identity();
			s[0][0] = scale.x;
			s[1][1] = scale.y;
			s[2][2] = scale.z;
			return s;
		}
	}
}