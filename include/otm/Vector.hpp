#pragma once
#include <algorithm>
#include <compare>
#include <functional>
#include <iterator>
#include <ostream>

namespace otm
{
	template <class T, size_t L>
	struct Vector;

	using Vec2 = Vector<float, 2>;
	using Vec3 = Vector<float, 3>;
	using Vec4 = Vector<float, 4>;

	template <class T, size_t L>
	struct UnitVec;

	using UVec2 = UnitVec<float, 2>;
	using UVec3 = UnitVec<float, 3>;
	using UVec4 = UnitVec<float, 4>;

	namespace detail
	{
		template <class T, size_t L>
		struct VecBase
		{
			template <class... Args>
			constexpr VecBase(Args... args) noexcept: data{args...} {}

			T data[L];
		};

		template <class T>
		struct VecBase<T, 2>
		{
			template <class... Args>
			constexpr VecBase(Args... args) noexcept: data{args...} {}

			union
			{
				T data[2];
				struct { T x, y; };
			};
		};

		template <class T>
		struct VecBase<T, 3>
		{
			template <class... Args>
			constexpr VecBase(Args... args) noexcept: data{args...} {}

			union
			{
				T data[3];
				struct { T x, y, z; };
			};
		};

		template <class T>
		struct VecBase<T, 4>
		{
			template <class... Args>
			constexpr VecBase(Args... args) noexcept: data{args...} {}

			union
			{
				T data[4];
				struct { T x, y, z, w; };
			};
		};
	}

	template <class T, size_t L>
	struct Vector : detail::VecBase<T, L>
	{
	private:
		using Base = detail::VecBase<T, L>;

	public:
		using value_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using reference = T&;
		using const_reference = const T&;
		using pointer = T*;
		using const_pointer = const T*;
		struct iterator;
		struct const_iterator;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		static constexpr Vector One() noexcept
		{
			Vector t;
			t.Transform([](auto&&...) { return static_cast<T>(1); });
			return t;
		}

		constexpr Vector() noexcept = default;
		explicit constexpr Vector(T x) noexcept: Base{x} {}

		template <class... Args>
		constexpr Vector(T x, T y, Args... args) noexcept :Base{x, y, static_cast<T>(args)...} {}

		template <class U, size_t M>
		explicit constexpr Vector(const Vector<U, M>& v) noexcept
		{
			for (size_t i = 0; i < std::min(L, M); ++i)
				(*this)[i] = static_cast<T>(v[i]);
		}

		template <class U, size_t M, class... Args>
		constexpr Vector(const Vector<U, M>& v, T arg, Args... args) noexcept
			:Vector{v}
		{
			static_assert(sizeof...(Args) < std::max<ptrdiff_t>(L - M, 0), "Too many arguments");
			(((begin() + M) << arg) << ... << static_cast<T>(args));
		}

		[[nodiscard]] constexpr T LenSqr() const noexcept { return *this | *this; }
		[[nodiscard]] auto Len() const noexcept { return std::sqrt(static_cast<std::common_type_t<float, T>>(LenSqr())); }

		[[nodiscard]] constexpr T DistSqr(const Vector& v) const noexcept { return (*this - v).LenSqr(); }
		[[nodiscard]] auto Dist(const Vector& v) const noexcept { return (*this - v).Len(); }

		void Normalize() noexcept { *this /= Len(); }
		[[nodiscard]] auto Unit() const noexcept;

		constexpr T& operator[](size_t i) noexcept { return this->data[i]; }
		constexpr T operator[](size_t i) const noexcept { return this->data[i]; }

		constexpr bool operator==(const Vector& other) const noexcept
		{
			return std::equal(this->data, this->data + L, other.data);
		}

		template <class Fn>
		constexpr Vector& Transform(const Vector& other, Fn&& fn) noexcept(std::is_nothrow_invocable_v<Fn, T, T>)
		{
			for (size_t i = 0; i < L; ++i)
				(*this)[i] = fn((*this)[i], other[i]);
			
			return *this;
		}

		template <class Fn>
		constexpr Vector& Transform(Fn&& fn) noexcept(std::is_nothrow_invocable_v<Fn, T>)
		{
			for (size_t i = 0; i < L; ++i)
				(*this)[i] = fn((*this)[i]);

			return *this;
		}

