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

/* Note: AWKLONG should not be used in gawk code:
  1) use awk_long_t/awk_ulong_t instead.
  2) To pass awk_long_t/awk_ulong_t to printf-like functions, use TO_AWK_LONG()/TO_AWK_ULONG(),
  3) for printf-format string, use AWKLONGFMT/AWKULONGFMT, e.g.:
   printf("abc %" AWKLONGFMT " 123", TO_AWK_LONG(x));  */
#ifdef _WIN64
#define AWKLONG		__int64
#define AWKLONGFMT	"PRId64"
#define AWKULONGFMT	"PRIu64"
#else
#define AWKLONG		long
#define AWKLONGFMT	"ld"
#define AWKULONGFMT	"lu"
#endif

/* For C or old C++, no type conversion checks, just typedefs.  */
#if !defined __cplusplus || __cplusplus < 201103L
typedef unsigned AWKLONG awk_ulong_t;
typedef AWKLONG awk_long_t;
#define TO_AWK_ULONG(ulw)	(ulw)
#define TO_AWK_LONG(lw)		(lw)
#endif

/* Maximum value of awk_long_t.  */
#define AWKLONGMAX	((awk_long_t) (AWKLONG) ((unsigned AWKLONG)-1/2))

/* Minimum value of awk_long_t.  */
#if INT_MIN + 1 == -INT_MAX
#define AWKLONGMIN	((awk_long_t) -(AWKLONGMAX) - 1)
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
	awk_long_t(AWKLONG l_) {l = l_;}
	awk_long_t(const awk_long_t &l_) {l = l_.l;}
	explicit awk_long_t(const double &d_) {l = (AWKLONG) d_;}
	explicit awk_long_t(const size_t s_) {l = (AWKLONG) s_;}
	struct awk_long_t &operator= (const struct awk_long_t &l_) {l =  l_.l; return *this;}
	struct awk_long_t &operator+=(const struct awk_long_t &l_) {l += l_.l; return *this;}
	struct awk_long_t &operator-=(const struct awk_long_t &l_) {l -= l_.l; return *this;}
	struct awk_long_t &operator+=(AWKLONG l_) {l += l_; return *this;}
	struct awk_long_t &operator-=(AWKLONG l_) {l -= l_; return *this;}
	struct awk_long_t &operator+=(int i_) {l += i_; return *this;}
	struct awk_long_t &operator-=(int i_) {l -= i_; return *this;}
	struct awk_long_t &operator*=(int i_) {l *= i_; return *this;}
	struct awk_long_t &operator++() {++l; return *this;}
	struct awk_long_t &operator--() {--l; return *this;}
	struct awk_long_t operator++(int) {AWKLONG r = l++; return r;}
	struct awk_long_t operator--(int) {AWKLONG r = l--; return r;}
	bool operator> (const struct awk_long_t &l_) const_method {return l >  l_.l;}
	bool operator>=(const struct awk_long_t &l_) const_method {return l >= l_.l;}
	bool operator< (const struct awk_long_t &l_) const_method {return l <  l_.l;}
	bool operator<=(const struct awk_long_t &l_) const_method {return l <= l_.l;}
	bool operator==(const struct awk_long_t &l_) const_method {return l == l_.l;}
	bool operator!=(const struct awk_long_t &l_) const_method {return l != l_.l;}
	bool operator> (AWKLONG l_) const_method {return l >  l_;}
	bool operator>=(AWKLONG l_) const_method {return l >= l_;}
	bool operator< (AWKLONG l_) const_method {return l <  l_;}
	bool operator<=(AWKLONG l_) const_method {return l <= l_;}
	bool operator==(AWKLONG l_) const_method {return l == l_;}
	bool operator!=(AWKLONG l_) const_method {return l != l_;}
	bool operator> (int i_) const_method {return l >  i_;}
	bool operator>=(int i_) const_method {return l >= i_;}
	bool operator< (int i_) const_method {return l <  i_;}
	bool operator<=(int i_) const_method {return l <= i_;}
	bool operator==(int i_) const_method {return l == i_;}
	bool operator!=(int i_) const_method {return l != i_;}
	operator bool() const_method {return l != 0l;}
	bool operator!() const_method {return l == 0l;}
	awk_long_t operator-() const_method {return -l;}
	awk_long_t operator+(const struct awk_long_t &l_) const_method {return l + l_.l;}
	awk_long_t operator-(const struct awk_long_t &l_) const_method {return l - l_.l;}
	awk_long_t operator+(AWKLONG l_) const_method {return l + l_;}
	awk_long_t operator-(AWKLONG l_) const_method {return l - l_;}
	awk_long_t operator+(int i_) const_method {return l + i_;}
	awk_long_t operator-(int i_) const_method {return l - i_;}
	awk_long_t operator*(int i_) const_method {return l*i_;}
	operator AWKLONG () const_method {return l;}
	operator long long () const_method {return l;}
