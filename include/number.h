#pragma once

#include <intrin.h>

#include <algorithm>
#include <type_traits>
#include <tuple>
#include <array>

namespace jrmwng
{
	namespace number
	{
#ifdef _M_X64
		using unsigned_type = unsigned long long;
#else
		using unsigned_type = unsigned long;
#endif

		namespace
		{
			class Cnop
			{
			public:
				Cnop(unsigned_type uValue)
				{
					// NOP
				}
				Cnop & operator = (unsigned_type)
				{
					return *this;
				}
				Cnop & operator |= (unsigned_type)
				{
					return *this;
				}
				operator unsigned_type () const
				{
					return 0U;
				}
			};
			class Cdebug_break
			{
			public:
				Cdebug_break(unsigned_type uValue)
				{
					if (uValue)
					{
						__debugbreak();
					}
				}
			};
			class Cthrow
			{
			public:
				Cthrow(unsigned_type uValue)
				{
					if (uValue)
					{
						throw uValue;
					}
				}
			};
			template <typename Taction>
			class Cunsigned
			{
				unsigned_type m_uValue;
			public:
				Cunsigned(unsigned_type uValue)
					: m_uValue(uValue)
				{
					Taction const action(uValue);
				}
				typename Cunsigned & operator = (unsigned_type uValue)
				{
					m_uValue = uValue;
					Taction const action(uValue);
					return *this;
				}
				typename Cunsigned & operator |= (unsigned_type uValue)
				{
					m_uValue |= uValue;
					Taction const action(uValue);
					return *this;
				}
				operator unsigned_type () const
				{
					return m_uValue;
				}
			};
		}

		template <typename Ta, typename Tb>
		struct numbers_traits
		{
		};
		struct add_tag {};
		struct minus_tag {};
		struct multiplies_tag {};
		struct dividies_tag {};

		struct integer_traits
		{
#ifdef _DEBUG
			using overflow_type  = Cunsigned<Cdebug_break>;
			using underflow_type = Cunsigned<Cdebug_break>;
#else
			using overflow_type  = Cnop;
			using underflow_type = Cnop;
#endif
		};

		template <size_t uUNSIGNED_BITS, typename Ttraits = integer_traits>
		class Cinteger
		{
			friend class Cinteger;
		public:
			template <size_t uACCUMULATOR_BITS>
			using accumulator_type = Cinteger<uUNSIGNED_BITS + uACCUMULATOR_BITS>;

		private:
			enum Econstant
			{
				SIZE = (uUNSIGNED_BITS + (sizeof(unsigned_type) * 8U - 1U)) / (sizeof(unsigned_type) * 8U),
				OVERFLOW_MASK = (uUNSIGNED_BITS < sizeof(unsigned_type) * 8U) ? (~((1U << (uUNSIGNED_BITS % (sizeof(unsigned_type) * 8U))) - 1U)) : 0U,
			};

			unsigned_type m_auValue[SIZE];
			typename Ttraits::overflow_type  m_uOverflow = 0;
			typename Ttraits::underflow_type m_uUnderflow = 0;

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

		public:
			template <typename... Tunsigned>
			Cinteger(unsigned_type uValue0, Tunsigned ... uValues)
				: m_auValue{ uValue0, uValues... }
				, m_uOverflow(m_auValue[SIZE - 1] & OVERFLOW_MASK)
			{
			}

			template <size_t u1, typename T1, size_t u2, typename T2>
			Cinteger(Cinteger<u1, T1> const & left, Cinteger<u2, T2> const & right, add_tag)
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
				m_uOverflow = (m_auValue[SIZE - 1] & OVERFLOW_MASK) | ubCarry;

				for (size_t uLeft = std::size(m_auValue), uLeftEnd = std::size(left.m_auValue); uLeft < uLeftEnd; uLeft++)
				{
					m_uOverflow |= left.m_auValue[uLeft];
				}
				for (size_t uRight = std::size(m_auValue), uRightEnd = std::size(right.m_auValue); uRight < uRightEnd; uRight++)
				{
					m_uOverflow |= right.m_auValue[uRight];
				}
			}
			template <size_t uTHAT, typename Tthat_traits>
			typename Cinteger & operator += (Cinteger<uTHAT, Tthat_traits> const & that)
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
				m_uOverflow = (m_auValue[SIZE - 1] & OVERFLOW_MASK) | ubCarry;

				for (size_t uIndex = std::size(m_auValue), uIndexEnd = std::size(that.m_auValue); uIndex < uIndexEnd; uIndex++)
				{
					m_uOverflow |= that.m_auValue[uIndex];
				}
				return *this;
			}

			template <size_t u1, typename T1, size_t u2, typename T2>
			Cinteger(Cinteger<u1, T1> const & left, Cinteger<u2, T2> const & right, minus_tag)
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

