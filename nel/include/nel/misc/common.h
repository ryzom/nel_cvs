/** \file misc/common.h
 * common algorithms, constants and functions
 *
 * $Id: common.h,v 1.65 2003/11/20 15:38:26 distrib Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef	NL_COMMON_H
#define	NL_COMMON_H

#include "nel/misc/types_nl.h"

#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
#include <float.h>

#ifdef NL_OS_WINDOWS
#	include <process.h>
#else
#	include <math.h>
#	include <unistd.h>
#	include <sys/types.h>
#endif

/// This namespace contains all miscellaneous class used by other module
namespace	NLMISC
{

// Windows posix function remapping
#ifdef NL_OS_WINDOWS
#define vsnprintf _vsnprintf
#endif // NL_OS_WINDOWS


/** Read the time stamp counter. Supports only intel architectures for now  
  */ 
#ifdef NL_CPU_INTEL

inline uint64 rdtsc()
{
	uint64 ticks;
#	ifndef NL_OS_WINDOWS		
		__asm__ volatile(".byte 0x0f, 0x31" : "=a" (ticks.low), "=d" (ticks.high));				
#	else 		
		__asm	rdtsc
		__asm	mov		DWORD PTR [ticks], eax
		__asm	mov		DWORD PTR [ticks + 4], edx		
#	endif
	return ticks;	
}

#endif	// NL_CPU_INTEL


/**
 * \def MaxCStringSize
 *
 * The maximum size allowed for C string (zero terminated string) buffer.
 * This value is used when we have to create a standard C string buffer and we don't know exactly the final size of the string.
 */
const int MaxCStringSize = 1024*2;


/** Pi constant in double format.
 */
const double Pi = 3.1415926535897932384626433832795;


/**
 * \def NLMISC_CONVERT_VARGS(dest,format)
 *
 * This macro converts variable arguments into C string (zero terminated string).
 * This function takes care to avoid buffer overflow.
 *
 * Example:
 *\code
	void MyFunction(const char *format, ...)
	{
		string str;
		NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);
		// str contains the result of the conversion
	}
 *\endcode
 *
 * \param _dest \c string or \c char* that contains the result of the convertion
 * \param _format format of the string, it must be the last argument before the \c '...'
 * \param _size size of the buffer that will contain the C string
 */
#define NLMISC_CONVERT_VARGS(_dest,_format,_size) \
char _cstring[_size]; \
va_list _args; \
va_start (_args, _format); \
int _res = vsnprintf (_cstring, _size-1, _format, _args); \
if (_res == -1 || _res == _size-1) \
{ \
	_cstring[_size-1] = '\0'; \
} \
va_end (_args); \
_dest = _cstring



/** sMart sprintf function. This function do a sprintf and add a zero at the end of the buffer
 * if there no enough room in the buffer.
 *
 * \param buffer a C string
 * \param count Size of the buffer
 * \param format of the string, it must be the last argument before the \c '...'
 */
sint smprintf( char *buffer, size_t count, const char *format, ... );


/** Return a float random inside the interval [0,mod]
 */
inline float frand(float mod)
{
	double	r = (double) rand();
	r/= (double) RAND_MAX;
	return (float)(r * mod);
}


/** Return -1 if f<0, 0 if f==0, 1 if f>1
 */
inline sint fsgn(double f)
{
	if(f<0)
		return -1;
	else if(f>0)
		return 1;
	else
		return 0;
}


/** Return the square of a number
 */
template<class T>	inline T sqr(const T &v)
{
	return v * v;
}


/** Force v to be inside the interval [min,max]. Warning: implicit cast are made if T,U or V are different.
 */
template<class T, class U, class V>	inline void clamp(T &v, const U &min, const V &max)
{
	v = (v < min) ? min : v;
	v = (v > max) ? max : v;
}


/** MIN/MAX extended functions.
 */
template<class T>	inline T minof(const T& a,  const T& b,  const T& c)
	{return std::min(std::min(a,b),c);}