#ifdef _WIN64
	explicit operator long () const_method {return (long) l;}
	explicit operator int () const_method {return (int) l;}
#else
	/* for use awk_long_t as array index */
	operator int () const_method {return (int) l;}
#endif
	explicit operator unsigned int () const_method {return (unsigned int) l;}
	explicit operator unsigned long () const_method {return (unsigned long) l;}
	explicit operator double () const_method {return (double) l;}
private:
	operator char () const;
	operator short () const;
	operator unsigned char () const;
	operator unsigned short () const;
	operator unsigned long long () const;
};
pure_inline bool operator> (AWKLONG l_, const struct awk_long_t &l) {return l_ >  l.l;}
pure_inline bool operator>=(AWKLONG l_, const struct awk_long_t &l) {return l_ >= l.l;}
pure_inline bool operator< (AWKLONG l_, const struct awk_long_t &l) {return l_ <  l.l;}
pure_inline bool operator<=(AWKLONG l_, const struct awk_long_t &l) {return l_ <= l.l;}
pure_inline bool operator==(AWKLONG l_, const struct awk_long_t &l) {return l_ == l.l;}
pure_inline bool operator!=(AWKLONG l_, const struct awk_long_t &l) {return l_ != l.l;}
pure_inline AWKLONG operator-(AWKLONG l_, const struct awk_long_t &l) {return l_ - l.l;}
struct awk_ulong_t {
	unsigned AWKLONG ul;
	awk_ulong_t() {}
	awk_ulong_t(unsigned ui_) {ul = ui_;}
	awk_ulong_t(unsigned AWKLONG ul_) {ul = ul_;}
	awk_ulong_t(const awk_ulong_t &ul_) {ul = ul_.ul;}
	explicit awk_ulong_t(const awk_long_t &l_) {ul = (unsigned AWKLONG) l_.l;}
	explicit awk_ulong_t(const double d_) {ul = (unsigned AWKLONG) d_;}
	explicit awk_ulong_t(const int i_) {ul = (unsigned) i_;}
	struct awk_ulong_t &operator= (const struct awk_ulong_t &ul_) {ul =  ul_.ul; return *this;}
	struct awk_ulong_t &operator+=(const struct awk_ulong_t &ul_) {ul += ul_.ul; return *this;}
	struct awk_ulong_t &operator-=(const struct awk_ulong_t &ul_) {ul -= ul_.ul; return *this;}
	struct awk_ulong_t &operator+=(const struct awk_long_t &l_) {ul += (unsigned AWKLONG) l_.l; return *this;}
	struct awk_ulong_t &operator-=(const struct awk_long_t &l_) {ul -= (unsigned AWKLONG) l_.l; return *this;}
	struct awk_ulong_t &operator%=(const struct awk_ulong_t &ul_) {ul %= ul_.ul; return *this;}
	struct awk_ulong_t &operator^=(const struct awk_ulong_t &ul_) {ul ^= ul_.ul; return *this;}
	struct awk_ulong_t &operator+=(unsigned AWKLONG ul_) {ul += ul_; return *this;}
	struct awk_ulong_t &operator-=(unsigned AWKLONG ul_) {ul -= ul_; return *this;}
	struct awk_ulong_t &operator+=(AWKLONG l_) {ul += (unsigned AWKLONG) l_; return *this;}
	struct awk_ulong_t &operator-=(AWKLONG l_) {ul -= (unsigned AWKLONG) l_; return *this;}
	struct awk_ulong_t &operator+=(int i_) {ul += (unsigned AWKLONG) i_; return *this;}
	struct awk_ulong_t &operator-=(int i_) {ul -= (unsigned AWKLONG) i_; return *this;}
	struct awk_ulong_t &operator*=(int i_) {ul *= (unsigned AWKLONG) i_; return *this;}
	struct awk_ulong_t &operator+=(unsigned u_) {ul += u_; return *this;}
	struct awk_ulong_t &operator&=(unsigned u_) {ul &= u_; return *this;}
	struct awk_ulong_t &operator+=(unsigned char c_) {ul += c_; return *this;}
	struct awk_ulong_t &operator++() {++ul; return *this;}
	struct awk_ulong_t &operator--() {--ul; return *this;}
	struct awk_ulong_t &operator/=(unsigned u_) {ul /= u_; return *this;}
	struct awk_ulong_t operator++(int) {unsigned AWKLONG r = ul++; return r;}
	struct awk_ulong_t operator--(int) {unsigned AWKLONG r = ul--; return r;}
	struct awk_ulong_t operator<<(unsigned u_) const_method {return ul << u_;}
	struct awk_ulong_t operator>>(unsigned u_) const_method {return ul >> u_;}
	bool operator> (const struct awk_ulong_t &ul_) const_method {return ul >  ul_.ul;}
	bool operator>=(const struct awk_ulong_t &ul_) const_method {return ul >= ul_.ul;}
	bool operator< (const struct awk_ulong_t &ul_) const_method {return ul <  ul_.ul;}
	bool operator<=(const struct awk_ulong_t &ul_) const_method {return ul <= ul_.ul;}
	bool operator==(const struct awk_ulong_t &ul_) const_method {return ul == ul_.ul;}
	bool operator!=(const struct awk_ulong_t &ul_) const_method {return ul != ul_.ul;}
	bool operator> (unsigned AWKLONG ul_) const_method {return ul >  ul_;}
	bool operator>=(unsigned AWKLONG ul_) const_method {return ul >= ul_;}
	bool operator< (unsigned AWKLONG ul_) const_method {return ul <  ul_;}
	bool operator<=(unsigned AWKLONG ul_) const_method {return ul <= ul_;}
	bool operator==(unsigned AWKLONG ul_) const_method {return ul == ul_;}
	bool operator!=(unsigned AWKLONG ul_) const_method {return ul != ul_;}
	bool operator> (unsigned ui_) const_method {return ul >  ui_;}
	bool operator>=(unsigned ui_) const_method {return ul >= ui_;}
	bool operator< (unsigned ui_) const_method {return ul <  ui_;}
	bool operator<=(unsigned ui_) const_method {return ul <= ui_;}
	bool operator==(unsigned ui_) const_method {return ul == ui_;}
	bool operator!=(unsigned ui_) const_method {return ul != ui_;}
#ifdef _WIN64
	bool operator<(size_t sz_) const_method {return ul < sz_;}
	bool operator>(size_t sz_) const_method {return ul > sz_;}
#endif
	size_t operator/(size_t sz_) const_method {return ul/sz_;}
	operator bool() const_method {return ul != 0ul;}
	bool operator!() const_method {return ul == 0ul;}
	awk_ulong_t operator~() const_method {return ~ul;}
	awk_ulong_t operator/(const struct awk_ulong_t &ul_) const_method {return ul / ul_.ul;}
	awk_ulong_t operator+(const struct awk_ulong_t &ul_) const_method {return ul + ul_.ul;}
	awk_ulong_t operator-(const struct awk_ulong_t &ul_) const_method {return ul - ul_.ul;}
	awk_ulong_t operator|(const struct awk_ulong_t &ul_) const_method {return ul | ul_.ul;}
	awk_ulong_t operator%(const struct awk_ulong_t &ul_) const_method {return ul % ul_.ul;}
	awk_ulong_t operator+(unsigned AWKLONG ul_) const_method {return ul + ul_;}
	awk_ulong_t operator-(unsigned AWKLONG ul_) const_method {return ul - ul_;}
	awk_ulong_t operator+(AWKLONG l_) const_method {return ul + (unsigned AWKLONG) l_;}
	awk_ulong_t operator-(AWKLONG l_) const_method {return ul - (unsigned AWKLONG) l_;}
	awk_ulong_t operator+(int i_) const_method {return ul + (unsigned AWKLONG) i_;}
	awk_ulong_t operator-(int i_) const_method {return ul - (unsigned AWKLONG) i_;}
	unsigned long long operator*(unsigned long long ull_) const_method {return ul * ull_;}
	unsigned long long operator+(unsigned long long ull_) const_method {return ul + ull_;}
	unsigned long long operator-(unsigned long long ull_) const_method {return ul - ull_;}
	size_t operator%(size_t s_) const_method {return ul%s_;}
	awk_ulong_t operator*(unsigned u_) const_method {return ul * u_;}
	awk_ulong_t operator+(unsigned u_) const_method {return ul + u_;}
	awk_ulong_t operator-(unsigned u_) const_method {return ul - u_;}
	awk_ulong_t operator+(const struct awk_long_t &l_) const_method {return ul + (unsigned AWKLONG) l_.l;}
	awk_ulong_t operator-(const struct awk_long_t &l_) const_method {return ul - (unsigned AWKLONG) l_.l;}
	operator unsigned AWKLONG () const_method {return ul;}
	operator unsigned long long () const_method {return ul;}
#ifdef _WIN64
	explicit operator unsigned long () const_method {return (unsigned long) ul;}
	explicit operator unsigned int () const_method {return (unsigned) ul;}
	explicit operator int () const_method {return (int) ul;}
	/* for use awk_ulong_t as array index */
	operator long long () const_method {return ul;}
#else
	operator size_t () const_method {return (size_t) ul;}
	/* for use awk_ulong_t as array index */
	operator int () const_method {return (int) ul;}
#endif
	explicit operator double () const_method {return (double) ul;}
	explicit operator long () const_method {return (long) ul;}
private:
	operator char () const;
	operator short () const;
	operator unsigned char () const;
	operator unsigned short () const;
#ifndef _WIN64
	operator long long () const;
#endif
};
pure_inline bool operator> (unsigned AWKLONG ul_, const struct awk_ulong_t &ul) {return ul_ >  ul.ul;}
pure_inline bool operator>=(unsigned AWKLONG ul_, const struct awk_ulong_t &ul) {return ul_ >= ul.ul;}
pure_inline bool operator< (unsigned AWKLONG ul_, const struct awk_ulong_t &ul) {return ul_ <  ul.ul;}
pure_inline bool operator<=(unsigned AWKLONG ul_, const struct awk_ulong_t &ul) {return ul_ <= ul.ul;}
pure_inline bool operator==(unsigned AWKLONG ul_, const struct awk_ulong_t &ul) {return ul_ == ul.ul;}
pure_inline bool operator!=(unsigned AWKLONG ul_, const struct awk_ulong_t &ul) {return ul_ != ul.ul;}
pure_inline bool operator==(size_t sz_, const struct awk_ulong_t &ul) {return sz_ == ul.ul;}
pure_inline bool operator!=(size_t sz_, const struct awk_ulong_t &ul) {return sz_ != ul.ul;}
pure_inline bool operator> (size_t sz_, const struct awk_ulong_t &ul) {return sz_ >  ul.ul;}
pure_inline bool operator>=(uint32_t u32_, const struct awk_ulong_t &ul) {return u32_ >= ul.ul;}
#ifdef _WIN64
pure_inline bool operator>=(size_t sz_, const struct awk_ulong_t &ul) {return sz_ >= ul.ul;}
#endif
pure_inline size_t operator-(size_t sz_, const struct awk_ulong_t &ul) {return sz_ - ul.ul;}
pure_inline size_t operator+(size_t sz_, const struct awk_ulong_t &ul) {return sz_ + ul.ul;}
pure_inline size_t operator*(size_t sz_, const struct awk_ulong_t &ul) {return sz_ * ul.ul;}
pure_inline awk_ulong_t operator+(unsigned char c, const struct awk_ulong_t &ul) {return c + ul.ul;}
#ifdef _WIN64
pure_inline awk_ulong_t operator*(unsigned u_, const struct awk_ulong_t &ul) {return u_ * ul.ul;}
#endif
pure_inline double operator/(double d_, const struct awk_ulong_t &ul) {return d_ / ul.ul;}
inline size_t operator+=(size_t &sz_, const struct awk_ulong_t &ul) {return sz_ += ul.ul;}
inline size_t operator-=(size_t &sz_, const struct awk_ulong_t &ul) {return sz_ -= ul.ul;}
inline uint32_t operator%=(uint32_t &u32_, const struct awk_ulong_t &ul) {return u32_ %= ul.ul;}
#ifdef _WIN64
inline size_t operator%=(size_t &sz_, const struct awk_ulong_t &ul) {return sz_ %= ul.ul;}
#endif
pure_inline unsigned AWKLONG TO_AWK_ULONG(const struct awk_ulong_t &ul) {return ul.ul;}
pure_inline AWKLONG TO_AWK_LONG(const struct awk_long_t &l) {return l.l;}
#undef pure_inline
#undef const_method
#if defined __GNUC__ && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#endif
#endif /* __cplusplus >= 201103L */