		constexpr void Negate() noexcept { Transform(std::negate<>{}); }
		constexpr Vector operator-() const noexcept
		{
			auto v = *this;
			v.Negate();
			return v;
		}

		constexpr Vector& operator+=(const Vector& v) noexcept
		{
			return Transform(v, std::plus<>{});
		}

		constexpr Vector& operator-=(const Vector& v) noexcept
		{
			return Transform(v, std::minus<>{});
		}

		constexpr Vector& operator*=(const Vector& v) noexcept
		{
			return Transform(v, std::multiplies<>{});
		}

		constexpr Vector& operator*=(T f) noexcept
		{
			return Transform([f](T v) { return v * f; });
		}

		constexpr Vector& operator/=(T f) noexcept
		{
			return Transform([f](T v) { return v / f; });
		}

		constexpr Vector operator+(const Vector& v) const noexcept { return Vector{*this} += v; }
		constexpr Vector operator-(const Vector& v) const noexcept { return Vector{*this} -= v; }

		template <class U>
		constexpr auto operator*(const Vector<U, L>& v) const noexcept
		{
			Vector<std::common_type_t<T, U>, L> r;
			for (size_t i = 0; i < L; ++i) r[i] = (*this)[i] * v[i];
			return r;
		}

		template <class U>
		constexpr auto operator*(U f) const noexcept
		{
			using V = std::common_type_t<T, U>;
			Vector<V, L> v = *this;
			v *= static_cast<V>(f);
			return v;
		}

		template <class U>
		constexpr auto operator/(U f) const noexcept
		{
			using V = std::common_type_t<T, U>;
			Vector<V, L> v = *this;
			v /= static_cast<V>(f);
			return v;
		}

		constexpr T operator|(const Vector& v) const noexcept
		{
			T t{};
			for (size_t i = 0; i < L; ++i) t += (*this)[i] * v[i];
			return t;
		}

		constexpr iterator operator<<(T v) noexcept { return begin() << v; }

		[[nodiscard]] constexpr iterator begin() noexcept { return this->data; }
		[[nodiscard]] constexpr const_iterator begin() const noexcept { return this->data; }
		[[nodiscard]] constexpr const_iterator cbegin() const noexcept { return this->data; }