template<class T>	inline T minof(const T& a,  const T& b,  const T& c,  const T& d)
	{return std::min(minof(a,b,c),d);}
template<class T>	inline T minof(const T& a,  const T& b,  const T& c,  const T& d,  const T& e)
	{return std::min(minof(a,b,c,d),e);}
template<class T>	inline T maxof(const T& a,  const T& b,  const T& c)
	{return std::max(std::max(a,b),c);}
template<class T>	inline T maxof(const T& a,  const T& b,  const T& c,  const T& d)
	{return std::max(maxof(a,b,c),d);}
template<class T>	inline T maxof(const T& a,  const T& b,  const T& c,  const T& d,  const T& e)
	{return std::max(maxof(a,b,c,d),e);}

/** \c contReset take a container like std::vector or std::deque and put his size to 0 like \c clear() but free all buffers.
 * This function is useful because \c resize(), \c clear(), \c erase() or \c reserve() methods never realloc when the array size come down.
 * \param a is the container to reset.
 */
template<class T>	inline void contReset (T& a)
{
	a.~T();
#undef new
	new (&a) T;
#define new NL_NEW
}

/** Return the value maximized to the next power of 2 of v.
 * Example:
 *   raiseToNextPowerOf2(8) is 8
 *   raiseToNextPowerOf2(5) is 8
 */
uint			raiseToNextPowerOf2 (uint v);

/** Return the power of 2 of v.
 * Example:
 *   getPowerOf2(8) is 3
 *   getPowerOf2(5) is 3
 */
uint			getPowerOf2 (uint v);

/** Return \c true if the value is a power of 2.
 */
bool			isPowerOf2 (sint32 v);


/** Converts from degrees to radians
 */
inline float	degToRad( float deg )
{
	return deg * (float)Pi / 180.0f;
}


/** Converts from radians to degrees
 */
inline float	radToDeg( float rad )
{
	return rad * 180.0f / (float)Pi;
}


/** Return true if double is a valid value (not inf nor nan)
 */
inline double	isValidDouble (double v)
{
#ifdef NL_OS_WINDOWS
	return _finite(v) && !_isnan(v);
#else
	return !isnan(v) && !isinf(v);
#endif
}


/** Convert a string in lower case.
 * \param a string to transform to lower case
 */
std::string		&strlwr ( std::string &str );
std::string		strlwr ( const std::string &str );

/** Convert a string in lower case.
 * \param a pointer to char to transform to lower case
 */
char			*strlwr ( char *str );

/** Convert a string in upper case.
 * \param a string to transform to upper case
 */
std::string		&strupr ( std::string &str );
std::string		strupr ( const std::string &str );

/** Convert a string in upper case.
 * \param a pointer to char to transform to upper case
 */
char			*strupr ( char *str );

/** Compare 2 C-Style strings without regard to case
  * \return 0 if strings are equal, < 0 if lhs < rhs, > 0 if lhs > rhs
  *
  * On Windows,   use stricmp
  * On GNU/Linux, create stricmp using strcasecmp and use stricmp
  */
#ifndef NL_OS_WINDOWS
inline int stricmp(const char *lhs, const char *rhs) { return strcasecmp(lhs, rhs); }
#endif

inline sint nlstricmp(const char *lhs, const char *rhs) { return stricmp(lhs, rhs); }
inline sint nlstricmp(const std::string &lhs, const std::string &rhs) { return stricmp(lhs.c_str(), rhs.c_str()); }
inline sint nlstricmp(const std::string &lhs, const char *rhs) { return stricmp(lhs.c_str(),rhs); }
inline sint nlstricmp(const char *lhs, const std::string &rhs) { return stricmp(lhs,rhs.c_str()); }

/** Signed 64 bit fseek. Same interface than fseek
  */
int		nlfseek64( FILE *stream, sint64 offset, int origin );

/**
 * Base class for all NeL exception.
 * It enables to construct simple string at the ctor.
 */