				m_uUnderflow = (m_auValue[SIZE - 1] & OVERFLOW_MASK) | ubBorrow;

				for (size_t uIndex = std::size(m_auValue), uIndexEnd = std::min(std::size(left.m_auValue), std::size(right.m_auValue)); uIndex < uIndexEnd; uIndex++)
				{
					unsigned_type uTemp;
					{
						ubBorrow = subborrow(ubBorrow, left.m_auValue, right.m_auValue, &uTemp);
					}
					if (ubBorrow)
					{
						m_uUnderflow |= uTemp;
					}
					else
					{
						m_uOverflow |= uTemp;
					}
				}
				for (size_t uLeft = std::max(std::size(m_auValue), std::size(right.m_auValue)), uLeftEnd = std::size(left.m_auValue); uLeft < uLeftEnd; uLeft++)
				{
					m_uOverflow |= left.m_auValue[uLeft];
				}
				for (size_t uRight = std::max(std::size(m_auValue), std::size(left.m_auValue)), uRightEnd = std::size(right.m_auValue); uRight < uRightEnd; uRight++)
				{
					m_uUnderflow |= right.m_auValue[uRight];
				}
			}
			template <size_t uTHAT, typename Tthat_traits>
			typename Cinteger & operator -= (Cinteger<uTHAT, Tthat_traits> const & that)
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

				m_uUnderflow = (m_auValue[SIZE - 1] & OVERFLOW_MASK) | ubBorrow;

				for (size_t uIndex = std::size(m_auValue), uIndexEnd = std::size(that.m_auValue); uIndex < uIndexEnd; uIndex++)
				{
					m_uUnderflow |= that.m_auValue[uIndex];
				}
				return *this;
			}

			template <size_t u1, typename T1, size_t u2, typename T2>
			Cinteger(Cinteger<u1, T1> const & left, Cinteger<u2, T2> const & right, multiplies_tag)
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

						m_uOverflow |= ubCarry3;
					}

					m_auValue[uIndex] = tLo;

					tLo = tHi;
					tHi = tCarry;
				}

				m_uOverflow |= (m_auValue[SIZE - 1] & OVERFLOW_MASK) | tLo | tHi;
			}
			template <size_t uTHAT, typename Tthat_traits>
			typename Cinteger & operator *= (Cinteger<uTHAT, Tthat_traits> const & that)
			{
				unsigned_type tCarry(0);
				unsigned_type tHi(0);

				// TODO: check for overflow bits

				for (size_t uIndexR = std::size(m_auValue); 0 < uIndexR; uIndexR--)
				{
					unsigned_type tLo(0);

					for (size_t uThat = 0, uThatEnd = std::min(uIndexR, std::size(that.m_auValue)), uThis = uIndexR - 1; uThat < uThatEnd; uThat++, uThis--)
					{
						unsigned_type tLocalHi;
						unsigned_type tLocalLo = mulx(m_auValue[uThis], that.m_auValue[uThat], &tLocalHi);

						unsigned char const ubCarry0 = 0;
						unsigned char const ubCarry1 = addcarryx(ubCarry0, tLo, tLocalLo, &tLo);
						unsigned char const ubCarry2 = addcarryx(ubCarry1, tHi, tLocalHi, &tHi);
						unsigned char const ubCarry3 = addcarryx(ubCarry2, tCarry, 0, &tCarry);

						m_uOverflow |= ubCarry3;
					}

					if (uIndexR + 1 < std::size(m_auValue))
					{
						m_auValue[uIndexR + 1] = tCarry;
					}
					else
					{
						m_uOverflow |= tCarry;
					}

					tCarry = tHi;
					tHi = tLo;
				}

				if (1 < std::size(m_auValue))
				{
					m_auValue[1] = tCarry;
				}
				else
				{
					m_uOverflow |= tCarry;
				}

				if (0 < std::size(m_auValue))
				{
					m_auValue[0] = tHi;
				}
				else
				{
					m_uOverflow |= tHi;
				}

				m_uOverflow |= (m_auValue[SIZE - 1] & OVERFLOW_MASK);
				return *this;
			}

			template <size_t u1, typename T1, size_t u2, typename T2>
			Cinteger(Cinteger<u1, T1> const & left, Cinteger<u2, T2> const & right, dividies_tag)
			{
				static_assert(false, "This constructor is not implemented");
			}

			Cinteger(char const *pcText, Cinteger<5U, Ttraits> const uRadix = 10U)
				: Cinteger(0U)
			{
				if (pcText)
				{
					for (char const *itDigit = pcText; *itDigit; ++itDigit)
					{
						char const cDigit = *itDigit;

						unsigned_type uDigit = 0U;
						{
							if ('0' <= cDigit && cDigit <= '9')
							{
								uDigit = cDigit - '0';
							}
							else if ('a' <= cDigit && cDigit <= 'z')
							{
								uDigit = 10U + cDigit - 'a';
							}
							else if ('A' <= cDigit && cDigit <= 'Z')
							{
								uDigit = 10U + cDigit - 'A';
							}
						}

						if (uDigit < uRadix.m_auValue[0])
						{
							*this *= uRadix;
							*this += Cinteger<5, Ttraits>(uDigit);
						}
						else
						{
							m_uOverflow |= uDigit;
						}
					}
					m_uOverflow |= (m_auValue[SIZE - 1] & OVERFLOW_MASK);
				}
			}

			template <size_t uTHAT_BITS, typename Tthat_traits, typename Tpredicate>
			bool compare(Cinteger<uTHAT_BITS, Tthat_traits> const & that, Tpredicate tPredicate) const
			{
				static_assert(std::is_same_v<Tpredicate, std::equal_to<unsigned_type>> == false, "Use 'std::not_equal_to<...>' instead of 'std::equal_to<...>'");

				for (size_t uThis = std::size(that.m_auValue), uThisEnd = std::size(m_auValue); uThis < uThisEnd; uThis++)
				{
					if (tPredicate(m_auValue[uThis], 0U))
					{
						return true;
					}
				}
				for (size_t uThat = std::size(m_auValue), uThatEnd = std::size(that.m_auValue); uThat < uThatEnd; uThat++)
				{
					if (tPredicate(0U, that.m_auValue[uThat]))
					{
						return true;
					}
				}
				for (size_t uIndexR = std::min(std::size(m_auValue), std::size(that.m_auValue)); 0 < uIndexR; uIndexR--)
				{
					if (tPredicate(m_auValue[uIndexR - 1], that.m_auValue[uIndexR - 1]))
					{
						return true;
					}
				}
				return false;
			}
			template <size_t uTHAT_BITS, typename Tthat_traits>
			bool operator < (Cinteger<uTHAT_BITS, Tthat_traits> const & that) const
			{
				return compare(that, std::less<unsigned_type>());
			}
			template <size_t uTHAT_BITS, typename Tthat_traits>
			bool operator <= (Cinteger<uTHAT_BITS, Tthat_traits> const & that) const
			{
				return compare(that, std::less_equal<unsigned_type>());
			}
			template <size_t uTHAT_BITS, typename Tthat_traits>
			bool operator > (Cinteger<uTHAT_BITS, Tthat_traits> const & that) const
			{
				return compare(that, std::greater<unsigned_type>());
			}
			template <size_t uTHAT_BITS, typename Tthat_traits>
			bool operator >= (Cinteger<uTHAT_BITS, Tthat_traits> const & that) const
			{
				return compare(that, std::greater_equal<unsigned_type>());
			}
			template <size_t uTHAT_BITS, typename Tthat_traits>
			bool operator != (Cinteger<uTHAT_BITS, Tthat_traits> const & that) const
			{
				return compare(that, std::not_equal_to<unsigned_type>());
			}
			template <size_t uTHAT_BITS, typename Tthat_traits>
			bool operator == (Cinteger<uTHAT_BITS, Tthat_traits> const & that) const
			{
				return compare(that, std::not_equal_to<unsigned_type>()) == false; // must not use 'equal_to'
			}
		};
		template <size_t u0, size_t u1, typename Ttraits>
		struct integers_traits
		{
			using add_type = Cinteger<(u0 > u1) ? (u0 + 1) : (u1 + 1), Ttraits>;
			using minus_type = Cinteger<(u0 > u1) ? (u0 + 1) : (u1 + 1), Ttraits>;
			using multiplies_type = Cinteger<u0 + u1, Ttraits>;
			using dividies_type = Cinteger<u0, Ttraits>;
		};
		template <size_t u0, size_t u1, typename Ttraits>
		auto operator + (Cinteger<u0, Ttraits> const & left, Cinteger<u1, Ttraits> const & right)
		{
			return typename integers_traits<u0, u1, Ttraits>::add_type(left, right, add_tag());
		}
		template <size_t u0, size_t u1, typename Ttraits>
		auto operator - (Cinteger<u0, Ttraits> const & left, Cinteger<u1, Ttraits> const & right)
		{
			return typename integers_traits<u0, u1, Ttraits>::minus_type(left, right, minus_tag());
		}
		template <size_t u0, size_t u1, typename Ttraits>
		auto operator * (Cinteger<u0, Ttraits> const & left, Cinteger<u1, Ttraits> const & right)
		{
			return typename integers_traits<u0, u1, Ttraits>::multiplies_type(left, right, multiplies_tag());
		}
		template <size_t u0, size_t u1, typename Ttraits>
		auto operator / (Cinteger<u0, Ttraits> const & left, Cinteger<u1, Ttraits> const & right)
		{
			return typename integers_traits<u0, u1, Ttraits>::multiplies_type(left, right, dividies_tag());
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