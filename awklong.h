/*
 * Copyright (C) 2020 Michael M. Builov, mbuilov@gmail.com
 *
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
 *
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

/* Standard "long" integer type is a disaster.
  It's size is at least 32 bits, but on common systems it varies:
  LINUX x86:      sizeof(long) == 4,
  LINUX x86_64:   sizeof(long) == 8, !!!
  WINDOWS x86:    sizeof(long) == 4,
  WINDOWS x86_64: sizeof(long) == 4. !!!
  So on WINDOWS x86_64, it is impossible for a compiler to detect (LINUX x86_64-specific) integer truncation
  bugs - when implicitly converting long -> int.
  To not depend on this behaviour, define and use our "long" type: awk_long_t.  */

/* Note: AWKLONG name should not be used in gawk code:
  1) use awk_long_t/awk_ulong_t instead.
  2) To pass awk_long_t/awk_ulong_t to printf-like functions, use TO_AWK_LONG()/TO_AWK_ULONG(),
  3) for printf-format string, use AWKLONGFMT/AWKULONGFMT, e.g.:
   printf("abc %" AWKLONGFMT " 123", TO_AWK_LONG(x));  */

/*#define AWKLONG_IS_LONGLONG 1*/

#if defined AWKLONG_IS_LONGLONG
# if AWKLONG_IS_LONGLONG == 0
#  define AWKLONG	long
# elif defined _WIN64
#  define AWKLONG	__int64
# else
#  define AWKLONG	long long
# endif
#elif defined _WIN64
# define AWKLONG	__int64
# define AWKLONG_IS_LONGLONG	1
#else
# define AWKLONG	long
#endif

#if defined AWKLONG_IS_LONGLONG && AWKLONG_IS_LONGLONG != 0
# define AWKLONGFMT	"lld"
# define AWKULONGFMT	"llu"
#else
# define AWKLONGFMT	"ld"
# define AWKULONGFMT	"lu"
#endif

/* For C or old C++, no type conversion checks, just typedefs.  */
#if !defined __cplusplus || __cplusplus < 201103L
typedef unsigned AWKLONG awk_ulong_t;
typedef AWKLONG awk_long_t;
#define TO_AWK_ULONG(ulw)	(ulw)
#define TO_AWK_LONG(lw)		(lw)
#endif

/* Maximum value of awk_long_t.  */
#define AWKLONGMAX	((AWKLONG) ((unsigned AWKLONG)-1/2))

/* Minimum value of awk_long_t.  */
#if INT_MIN + 1 == -INT_MAX
#define AWKLONGMIN	(-(AWKLONGMAX) - 1)
#elif !defined(AWKLONGMIN)
#error please define AWKLONGMIN
#endif

/* To avoid integer truncation bugs:
  - conversions from awk_long_t to int should be explicit.
  - conversions from awk_ulong_t to unsigned int should be explicit.  */
#if defined __cplusplus && __cplusplus >= 201103L
#if defined __GNUC__ && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
#ifdef __clang__
/* Const class methods are pure by the definition, but clang complains about side-effects when
  these are called in __builtin_assume(), so additionally mark const methods as pure ones;
  for the same purposes, also mark pure inline functions as pure ones.  */