class Exception : public std::exception
{
protected:
	std::string	_Reason;
public:
	Exception();
	Exception(const std::string &reason);
	Exception(const char *format, ...);
	virtual ~Exception() throw() {};
	virtual const char	*what() const throw();
};


/**
 * Portable Sleep() function that suspends the execution of the calling thread for a number of milliseconds.
 * Note: the resolution of the timer is system-dependant and may be more than 1 millisecond.
 */
void nlSleep( uint32 ms );


/// Returns Process Id (note: on Linux, Process Id is the same as the Thread Id)
#ifdef NL_OS_WINDOWS
#	define getpid _getpid
#endif

/// Returns Thread Id (note: on Linux, Process Id is the same as the Thread Id)
uint getThreadId();

/// Returns a readable string from a vector of bytes. unprintable char are replaced by '?'
std::string stringFromVector( const std::vector<uint8>& v, bool limited = true );


/// Convert a string into an sint64 (same as atoi() function but for 64 bits intergers)
sint64 atoiInt64 (const char *ident, sint64 base = 10);

/// Convert an sint64 into a string (same as itoa() function but for 64 bits intergers)
void itoaInt64 (sint64 number, char *str, sint64 base = 10);


/// Convert a number in bytes into a string that is easily readable by an human, for example 105123 -> "102kb"
std::string bytesToHumanReadable (const std::string &bytes);
std::string bytesToHumanReadable (uint32 bytes);

/// Convert a human readable into a bytes,  for example "102kb" -> 105123
uint32 humanReadableToBytes (const std::string &str);

/// Convert a time into a string that is easily readable by an human, for example 3600 -> "1h"
std::string secondsToHumanReadable (uint32 time);


/// Get a bytes or time in string format and convert it in seconds or bytes
uint32 fromHumanReadable (const std::string &str);


/// This function execute a program in the background and returns instantly (used for example to launch services in AES).
/// The program will be launch in the current directory
bool launchProgram (const std::string &programName, const std::string &arguments);



#ifdef NL_OS_WINDOWS

//
// These functions are needed by template system to failed the compilation if you pass bad type on nlinfo...
//

inline void _check(int a) { }
inline void _check(unsigned int a) { }
inline void _check(char a) { }
inline void _check(unsigned char a) { }
inline void _check(long a) { }
inline void _check(unsigned long a) { }
#ifndef NL_COMP_VC7
inline void _check(uint8 a) { }
#endif
inline void _check(sint8 a) { }
inline void _check(uint16 a) { }
inline void _check(sint16 a) { }
#ifndef NL_COMP_VC7
inline void _check(uint32 a) { }
inline void _check(sint32 a) { }
#endif
inline void _check(uint64 a) { }
inline void _check(sint64 a) { }
inline void _check(float a) { }
inline void _check(double a) { }
inline void _check(const char *a) { }
inline void _check(const void *a) { }

#define CHECK_TYPES(__a,__b) \
	inline __a(const char *fmt) { __b(fmt); } \
	template<class A> __a(const char *fmt, A a) { _check(a); __b(fmt, a); } \
	template<class A, class B> __a(const char *fmt, A a, B b) { _check(a); _check(b); __b(fmt, a, b); } \
	template<class A, class B, class C> __a(const char *fmt, A a, B b, C c) { _check(a); _check(b); _check(c); __b(fmt, a, b, c); } \
	template<class A, class B, class C, class D> __a(const char *fmt, A a, B b, C c, D d) { _check(a); _check(b); _check(c); _check(d); __b(fmt, a, b, c, d); } \
	template<class A, class B, class C, class D, class E> __a(const char *fmt, A a, B b, C c, D d, E e) { _check(a); _check(b); _check(c); _check(d); _check(e); __b(fmt, a, b, c, d, e); } \
	template<class A, class B, class C, class D, class E, class F> __a(const char *fmt, A a, B b, C c, D d, E e, F f) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); __b(fmt, a, b, c, d, e, f); } \
	template<class A, class B, class C, class D, class E, class F, class G> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); __b(fmt, a, b, c, d, e, f, g); } \
	template<class A, class B, class C, class D, class E, class F, class G, class H> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); __b(fmt, a, b, c, d, e, f, g, h); } \
	template<class A, class B, class C, class D, class E, class F, class G, class H, class I> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); __b(fmt, a, b, c, d, e, f, g, h, i); } \
	template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); __b(fmt, a, b, c, d, e, f, g, h, i, j); } \
	template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); __b(fmt, a, b, c, d, e, f, g, h, i, j, k); } \
	template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l); } \
	template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m); } \
	template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n); } \
	template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o); } \
	template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p); } \
	template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q); } \
	template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r); } \
	template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s); } \
	template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t); } \
	template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u); } \
	template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v); } \
	template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w); } \
	template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W, class X> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w, X x) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); _check(x); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x); } \
	template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W, class X, class Y> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w, X x, Y y) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); _check(x); _check(y); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y); } \
	template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W, class X, class Y, class Z> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w, X x, Y y, Z z) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); _check(x); _check(y); _check(z); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z); }
