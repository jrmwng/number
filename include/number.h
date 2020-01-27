#pragma once

#include <intrin.h>

#include <algorithm>
#include <type_traits>

namespace jrmwng
{
	namespace number
	{
		template <typename Ta, typename Tb>
		struct numbers_traits
		{
		};
		struct add_tag {};
		struct minus_tag {};
		struct multiplies_tag {};
		struct dividies_tag {};

		template <size_t uUNSIGNED_BITS>
		class Cinteger
		{
			friend class Cinteger;
		public:
#ifdef _M_X64
			using unsigned_type = unsigned long long;
#else
			using unsigned_type = unsigned int;
#endif
			template <size_t uACCUMULATOR_BITS>
			using accumulator_type = Cinteger<uUNSIGNED_BITS + uACCUMULATOR_BITS>;
		private:
			enum Econstant
			{
				SIZE = (uUNSIGNED_BITS + (sizeof(unsigned_type) * 8 - 1)) / (sizeof(unsigned_type) * 8)
			};
			unsigned_type m_auValue[SIZE];

#ifdef _M_X64
			static unsigned char addcarry(unsigned char ubCarry, unsigned long long uLeft, unsigned long long uRight, unsigned long long *puResult)
			{
				return _addcarry_u64(ubCarry, uLeft, uRight, puResult);
			}
			static unsigned char subborrow(unsigned char ubBorrow, unsigned long long uLeft, unsigned long long uRight, unsigned long long *puResult)
			{
				return _subborrow_u64(ubBorrow, uLeft, uRight, puResult);
			}
			static unsigned char addcarryx(unsigned char ubCarry, unsigned long long uLeft, unsigned long long uRight, unsigned long long *puResult)
			{
				return _addcarryx_u64(ubCarry, uLeft, uRight, puResult);
			}
			static unsigned long long mulx(unsigned long long uLeft, unsigned long long uRight, unsigned long long *puHigh)
			{
				return _mulx_u64(uLeft, uRight, puHigh);
			}
#else
			static unsigned char addcarry(unsigned char ubCarry, unsigned int uLeft, unsigned int uRight, unsigned int *puResult)
			{
				return _addcarry_u32(ubCarry, uLeft, uRight, puResult);
			}
			static unsigned char subborrow(unsigned char ubBorrow, unsigned int uLeft, unsigned int uRight, unsigned int *puResult)
			{
				return _subborrow_u32(ubBorrow, uLeft, uRight, puResult);
			}
			static unsigned char addcarryx(unsigned char ubCarry, unsigned int uLeft, unsigned int uRight, unsigned int *puResult)
			{
				return _addcarryx_u32(ubCarry, uLeft, uRight, puResult);
			}
			static unsigned int mulx(unsigned int uLeft, unsigned int uRight, unsigned int *puHigh)
			{
				return _mulx_u32(uLeft, uRight, puHigh);
			}
#endif
			template <size_t... uINDEX>
			Cinteger(unsigned_type tValue, std::index_sequence<uINDEX...>)
				: m_auValue{ tValue, (uINDEX, unsigned_type(0))... }
			{
				static_assert(sizeof...(uINDEX) + 1 == SIZE, "requirement of this constructor");
			}
		public:
			Cinteger(unsigned_type tValue = 0)
				: Cinteger(tValue, std::make_index_sequence<SIZE - 1>())
			{
			}