#define const_method const __attribute__ ((pure))
#define pure_inline __attribute__ ((pure)) inline
#else
#define const_method const
#define pure_inline inline
#endif
struct awk_long_t {
	AWKLONG l;
	awk_long_t() {}
	awk_long_t(int i_) {l = i_;}
#if defined AWKLONG_IS_LONGLONG && AWKLONG_IS_LONGLONG != 0
	awk_long_t(long l_) {l = l_;}
#endif
	awk_long_t(AWKLONG l_) {l = l_;}
	awk_long_t(const awk_long_t &l_) {l = l_.l;}
	explicit awk_long_t(const double &d_) {l = (AWKLONG) d_;}
	explicit awk_long_t(const unsigned u_) {l = (AWKLONG) u_;}
#if defined _WIN64 || (defined __SIZEOF_POINTER__ && __SIZEOF_POINTER__ >= 8)
	explicit awk_long_t(const unsigned long long ull_) {l = (AWKLONG) ull_;}
#if !defined AWKLONG_IS_LONGLONG || AWKLONG_IS_LONGLONG == 0
	explicit awk_long_t(const long long ll_) {l = (AWKLONG) ll_;}
#endif
#endif
	struct awk_long_t &operator++() {++l; return *this;}
	struct awk_long_t &operator--() {--l; return *this;}
	struct awk_long_t operator++(int) {AWKLONG r = l++; return r;}
	struct awk_long_t operator--(int) {AWKLONG r = l--; return r;}
	struct awk_long_t &operator= (const struct awk_long_t &l_) {l =  l_.l; return *this;}
	struct awk_long_t &operator+=(const struct awk_long_t &l_) {l += l_.l; return *this;}
	struct awk_long_t &operator-=(const struct awk_long_t &l_) {l -= l_.l; return *this;}
	struct awk_long_t &operator*=(int i_) {l *= i_; return *this;}
	bool operator> (const struct awk_long_t &l_) const_method {return l >  l_.l;}
	bool operator>=(const struct awk_long_t &l_) const_method {return l >= l_.l;}
	bool operator< (const struct awk_long_t &l_) const_method {return l <  l_.l;}
	bool operator<=(const struct awk_long_t &l_) const_method {return l <= l_.l;}
	bool operator==(const struct awk_long_t &l_) const_method {return l == l_.l;}
	bool operator!=(const struct awk_long_t &l_) const_method {return l != l_.l;}
	bool operator> (const AWKLONG &l_) const_method {return l >  l_;}
	bool operator>=(const AWKLONG &l_) const_method {return l >= l_;}
	bool operator<=(const AWKLONG &l_) const_method {return l <= l_;}
#if defined AWKLONG_IS_LONGLONG && AWKLONG_IS_LONGLONG != 0
	bool operator> (const long &l_) const_method {return l > l_;}
	bool operator< (const long &l_) const_method {return l > l_;}
#endif
	operator bool() const_method {return l != 0;}
	bool operator!() const_method {return l == 0;}
	bool operator> (int i_) const_method {return l >  i_;}
	bool operator>=(int i_) const_method {return l >= i_;}
	bool operator< (int i_) const_method {return l <  i_;}
	bool operator<=(int i_) const_method {return l <= i_;}
	bool operator==(int i_) const_method {return l == i_;}
	bool operator!=(int i_) const_method {return l != i_;}
	awk_long_t operator-() const_method {return -l;}
	awk_long_t operator+(const struct awk_long_t &l_) const_method {return l + l_.l;}
	awk_long_t operator-(const struct awk_long_t &l_) const_method {return l - l_.l;}
	awk_long_t operator+(int i_) const_method {return l + i_;}
	awk_long_t operator-(int i_) const_method {return l - i_;}
	awk_long_t operator*(int i_) const_method {return l * i_;}
	awk_long_t operator/(int i_) const_method {return l / i_;}
	operator AWKLONG () const_method {return l;}
#if !defined AWKLONG_IS_LONGLONG || AWKLONG_IS_LONGLONG == 0
	/* long -> long long */
	operator long long () const_method {return l;}
#else
	/* long long -> long */
	explicit operator long () const_method {return (long) l;}
#endif
#if !defined _WIN64 && (!defined __SIZEOF_POINTER__ || __SIZEOF_POINTER__ < 8)
	/* for use awk_long_t as array index */
	operator int () const_method {return (int) l;}
#else
	explicit operator int () const_method {return (int) l;}
#endif
	explicit operator unsigned int () const_method {return (unsigned) l;}
	explicit operator double () const_method {return (double) l;}
#if defined _WIN64 || (defined __SIZEOF_POINTER__ && __SIZEOF_POINTER__ >= 8)
	explicit operator unsigned long long () const_method {return (unsigned long long) l;}
#endif
private:
	operator char () const;
	operator short () const;
	operator unsigned char () const;
	operator unsigned short () const;
	operator unsigned long () const;
#if !defined _WIN64 && (!defined __SIZEOF_POINTER__ || __SIZEOF_POINTER__ < 8)
	operator unsigned long long () const;
#endif
};
pure_inline bool operator> (int i_, const struct awk_long_t &l) {return i_ > l.l;}
pure_inline bool operator<=(int i_, const struct awk_long_t &l) {return i_ <= l.l;}
pure_inline bool operator==(int i_, const struct awk_long_t &l) {return i_ == l.l;}
pure_inline awk_long_t operator-(const AWKLONG &l_, const struct awk_long_t &l) {return l_ - l.l;}
struct awk_ulong_t {
	unsigned AWKLONG ul;
	awk_ulong_t() {}
	awk_ulong_t(unsigned ui_) {ul = ui_;}
#if defined AWKLONG_IS_LONGLONG && AWKLONG_IS_LONGLONG != 0
	awk_ulong_t(unsigned long ul_) {ul = ul_;}
#endif
	awk_ulong_t(unsigned AWKLONG ul_) {ul = ul_;}
	awk_ulong_t(const awk_ulong_t &ul_) {ul = ul_.ul;}
	explicit awk_ulong_t(const awk_long_t &l_) {ul = (unsigned AWKLONG) l_.l;}
	explicit awk_ulong_t(const double d_) {ul = (unsigned AWKLONG) d_;}
	explicit awk_ulong_t(const int i_) {ul = (unsigned) i_;}
#if defined _WIN64 || (defined __SIZEOF_POINTER__ && __SIZEOF_POINTER__ >= 8)
#if !defined AWKLONG_IS_LONGLONG || AWKLONG_IS_LONGLONG == 0
	explicit awk_ulong_t(const unsigned long long ull_) {ul = (unsigned AWKLONG) ull_;}
#endif
#endif
	struct awk_ulong_t &operator++() {++ul; return *this;}
	struct awk_ulong_t &operator--() {--ul; return *this;}
	struct awk_ulong_t operator++(int) {unsigned AWKLONG r = ul++; return r;}
	struct awk_ulong_t operator--(int) {unsigned AWKLONG r = ul--; return r;}
	struct awk_ulong_t &operator= (const struct awk_ulong_t &ul_) {ul =  ul_.ul; return *this;}
	struct awk_ulong_t &operator+=(const struct awk_ulong_t &ul_) {ul += ul_.ul; return *this;}
	struct awk_ulong_t &operator-=(const struct awk_ulong_t &ul_) {ul -= ul_.ul; return *this;}
	struct awk_ulong_t &operator+=(const struct awk_long_t &l_) {ul += (unsigned AWKLONG) l_.l; return *this;}
	struct awk_ulong_t &operator-=(const struct awk_long_t &l_) {ul -= (unsigned AWKLONG) l_.l; return *this;}
	struct awk_ulong_t &operator%=(const struct awk_ulong_t &ul_) {ul %= ul_.ul; return *this;}
	struct awk_ulong_t &operator^=(const struct awk_ulong_t &ul_) {ul ^= ul_.ul; return *this;}
	struct awk_ulong_t &operator*=(int i_) {ul *= (unsigned AWKLONG) i_; return *this;}
	struct awk_ulong_t &operator&=(unsigned u_) {ul &= u_; return *this;}
	struct awk_ulong_t &operator/=(unsigned u_) {ul /= u_; return *this;}
	struct awk_ulong_t &operator+=(unsigned u_) {ul += u_; return *this;}
	struct awk_ulong_t &operator+=(unsigned char c_) {ul += c_; return *this;}
	struct awk_ulong_t operator<<(unsigned u_) const_method {return ul << u_;}
	struct awk_ulong_t operator>>(unsigned u_) const_method {return ul >> u_;}
	bool operator> (const struct awk_ulong_t &ul_) const_method {return ul >  ul_.ul;}
	bool operator>=(const struct awk_ulong_t &ul_) const_method {return ul >= ul_.ul;}
	bool operator< (const struct awk_ulong_t &ul_) const_method {return ul <  ul_.ul;}
	bool operator<=(const struct awk_ulong_t &ul_) const_method {return ul <= ul_.ul;}
	bool operator==(const struct awk_ulong_t &ul_) const_method {return ul == ul_.ul;}
	bool operator!=(const struct awk_ulong_t &ul_) const_method {return ul != ul_.ul;}
	bool operator> (unsigned u_) const_method {return ul >  u_;}
	bool operator>=(unsigned u_) const_method {return ul >= u_;}
	bool operator< (unsigned u_) const_method {return ul <  u_;}
	bool operator<=(unsigned u_) const_method {return ul <= u_;}
	bool operator==(unsigned u_) const_method {return ul == u_;}
	bool operator!=(unsigned u_) const_method {return ul != u_;}
	operator bool() const_method {return ul != 0u;}
	bool operator!() const_method {return ul == 0u;}
#if defined _WIN64 || (defined __SIZEOF_POINTER__ && __SIZEOF_POINTER__ >= 8)
	bool operator> (unsigned long long ull_) const_method {return ul > ull_;}
	bool operator>=(unsigned long long ull_) const_method {return ul >= ull_;}
	bool operator< (unsigned long long ull_) const_method {return ul < ull_;}
	bool operator<=(unsigned long long ull_) const_method {return ul <= ull_;}
#endif
	awk_ulong_t operator~() const_method {return ~ul;}
	awk_ulong_t operator*(const struct awk_ulong_t &ul_) const_method {return ul * ul_.ul;}
	awk_ulong_t operator/(const struct awk_ulong_t &ul_) const_method {return ul / ul_.ul;}
	awk_ulong_t operator+(const struct awk_ulong_t &ul_) const_method {return ul + ul_.ul;}
	awk_ulong_t operator-(const struct awk_ulong_t &ul_) const_method {return ul - ul_.ul;}
	awk_ulong_t operator|(const struct awk_ulong_t &ul_) const_method {return ul | ul_.ul;}
	awk_ulong_t operator%(const struct awk_ulong_t &ul_) const_method {return ul % ul_.ul;}
	awk_ulong_t operator+(const struct awk_long_t &l_) const_method {return ul + (unsigned AWKLONG) l_.l;}
	awk_ulong_t operator-(const struct awk_long_t &l_) const_method {return ul - (unsigned AWKLONG) l_.l;}
	awk_ulong_t operator+(int i_) const_method {return ul + (unsigned AWKLONG) i_;}
	awk_ulong_t operator-(int i_) const_method {return ul - (unsigned AWKLONG) i_;}
	awk_ulong_t operator+(unsigned u_) const_method {return ul + u_;}
	awk_ulong_t operator-(unsigned u_) const_method {return ul - u_;}
	awk_ulong_t operator*(unsigned u_) const_method {return ul * u_;}
	awk_ulong_t operator/(unsigned u_) const_method {return ul / u_;}
	awk_ulong_t operator-(unsigned long ul_) const_method {return ul - ul_;}
	unsigned operator%(unsigned u_) const_method {return (unsigned) (ul % u_);}
#if defined _WIN64 || (defined __SIZEOF_POINTER__ && __SIZEOF_POINTER__ >= 8)
	unsigned long long operator*(unsigned long long ull_) const_method {return ul * ull_;}
	unsigned long long operator+(unsigned long long ull_) const_method {return ul + ull_;}
	unsigned long long operator-(unsigned long long ull_) const_method {return ul - ull_;}
	awk_ulong_t operator/(unsigned long long ull_) const_method {return (unsigned AWKLONG) (ul / ull_);}
	awk_ulong_t operator%(unsigned long long ull_) const_method {return (unsigned AWKLONG) (ul % ull_);}
#endif
	operator unsigned AWKLONG () const_method {return ul;}
#if !defined AWKLONG_IS_LONGLONG || AWKLONG_IS_LONGLONG == 0
	/* unsigned long -> unsigned long long */
	operator unsigned long long () const_method {return ul;}
#endif
#if !defined _WIN64 && (!defined __SIZEOF_POINTER__ || __SIZEOF_POINTER__ < 8)
	/* for use awk_ulong_t as array index */
	operator int () const_method {return (int) ul;}
#else
	/* for use awk_ulong_t as array index */
	operator long long () const_method {return (long long) ul;}
	explicit operator int () const_method {return (int) ul;}
#endif
	explicit operator long () const_method {return (long) ul;}
	explicit operator unsigned int () const_method {return (unsigned) ul;}
	explicit operator double () const_method {return (double) ul;}
private:
	operator char () const;
	operator short () const;
#if !defined _WIN64 && (!defined __SIZEOF_POINTER__ || __SIZEOF_POINTER__ < 8)
	operator long long () const;
#endif
	operator unsigned char () const;
	operator unsigned short () const;
#if defined AWKLONG_IS_LONGLONG && AWKLONG_IS_LONGLONG != 0
	operator unsigned long () const;
#endif
};
pure_inline bool operator==(size_t sz_, const struct awk_ulong_t &ul) {return sz_ == ul.ul;}
pure_inline bool operator!=(size_t sz_, const struct awk_ulong_t &ul) {return sz_ != ul.ul;}
pure_inline bool operator> (size_t sz_, const struct awk_ulong_t &ul) {return sz_ >  ul.ul;}
pure_inline bool operator>=(size_t sz_, const struct awk_ulong_t &ul) {return sz_ >= ul.ul;}
#if defined _WIN64 || (defined __SIZEOF_POINTER__ && __SIZEOF_POINTER__ >= 8)
pure_inline bool operator> (unsigned u_, const struct awk_ulong_t &ul) {return u_ >  ul.ul;}
#endif
inline size_t operator%=(size_t &sz_, const struct awk_ulong_t &ul) {sz_ = (size_t) (sz_ % ul.ul); return sz_;}
pure_inline awk_ulong_t operator+(unsigned char c, const struct awk_ulong_t &ul) {return c + ul.ul;}
pure_inline awk_ulong_t operator*(unsigned u, const struct awk_ulong_t &ul) {return u * ul.ul;}
pure_inline unsigned AWKLONG TO_AWK_ULONG(const struct awk_ulong_t &ul) {return ul.ul;}
pure_inline AWKLONG TO_AWK_LONG(const struct awk_long_t &l) {return l.l;}
#undef pure_inline
#undef const_method
#if defined __GNUC__ && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#endif
#endif /* __cplusplus >= 201103L */
