/** \file common.cpp
 * Common functions
 *
 * $Id: common.cpp,v 1.24 2002/06/11 10:49:40 lecroart Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#include "stdmisc.h"

#ifdef NL_OS_WINDOWS
#include <windows.h>
#elif defined NL_OS_UNIX
#include <unistd.h>
#endif

using namespace std;

#ifdef NL_OS_WINDOWS
#  ifdef __STL_DEBUG
#    define STL_MODE "debug"
#  else
#    define STL_MODE "normal"
#  endif // __STL_DEBUG
#  if(__SGI_STL_PORT < 0x400)
#    define STL_STR_VER "< 4.0"
#  elif(__SGI_STL_PORT == 0x400)
#    define STL_STR_VER "4.0"
#  elif(__SGI_STL_PORT == 0x450)
#    define STL_STR_VER "4.5"
#  elif(__SGI_STL_PORT > 0x450)
#    define STL_STR_VER "> 4.5"
#  endif // __SGI_STL_PORT
#  pragma message("Using STLport version "STL_STR_VER" in "STL_MODE" mode")
#endif // NL_OS_WINDOWS

namespace	NLMISC
{

/*
 * Portable Sleep() function that suspends the execution of the calling thread for a number of milliseconds.
 * Note: the resolution of the timer is system-dependant and may be more than 1 millisecond.
 */
void nlSleep( uint32 ms )
{
#ifdef NL_OS_WINDOWS

/// \todo yoyo: BUG WITH DEBUG/_CONSOLE!!!! a Sleep(0) "block" the other thread!!!
#ifdef NL_DEBUG
	ms = max(ms, (uint32)1);
#endif

	Sleep( ms );

#elif defined NL_OS_UNIX
	usleep( ms*1000 );
#endif
}


/*
 * Returns Thread Id (note: on Linux, Process Id is the same as the Thread Id)
 */
uint getThreadId()
{
#ifdef NL_OS_WINDOWS
	return GetCurrentThreadId();
#elif defined NL_OS_UNIX
	return getpid();
#endif

}


/*
 * Returns a readable string from a vector of bytes. '\0' are replaced by ' '
 */
string stringFromVector( const vector<uint8>& v, bool limited )
{
	string s;

	if (!v.empty())
	{
		int size = v.size ();
		if (limited && size > 1000)
		{
			string middle = "...<buf too big,skip middle part>...";
			s.resize (1000 + middle.size());
			memcpy (&*s.begin(), &*v.begin(), 500);
			memcpy (&*s.begin()+500, &*middle.begin(), middle.size());
			memcpy (&*s.begin()+500+middle.size(), &*v.begin()+size-500, 500);
		}
		else
		{
			s.resize (size);
			memcpy( &*s.begin(), &*v.begin(), v.size() );
		}

		// Replace '\0' characters
		string::iterator is;
		for ( is=s.begin(); is!=s.end(); ++is )
		{
			// remplace non printable char and % with '?' chat
			if ( ! isprint((uint8)(*is)) || (*is) == '%')
			{
				(*is) = '?';
			}
		}
	}
/*
	if ( ! v.empty() )
	{
		// Copy contents
		s.resize( v.size() );
		memcpy( &*s.begin(), &*v.begin(), v.size() );

		// Replace '\0' characters
		string::iterator is;
		for ( is=s.begin(); is!=s.end(); ++is )
		{
			// remplace non printable char and % with '?' chat
			if ( ! isprint((*is)) || (*is) == '%')
			{
				(*is) = '?';
			}
		}
	}
*/	return s;
}


sint smprintf( char *buffer, size_t count, const char *format, ... )
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


sint64 atoiInt64 (const char *ident, sint64 base)
{
	sint64 number = 0;
	bool neg = false;

	// NULL string
	nlassert (ident != NULL);

	// empty string
	if (*ident == '\0') goto end;
	
	// + sign
	if (*ident == '+') ident++;

	// - sign
	if (*ident == '-') { neg = true; ident++; }

	while (*ident != '\0')
	{
		if (isdigit(*ident))
		{
			number *= base;
			number += (*ident)-'0';
		}
		else if (base > 10 && islower(*ident))
		{
			number *= base;
			number += (*ident)-'a'+10;
		}
		else if (base > 10 && isupper(*ident))
		{
			number *= base;
			number += (*ident)-'A'+10;
		}
		else
		{
			goto end;
		}
		ident++;
	}
end:
	if (neg) number = -number;
	return number;

///	old sameh algo
/*	uint64 k = 0;

	while (*ident != '\0')
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
*/
}