			template <size_t u1, size_t u2>
			Cinteger(Cinteger<u1> const & left, Cinteger<u2> const & right, add_tag)
			{
				unsigned char ubCarry = 0;

				for (size_t uIndex = 0, uIndexEnd = std::min(std::size(left.m_auValue), std::size(right.m_auValue)); uIndex < uIndexEnd; uIndex++)
				{
					ubCarry = addcarry(ubCarry, left.m_auValue[uIndex], right.m_auValue[uIndex], &m_auValue[uIndex]);
				}
				for (size_t uIndex = std::size(left.m_auValue), uIndexEnd = std::size(right.m_auValue); uIndex < uIndexEnd; uIndex++)
				{
					ubCarry = addcarry(ubCarry, 0, right.m_auValue[uIndex], &m_auValue[uIndex]);
				}
				for (size_t uIndex = std::size(right.m_auValue), uIndexEnd = std::size(left.m_auValue); uIndex < uIndexEnd; uIndex++)
				{
					ubCarry = addcarry(ubCarry, left.m_auValue[uIndex], 0, &m_auValue[uIndex]);
				}
				for (size_t uIndex = std::max(std::size(left.m_auValue), std::size(right.m_auValue)), uIndexEnd = std::size(m_auValue); uIndex < uIndexEnd; uIndex++)
				{
					ubCarry = addcarry(ubCarry, 0, 0, &m_auValue[uIndex]);
				}
			}
			template <size_t uTHAT>
			typename Cinteger & operator += (Cinteger<uTHAT> const & that)
			{
				unsigned char ubCarry = 0;

				for (size_t uIndex = 0, uIndexEnd = std::min(std::size(m_auValue), std::size(that.m_auValue)); uIndex < uIndexEnd; uIndex++)
				{
					ubCarry = addcarry(ubCarry, m_auValue[uIndex], that.m_auValue[uIndex], &m_auValue[uIndex]);
				}
				for (size_t uIndex = std::size(that.m_auValue), uIndexEnd = std::size(m_auValue); uIndex < uIndexEnd; uIndex++)
				{
					ubCarry = addcarry(ubCarry, m_auValue[uIndex], 0, &m_auValue[uIndex]);
				}
				if (ubCarry)
				{
					__debugbreak(); // overflow
				}
#ifdef _DEBUG
				if (m_auValue[SIZE - 1] & ~((1 << (uUNSIGNED_BITS % (sizeof(unsigned_type) * 8))) - 1))
				{
					__debugbreak(); // overflow
				}
#endif
				for (size_t uIndex = std::size(m_auValue), uIndexEnd = std::size(that.m_auValue); uIndex < uIndexEnd; uIndex++)
				{
					if (that.m_auValue[uIndex])
					{
						__debugbreak(); // overflow
					}
				}
				return *this;
			}

			template <size_t u1, size_t u2>
			Cinteger(Cinteger<u1> const & left, Cinteger<u2> const & right, minus_tag)
			{
				unsigned char ubBorrow = 0;

				for (size_t uIndex = 0, uIndexEnd = std::min(std::size(left.m_auValue), std::size(right.m_auValue)); uIndex < uIndexEnd; uIndex++)
				{
					ubBorrow = subborrow(ubBorrow, left.m_auValue[uIndex], right.m_auValue[uIndex], &m_auValue[uIndex]);
				}
				for (size_t uIndex = std::size(left.m_auValue), uIndexEnd = std::size(right.m_auValue); uIndex < uIndexEnd; uIndex++)
				{
					ubBorrow = subborrow(ubBorrow, 0, right.m_auValue[uIndex], &m_auValue[uIndex]);
				}
				for (size_t uIndex = std::size(right.m_auValue), uIndexEnd = std::size(left.m_auValue); uIndex < uIndexEnd; uIndex++)
				{
					ubBorrow = subborrow(ubBorrow, left.m_auValue[uIndex], 0, &m_auValue[uIndex]);
				}
				for (size_t uIndex = std::max(std::size(left.m_auValue), std::size(right.m_auValue)), uIndexEnd = std::size(m_auValue); uIndex < uIndexEnd; uIndex++)
				{
					ubBorrow = subborrow(ubBorrow, 0, 0, &m_auValue[uIndex]);
				}
			}
			template <size_t uTHAT>
			typename Cinteger & operator -= (Cinteger<uTHAT> const & that)
			{
				unsigned char ubBorrow = 0;

				for (size_t uIndex = 0, uIndexEnd = std::min(std::size(m_auValue), std::size(that.m_auValue)); uIndex < uIndexEnd; uIndex++)
				{
					ubBorrow = subborrow(ubBorrow, m_auValue[uIndex], that.m_auValue[uIndex], &m_auValue[uIndex]);
				}
				for (size_t uIndex = std::size(that.m_auValue), uIndexEnd = std::size(m_auValue); uIndex < uIndexEnd; uIndex++)
				{
					ubBorrow = subborrow(ubBorrow, m_auValue[uIndex], 0, &m_auValue[uIndex]);
				}
				if (ubBorrow)
				{
					__debugbreak(); // underflow
				}
				for (size_t uIndex = std::size(m_auValue), uIndexEnd = std::size(that.m_auValue); uIndex < uIndexEnd; uIndex++)
				{
					if (that.m_auValue[uIndex])
					{
						__debugbreak(); // underflow
					}
				}
				return *this;
			}

