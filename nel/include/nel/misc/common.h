/** \file common.h
 * common algorithms, constants and functions
 *
 * $Id: common.h,v 1.28 2001/05/28 12:40:31 chafik Exp $
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
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>


namespace	NLMISC
{

// Windows posix function remapping
#ifdef NL_OS_WINDOWS
#define vsnprintf _vsnprintf
#endif // NL_OS_WINDOWS



/**
 * \def MaxCStringSize
 *
 * The maximum size allowed for C string (zero terminated string) buffer.
 * This value is used when we have to create a standard C string buffer and we don't know exactly the final size of the string.
 */
const int MaxCStringSize = 1024*64;


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
int res = vsnprintf (_cstring, _size-1, _format, _args); \
if (res == -1 || res == _size-1) \
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
inline sint smprintf( char *buffer, size_t count, const char *format, ... )
{
	sint ret;

	va_list args;
	va_start( args, format );
	ret = vsnprintf( buffer, count, format, args );
	if ( ret == -1 )
	{
		buffer[count-1] = '\0';
	}
	va_end( args );

	return( ret );
}


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
	new (&a) T;
}

/** Return the value maximized to the next power of 2 of v.
 * Example:
 *   raiseToNextPowerOf2(8) is 8
 *   raiseToNextPowerOf2(5) is 8
 */
inline uint	raiseToNextPowerOf2(uint v)
{
	uint	res=1;
	while(res<v)
		res<<=1;
	
	return res;
}

/** Return the power of 2 of v.
 * Example:
 *   getPowerOf2(8) is 3
 *   getPowerOf2(5) is 3
 */
inline uint	getPowerOf2(uint v)
{
	uint	res=1;
	uint	ret=0;
	while(res<v)
	{
		ret++;
		res<<=1;
	}
	
	return ret;
}


/** Return \c true if the value is a power of 2.
 */
inline bool isPowerOf2(sint32 v)
{
	while(v)
	{
		if(v&1)
		{
			v>>=1;
			if(v)
				return false;
		}
		else
			v>>=1;
	}

	return true;
}


/** Convert a string in lower case.
 * \param a string to transform to lower case
 */
inline std::string &strlwr ( std::string &str )
{
	for (int i=str.size()-1; i>=0; i--)
	{
		str[i] = tolower(str[i]);
	}

	return (str);
}

/** Convert a string in lower case.
 * \param a pointer to char to transform to lower case
 */
inline char *strlwr ( char *str )
{
	if (str == NULL)
		return (NULL);

	while (*str != '\0')
	{
		*str = tolower(*str);
		str++;
	}

	return (str);
}

/** Convert a string in upper case.
 * \param a string to transform to upper case
 */
inline std::string &strupr ( std::string &str )
{
	for (int i=str.size()-1; i>=0; i--)
	{
		str[i] = toupper(str[i]);
	}

	return (str);
}

/** Convert a string in upper case.
 * \param a pointer to char to transform to upper case
 */
inline char *strupr ( char *str )
{
	if (str == NULL)
		return (NULL);

	while (*str != '\0')
	{
		*str = toupper(*str);
		str++;
	}

	return (str);
}


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
	virtual const char	*what() const throw();
};


/**
 * Portable Sleep() function that suspends the execution of the calling thread for a number of milliseconds.
 * Note: the resolution of the timer is system-dependant and may be more than 1 millisecond.
 */
void nlSleep( uint32 ms );


/// Returns Process Id (note: on Linux, Process Id is the same as the Thread Id)
#ifdef NL_OS_WINDOWS
#define getpid _getpid
#endif

/// Returns Thread Id (note: on Linux, Process Id is the same as the Thread Id)
uint getThreadId();

/// Returns a readable string from a vector of bytes. '\0' are replaced by ' '
std::string stringFromVector( const std::vector<uint8>& v );


/** Returns a string corresponding to the class T in string format
 * Example:
 *  string num = toString (1234); // num = "1234";
 */
template<class T> std::string toString (const T &t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}

inline sint64 atoiInt64(const char *ident,sint64 base = 10)
{
	uint64 k = 0;

	while(*ident != 0)
	{
		switch(*(ident++))
		{
		case '0':
			k +=0;
			break;
		case '1':
			k +=1;
			break;
		case '2':
			k +=2;
			break;
		case '3':
			k +=3;
			break;
		case '4':
			k +=4;
			break;
		case '5':
			k +=5;
			break;
		case '6':
			k +=6;
			break;
		case '7':
			k +=7;
			break;
		case '8':
			k +=8;
			break;
		case '9':
			k +=9;
			break;
		case 'a':
			k +=10;
			break;
		case 'b':
			k +=11;
			break;
		case 'c':
			k +=12;
			break;
		case 'd':
			k +=13;
			break;
		case 'e':
			k +=14;
			break;
		case 'f':
			k +=15;
			break;

		case 'A':
			k +=10;
			break;
		case 'B':
			k +=11;
			break;
		case 'C':
			k +=12;
			break;
		case 'D':
			k +=13;
			break;
		case 'E':
			k +=14;
			break;
		case 'F':
			k +=15;
			break;

		case 0:
			return k;
			break;
		}
		if(*ident != 0) k *= base;
	}

	return k;
}


inline void itoaInt64(sint64 numbre,char *str,sint64 base = 10) 
{									
	str[0] = 0;
	char b[256];
	if(!numbre)
	{
		str[0] = '0';
		str[1] = 0;
	}
	memset(b,0,255);
	memset(b,'0',64);
	sint n;
	uint64 x = numbre;
	char baseTable[] = "0123456789abcdefghijklmnopqrstuvwyz";
	for(n = 0; n < 64; n ++)
	{
		sint num = (sint)(x % base);
		b[64 - n] = baseTable[num];
		if(!x) 
		{
			int k;
			int j = 0;
			for(k = 64 - n + 1; k <= 64; k++) 	
			{
				str[j ++] = b[k];
			}
			str[j] = 0;
			break;
		}
		x /= base;
	}
		
}


}	// NLMISC

#endif	// NL_COMMON_H