#endif


/** Returns a string corresponding to the class T in string format.
 * Example:
 *  string num = toString (1234); // num = "1234";
 */
template<class T> std::string toString (const T &t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}


/** Returns a string corresponding to the format and parameter (like printf).
 * Example:
 *  string hexnum = toString ("%x", 255); // hexnum = "ff";
 */
#ifdef NL_OS_WINDOWS
inline std::string _toString (const char *format, ...)
#else
inline std::string toString (const char *format, ...)
#endif
{
	std::string Result;
	NLMISC_CONVERT_VARGS (Result, format, NLMISC::MaxCStringSize);
	return Result;
}

#ifdef NL_OS_WINDOWS
CHECK_TYPES(std::string toString, return _toString)
#endif // NL_OS_WINDOWS



#ifdef NL_OS_UNIX
inline std::string toString (const uint8 &t)
{
	std::stringstream ss;
	ss << (unsigned int)t;
	return ss.str();
}

inline std::string toString (const sint8 &t)
{
	std::stringstream ss;
	ss << (unsigned int)t;
	return ss.str();
}
#endif // NL_OS_UNIX


/** Explode a string into a vector of string with *sep* as separator. If sep can be more than 1 char, in this case,
 * we find the entire sep to separator (it s not a set of possible separator)
 *
 * \param skipEmpty if true, we don't put in the res vector empty string
 */
void explode (const std::string &src, const std::string &sep, std::vector<std::string> &res, bool skipEmpty = false);


/* All the code above is used to add our types (uint8, ...) in the stringstream (used by the toString() function).
 * So we can use stringstream operator << and >> with all NeL simple types (except for ucchar and ucstring)
 */

#ifdef NL_OS_WINDOWS

#if _MSC_VER < 1300	// visual or older (on visual .NET, we don't need to do that)

#define NLMISC_ADD_BASIC_ISTREAM_OPERATOR(__type,__casttype) \
template <class _CharT, class _Traits> \
std::basic_istream<_CharT, _Traits>& __cdecl \
operator>>(std::basic_istream<_CharT, _Traits>& __is, __type& __z) \
{ \
	__casttype __z2 = (__casttype) __z; \
	__is.operator>>(__z2); \
	__z = (__type) __z2; \
	return __is; \
} \
 \
template <class _CharT, class _Traits> \
std::basic_ostream<_CharT, _Traits>& __cdecl \
operator<<(std::basic_ostream<_CharT, _Traits>& __os, const __type& __z) \
{ \
	std::basic_ostringstream<_CharT, _Traits, std::allocator<_CharT> > __tmp; \
	__tmp << (__casttype) __z; \
	return __os << __tmp.str(); \
}