			template <size_t u1, size_t u2>
			Cinteger(Cinteger<u1> const & left, Cinteger<u2> const & right, multiplies_tag)
			{
				unsigned_type tLo(0);
				unsigned_type tHi(0);

				for (size_t uIndex = 0, uIndexEnd = std::size(m_auValue); uIndex < uIndexEnd; uIndex++)
				{
					unsigned_type tCarry(0);

					for (size_t uLeft = std::max(uIndex, (std::size(right.m_auValue) - 1)) - (std::size(right.m_auValue) - 1), uLeftEnd = std::min(uIndex + 1, std::size(left.m_auValue)); uLeft < uLeftEnd; uLeft++)
					{
						size_t const uRight = uIndex - uLeft;

						unsigned_type tLocalHi;
						unsigned_type tLocalLo = mulx(left.m_auValue[uLeft], right.m_auValue[uRight], &tLocalHi);

						unsigned char const ubCarry0 = 0;
						unsigned char const ubCarry1 = addcarryx(ubCarry0, tLo, tLocalLo, &tLo);
						unsigned char const ubCarry2 = addcarryx(ubCarry1, tHi, tLocalHi, &tHi);
						unsigned char const ubCarry3 = addcarryx(ubCarry2, tCarry, 0, &tCarry);

#ifdef _DEBUG
						if (ubCarry3)
						{
							__debugbreak();
						}
#endif
					}

					m_auValue[uIndex] = tLo;

					tLo = tHi;
					tHi = tCarry;
				}

#ifdef _DEBUG
				if (tLo || tHi)
				{
					__debugbreak();
				}
#endif
			}
			template <size_t uTHAT>
			typename Cinteger & operator *= (Cinteger<uTHAT> const & that)
			{
				unsigned_type tCarry(0);
				unsigned_type tHi(0);

				for (size_t uIndexR = std::size(m_auValue); 0 < uIndexR; uIndexR--)
				{
					unsigned_type tLo(0);

					for (size_t uThat = 0, uThatEnd = std::min(std::size(m_auValue), uIndexR), uThis = uIndexR - 1; uThat < uThatEnd; uThat++, uThis--)
					{
						unsigned_type tLocalHi;
						unsigned_type tLocalLo = mulx(m_auValue[uThis], that.m_auValue[uThat], &tLocalHi);

						unsigned char const ubCarry0 = 0;
						unsigned char const ubCarry1 = addcarryx(ubCarry0, tLo, tLocalLo, &tLo);
						unsigned char const ubCarry2 = addcarryx(ubCarry1, tHi, tLocalHi, &tHi);
						unsigned char const ubCarry3 = addcarryx(ubCarry2, tCarry, 0, &tCarry);

#ifdef _DEBUG
						if (ubCarry3)
						{
							__debugbreak();
						}
#endif
					}

					if (uIndexR + 1 < std::size(m_auValue))
					{
						m_auValue[uIndexR + 1] = tCarry;
					}
#ifdef _DEBUG
					else if (tCarry)
					{
						__debugbreak();
					}
#endif

					tCarry = tHi;
					tHi = tLo;
				}

				if (1 < std::size(m_auValue))
				{
					m_auValue[1] = tCarry;
				}
#ifdef _DEBUG
				else if (tCarry)
				{
					__debugbreak();
				}
#endif

				if (0 < std::size(m_auValue))
				{
					m_auValue[0] = tHi;
				}
#ifdef _DEBUG
				else if (tHi)
				{
					__debugbreak();
				}
#endif
				return *this;
			}

			template <size_t u1, size_t u2>
			Cinteger(Cinteger<u1> const & left, Cinteger<u2> const & right, dividies_tag)
			{
				static_assert(false, "This constructor is not implemented");
			}
		};
		template <size_t u0, size_t u1>
		struct integers_traits
		{
			using add_type = Cinteger<(u0 > u1) ? (u0 + 1) : (u1 + 1)>;
			using minus_type = Cinteger<(u0 > u1) ? (u0 + 1) : (u1 + 1)>;
			using multiplies_type = Cinteger<u0 + u1>;
			using dividies_type = Cinteger<u0>;
		};
		template <size_t u0, size_t u1>
		auto operator + (Cinteger<u0> const & left, Cinteger<u1> const & right)
		{
			return typename integers_traits<u0, u1>::add_type(left, right, add_tag());
		}
		template <size_t u0, size_t u1>
		auto operator - (Cinteger<u0> const & left, Cinteger<u1> const & right)
		{
			return typename integers_traits<u0, u1>::minus_type(left, right, minus_tag());
		}
		template <size_t u0, size_t u1>
		auto operator * (Cinteger<u0> const & left, Cinteger<u1> const & right)
		{
			return typename integers_traits<u0, u1>::multiplies_type(left, right, multiplies_tag());
		}
		template <size_t u0, size_t u1>
		auto operator / (Cinteger<u0> const & left, Cinteger<u1> const & right)
		{
			return typename integers_traits<u0, u1>::multiplies_type(left, right, dividies_tag());
		}