void itoaInt64 (sint64 number, char *str, sint64 base)
{
	str[0] = '\0';
	char b[256];
	if(!number)
	{
		str[0] = '0';
		str[1] = '\0';
		return;
	}
	memset(b,'\0',255);
	memset(b,'0',64);
	sint n;
	sint64 x = number;
	if (x < 0) x = -x;
	char baseTable[] = "0123456789abcdefghijklmnopqrstuvwyz";
	for(n = 0; n < 64; n ++)
	{
		sint num = (sint)(x % base);
		b[64 - n] = baseTable[num];
		if(!x) 
		{
			int k;
			int j = 0;
	
			if (number < 0)
			{
				str[j++] = '-';
			}

			for(k = 64 - n + 1; k <= 64; k++) 	
			{
				str[j ++] = b[k];
			}
			str[j] = '\0';
			break;
		}
		x /= base;
	}
}

uint raiseToNextPowerOf2(uint v)
{
	uint	res=1;
	while(res<v)
		res<<=1;
	
	return res;
}

uint	getPowerOf2(uint v)
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

bool isPowerOf2(sint32 v)
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

std::string strlwr ( const std::string &str )
{
	string res;
	res.reserve (str.size());
	for (uint i=0; i<str.size(); i++)
	{
		if ( (str[i] >= 'A') && (str[i] <= 'Z') )
			res += str[i] - 'A' + 'a';
		else
			res += str[i];
	}

	return res;
}

std::string &strlwr ( std::string &str )
{
	for (int i=str.size()-1; i>=0; i--)
	{
		if ( (str[i] >= 'A') && (str[i] <= 'Z') )
		{
			str[i] = str[i] - 'A' + 'a';
		}
	}

	return (str);
}

char *strlwr ( char *str )
{
	if (str == NULL)
		return (NULL);

	while (*str != '\0')
	{
		if ( (*str >= 'A') && (*str <= 'Z') )
		{
			*str = *str - 'A' + 'a';
		}
		str++;
	}

	return (str);
}

std::string &strupr ( std::string &str )
{
	for (int i=str.size()-1; i>=0; i--)
	{
		if ( (str[i] >= 'a') && (str[i] <= 'z') )
		{
			str[i] = str[i] - 'a' + 'A';
		}
	}

	return (str);
}

char *strupr ( char *str )
{
	if (str == NULL)
		return (NULL);

	while (*str != '\0')
	{
		if ( (*str >= 'a') && (*str <= 'z') )
		{
			*str = *str - 'a' + 'A';
		}
		str++;
	}

	return (str);
}

sint nlstricmp(const char *lhs,const char *rhs)
{
	nlassert(lhs);
	nlassert(rhs);
	uint lchar, rchar;
	while (*lhs != '\0' && *rhs != '\0')
	{
		lchar = ::toupper(*lhs);
		rchar = ::toupper(*rhs);
		if (lchar != rchar) return lchar - rchar;
		++lhs;
		++rhs;
	}
	if (*lhs != 0) return 1;
	if (*rhs != 0) return -1;
	return 0;
}

sint nlstricmp(const std::string &lhs,const std::string &rhs)
{
	return nlstricmp(lhs.c_str(), rhs.c_str());
}



//
// Exceptions
//

Exception::Exception() : _Reason("Unknown Exception")
{
//	nlwarning("Exception will be launched: %s", _Reason.c_str());
}

Exception::Exception(const std::string &reason) : _Reason(reason)
{
	nlwarning("Exception will be launched: %s", _Reason.c_str());
}

Exception::Exception(const char *format, ...)
{
	NLMISC_CONVERT_VARGS (_Reason, format, NLMISC::MaxCStringSize);
	nlwarning("Exception will be launched: %s", _Reason.c_str());
}

const char	*Exception::what() const throw()
{
	return _Reason.c_str();
}


} // NLMISC