NLMISC_ADD_BASIC_ISTREAM_OPERATOR(uint8, unsigned int);
NLMISC_ADD_BASIC_ISTREAM_OPERATOR(sint8, signed int);
NLMISC_ADD_BASIC_ISTREAM_OPERATOR(uint16, unsigned int);
NLMISC_ADD_BASIC_ISTREAM_OPERATOR(sint16, signed int);
NLMISC_ADD_BASIC_ISTREAM_OPERATOR(uint32, unsigned int);
NLMISC_ADD_BASIC_ISTREAM_OPERATOR(sint32, signed int);

#endif // _MSC_VER < 1300


template <class _CharT, class _Traits>
std::basic_istream<_CharT, _Traits>& __cdecl
operator>>(std::basic_istream<_CharT, _Traits>& __is, uint64& __z)
{
	__z = 0;
	bool neg = false;
	char c;
	do
	{
		__is >> c;
	}
	while (isspace(c));

	if (c == '-')
	{
		neg = true;
		__is >> c;
	}

	while (isdigit(c))
	{
		__z *= 10;
		__z += c-'0';
		__is >> c;
		if (__is.fail())
			break;
	}

	if (neg) __z = 0;

	return __is;
}

template <class _CharT, class _Traits>
std::basic_ostream<_CharT, _Traits>& __cdecl
operator<<(std::basic_ostream<_CharT, _Traits>& __os, const uint64& __z)
{
	std::basic_ostringstream<_CharT, _Traits, std::allocator<_CharT> > __res;

	if (__z == 0)
	{
		__res << '0';
	}
	else
	{
		std::basic_ostringstream<_CharT, _Traits, std::allocator<_CharT> > __tmp;
		uint64	__z2 = __z;
		while (__z2 != 0)
		{
			__tmp << (char)((__z2%10)+'0');
			__z2 /= 10;
		}

		uint __s = __tmp.str().size();
		for (uint i = 0; i < __s; i++)
			__res << __tmp.str()[__s - 1 - i];
	}
	return __os << __res.str();
}

template <class _CharT, class _Traits>
std::basic_istream<_CharT, _Traits>& __cdecl
operator>>(std::basic_istream<_CharT, _Traits>& __is, sint64& __z)
{
	__z = 0;
	bool neg = false;
	char c;
	do
	{
		__is >> c;
	}
	while (isspace(c));

	if (c == '-')
	{
		neg = true;
		__is >> c;
	}

	while (isdigit(c))
	{
		__z *= 10;
		__z += c-'0';
		__is >> c;
		if (__is.fail())
			break;
	}

	if (neg) __z = -__z;

	return __is;
}

template <class _CharT, class _Traits>
std::basic_ostream<_CharT, _Traits>& __cdecl
operator<<(std::basic_ostream<_CharT, _Traits>& __os, const sint64& __z)
{
	std::basic_ostringstream<_CharT, _Traits, std::allocator<_CharT> > __res;

	if (__z == 0)
	{
		__res << '0';
	}
	else 
	{
		sint64	__z2 = __z;

		if (__z2 < 0)
		{
			__res << '-';
		}

		std::basic_ostringstream<_CharT, _Traits, std::allocator<_CharT> > __tmp;
		while (__z2 != 0)
		{
			if (__z2 < 0)
			{
				__tmp << (char)((-(__z2%10))+'0');
			}
			else
			{
				__tmp << (char)((__z2%10)+'0');
			}
			__z2 /= 10;
		}

		uint __s = __tmp.str().size();
		for (uint i = 0; i < __s; i++)
			__res << __tmp.str()[__s - 1 - i];
	}
	return __os << __res.str();
}

#endif // NL_OS_WINDOWS


class CLog;

/// Display the bits (with 0 and 1) composing a byte (from right to left)
void displayByteBits( uint8 b, uint nbits, sint beginpos, bool displayBegin, NLMISC::CLog *log );

/// Display the bits (with 0 and 1) composing a number (uint32) (from right to left)
void displayDwordBits( uint32 b, uint nbits, sint beginpos, bool displayBegin, NLMISC::CLog *log );


} // NLMISC

#endif	// NL_COMMON_H