		// a/b
		template <typename Ta, typename Tb>
		class Crational
		{
			Ta m_tA;
			Tb m_tB;
		public:
			Crational(Ta const & tA, Tb const & tB)
				: m_tA(tA)
				, m_tB(tB)
			{
			}

			Ta const & get_numerator() const
			{
				return m_tA;
			}
			Tb const & get_denumerator() const
			{
				return m_tB;
			}
		};
		template <typename Ta, typename Tb, typename Tc, typename Td>
		struct rational_traits
		{
			// (a/b) + (c/d) = (a*d + b*c) / (b*d)
			using add_type = Crational<typename numbers_traits<typename numbers_traits<Ta, Td>::multiplies_type, typename numbers_traits<Tb, Tc>::multiplies_type>::add_type, typename numbers_traits<Tb, Td>::multiplies_type>;
			// (a/b) - (c/d) = (a*d - b*c) / (b*d)
			using minus_type = Crational<typename numbers_traits<typename numbers_traits<Ta, Td>::multiplies_type, typename numbers_traits<Tb, Tc>::multiplies_type>::minus_type, typename numbers_traits<Tb, Td>::multiplies_type>;
			// (a/b) * (c/d) = (a*c) / (b*d)
			using multiplies_type = Crational<typename numbers_traits<Ta, Tc>::multiplies_type, typename numbers_traits<Tb, Td>::multiplies_type>;
			// (a/b) / (c/d) = (a*d) / (b*c)
			using dividies_type = Crational<typename numbers_traits<Ta, Tb>::multiplies_type, typename numbers_traits<Tc, Td>::multiplies_type>;
		};
		template <typename Ta, typename Tb, typename Tc, typename Td>
		struct numbers_traits<Crational<Ta, Tb>, Crational<Tc, Td>>
			: rational_traits<Ta, Tb, Tc, Td>
		{
		};

		// (a/b) + (c/d) = (a*d + b*c) / (b*d)
		template <typename Ta, typename Tb, typename Tc, typename Td>
		auto operator + (Crational<Ta, Tb> const & left, Crational<Tc, Td> const & right)
		{
			Ta const & a = left.get_numerator();
			Tb const & b = left.get_denumerator();
			Tc const & c = right.get_numerator();
			Td const & d = right.get_denumerator();

			return typename rational_traits<Ta, Tb, Tc, Td>::add_type(a * d + b * c, b * d);
		}

		// (a/b) - (c/d) = (a*d - b*c) / (b*d)
		template <typename Ta, typename Tb, typename Tc, typename Td>
		auto operator - (Crational<Ta, Tb> const & left, Crational<Tc, Td> const & right)
		{
			Ta const & a = left.get_numerator();
			Tb const & b = left.get_denumerator();
			Tc const & c = right.get_numerator();
			Td const & d = right.get_denumerator();

			return typename rational_traits<Ta, Tb, Tc, Td>::minus_type(a * d - b * c, b * d);
		}

		// (a/b) * (c/d) = (a*c) / (b*d)
		template <typename Ta, typename Tb, typename Tc, typename Td>
		auto operator * (Crational<Ta, Tb> const & left, Crational<Tc, Td> const & right)
		{
			Ta const & a = left.get_numerator();
			Tb const & b = left.get_denumerator();
			Tc const & c = right.get_numerator();
			Td const & d = right.get_denumerator();

			return typename rational_traits<Ta, Tb, Tc, Td>::multiplies_type(a * c, b * d);
		}

		// (a/b) / (c/d) = (a*d) / (b*c)
		template <typename Ta, typename Tb, typename Tc, typename Td>
		auto operator / (Crational<Ta, Tb> const & left, Crational<Tc, Td> const & right)
		{
			Ta const & a = left.get_numerator();
			Tb const & b = left.get_denumerator();
			Tc const & c = right.get_numerator();
			Td const & d = right.get_denumerator();

			return typename rational_traits<Ta, Tb, Tc, Td>::dividies_type(a * b, c * d);
		}
	}
}