		[[nodiscard]] constexpr iterator end() noexcept { return this->data + L; }
		[[nodiscard]] constexpr const_iterator end() const noexcept { return this->data + L; }
		[[nodiscard]] constexpr const_iterator cend() const noexcept { return this->data + L; }
		
		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }
		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{end()}; }
		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{cend()}; }

		[[nodiscard]] constexpr reverse_iterator rend() noexcept { return reverse_iterator{begin()}; }
		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator{begin()}; }
		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator{cbegin()}; }

		struct const_iterator
		{
			using iterator_category = std::contiguous_iterator_tag;
			using value_type = T;
			using difference_type = ptrdiff_t;
			using pointer = const T*;
			using reference = T;

			constexpr const_iterator() noexcept = default;

			constexpr explicit operator pointer() const noexcept { return ptr; }

			constexpr reference operator*() const noexcept { return *ptr; }
			constexpr reference operator[](difference_type n) const noexcept { return ptr[n]; }
			constexpr pointer operator->() const noexcept { return ptr; }

			constexpr const_iterator& operator++() noexcept
			{
				++ptr;
				return *this;
			}

			constexpr const_iterator operator++(int) noexcept
			{
				const_iterator it = *this;
				++ptr;
				return it;
			}

			constexpr const_iterator& operator--() noexcept
			{
				--ptr;
				return *this;
			}

			constexpr const_iterator operator--(int) noexcept
			{
				const_iterator it = *this;
				--ptr;
				return it;
			}

			constexpr const_iterator& operator+=(difference_type n) noexcept
			{
				ptr += n;
				return *this;
			}

			constexpr const_iterator operator+(difference_type n) const noexcept { return const_iterator{ptr + n}; }

			constexpr const_iterator& operator-=(difference_type n) noexcept
			{
				ptr -= n;
				return *this;
			}

			constexpr const_iterator operator-(difference_type n) const noexcept { return const_iterator{ptr - n}; }
			constexpr difference_type operator-(const const_iterator& rhs) const noexcept { return ptr - rhs.ptr; }
			constexpr auto operator<=>(const const_iterator& it) const noexcept = default;
			constexpr const_iterator& operator>>(value_type& v) noexcept { v = **this; return ++*this; }

		protected:
			friend Vector;
			constexpr explicit const_iterator(T* data) noexcept: ptr{data} {}
			T* ptr = nullptr;
		};

		struct iterator : const_iterator
		{
			using iterator_category = std::contiguous_iterator_tag;
			using value_type = T;
			using difference_type = ptrdiff_t;
			using pointer = T*;
			using reference = T&;

			constexpr iterator() noexcept = default;

			constexpr explicit operator pointer() const noexcept { return this->ptr; }

			constexpr reference operator*() const noexcept { return *this->ptr; }
			constexpr reference operator[](difference_type n) const noexcept { return this->ptr[n]; }
			constexpr pointer operator->() const noexcept { return this->ptr; }

			constexpr iterator& operator++() noexcept
			{
				++this->ptr;
				return *this;
			}

			constexpr iterator operator++(int) noexcept
			{
				iterator it = *this;
				++this->ptr;
				return it;
			}

			constexpr iterator& operator--() noexcept
			{
				--this->ptr;
				return *this;
			}

			constexpr iterator operator--(int) noexcept
			{
				iterator it = *this;
				--this->ptr;
				return it;
			}

			constexpr iterator& operator+=(difference_type n) noexcept
			{
				this->ptr += n;
				return *this;
			}

			constexpr iterator operator+(difference_type n) const noexcept { return iterator{this->ptr + n}; }

			constexpr iterator& operator-=(difference_type n) noexcept
			{
				this->ptr -= n;
				return *this;
			}

			constexpr iterator operator-(difference_type n) const noexcept { return iterator{this->ptr - n}; }
			constexpr difference_type operator-(const iterator& rhs) const noexcept { return this->ptr - rhs.ptr; }
			constexpr auto operator<=>(const iterator& it) const noexcept = default;

			constexpr iterator& operator<<(const value_type& v) noexcept { **this = v; return ++*this; }
			constexpr iterator& operator<<(value_type&& v) noexcept { **this = std::move(v); return ++*this; }
			constexpr iterator& operator>>(value_type& v) noexcept { v = **this; return ++*this; }

		protected:
			friend Vector;
			constexpr iterator(pointer data) noexcept: const_iterator{data} {}
		};
	};

	template <class... Args>
	Vector(Args...) -> Vector<std::common_type_t<Args...>, sizeof...(Args)>;

	template <class T, size_t L, class... Args>
	Vector(Vector<T, L>, Args...) -> Vector<std::common_type_t<T, Args...>, L + sizeof...(Args)>;
	
	template <class F, class T, size_t L>
	constexpr auto operator*(F f, const Vector<T, L>& v) noexcept
	{
		return v * f;
	}

	template <class T, size_t L>
	std::ostream& operator<<(std::ostream& os, const Vector<T, L>& v)
	{
		os << v[0];
		for (size_t i = 1; i < L; ++i)
			os << ' ' << v[i];
		return os;
	}

	template <class T, size_t L>
	struct UnitVec
	{
		[[nodiscard]] constexpr const Vector<T, L>& Get() const noexcept { return v; }
		
	private:
		friend Vector<T, L>;
		
		explicit UnitVec(const Vector<T, L>& v) noexcept
			:v{v}
		{
		}
		
		const Vector<T, L> v;
	};

	template <class T, size_t L>
	auto Vector<T, L>::Unit() const noexcept
	{
		return UnitVec{*this / Len()};
	}

	template <class T, class U>
	constexpr auto operator^(const Vector<T, 3>& a, const Vector<U, 3>& b) noexcept
	{
		return Vector{a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
	}

	template <class T>
	constexpr Vector<T, 3>& operator^=(Vector<T, 3>& a, const Vector<T, 3>& b) noexcept
	{
		return a = a ^ b;
	}
}