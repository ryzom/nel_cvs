/** \file common.cpp
 * Common functions
 *
 * $Id: common.cpp,v 1.64.4.1 2004/10/28 17:37:46 corvazier Exp $
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
#  include <windows.h>
#  include <io.h>
#  undef min
#  undef max
#elif defined NL_OS_UNIX
#  include <unistd.h>
#  include <string.h>
#  include <errno.h>
#  include <signal.h>
#  include <pthread.h>
#endif

#include "nel/misc/command.h"
#include "nel/misc/path.h"

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
#  elif(__SGI_STL_PORT == 0x452)
#    define STL_STR_VER "4.5.3"
#  elif(__SGI_STL_PORT > 0x452)
#    define STL_STR_VER "> 4.5.3"
#  else
#    define STL_STR_VER "Unknown"
#  endif // __SGI_STL_PORT
#  pragma message("Using STLport version "STL_STR_VER" in "STL_MODE" mode")

#if FINAL_VERSION
#  pragma message("Using **** FINAL_VERSION ****")
#else
#  pragma message("Not using FINAL_VERSION")
#endif

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
	//usleep( ms*1000 ); // resolution: 20 ms!

	timespec ts;
	ts.tv_sec = ms/1000;
	ts.tv_nsec = (ms%1000)*1000000;
	int res;
	do
	{
		res = nanosleep( &ts, &ts ); // resolution: 10 ms (with common scheduling policy)
	}
	while ( (res != 0) && (errno==EINTR) );
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
	return pthread_self();
	// doesnt work on linux kernel 2.6	return getpid();
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

string bytesToHumanReadable (const std::string &bytes)
{
	static char *divTable[]= { "B", "kB", "mB", "gB" };
	uint div = 0;
	uint64 res = atoiInt64(bytes.c_str());
	uint64 newres = res;
	while (true)
	{
		newres /= 1024;
		if(newres < 8 || div > 2)
			break;
		div++;
		res = newres;
	}
	return toString ("%"NL_I64"u%s", res, divTable[div]);
}

string bytesToHumanReadable (uint32 bytes)
{
	static char *divTable[]= { "B", "kB", "mB", "gB" };
	uint div = 0;
	uint32 res = bytes;
	uint32 newres = res;
	while (true)
	{
		newres /= 1024;
		if(newres < 8 || div > 2)
			break;
		div++;
		res = newres;
	}
	return toString ("%u%s", res, divTable[div]);
}

uint32 humanReadableToBytes (const string &str)
{
	uint32 res;

	if(str.empty())
		return 0;

	// not a number
	if(str[0]<'0' || str[0]>'9')
		return 0;

	res = atoi (str.c_str());

	if(str[str.size()-1] == 'b')
	{
		if (str.size()<3)
			return res;

		// there s no break and it's **normal**
		switch (str[str.size()-2])
		{
		case 'g': res *= 1024;
		case 'm': res *= 1024;
		case 'k': res *= 1024;
		default: ;
		}
	}

	return res;
}


NLMISC_CATEGORISED_COMMAND(nel,btohr, "Convert a bytes number into an human readable number", "<int>")
{
	if (args.size() != 1)
		return false;
	
	log.displayNL("%s -> %s", args[0].c_str(), bytesToHumanReadable(args[0]).c_str());

	return true;
}


NLMISC_CATEGORISED_COMMAND(nel,hrtob, "Convert a human readable number into a bytes number", "<hr>")
{
	if (args.size() != 1)
		return false;
	
	log.displayNL("%s -> %u", args[0].c_str(), humanReadableToBytes(args[0]));
	
	return true;
}


string secondsToHumanReadable (uint32 time)
{
	static char *divTable[] = { "s", "mn", "h", "d" };
	static uint  divCoef[]  = { 60, 60, 24 };
	uint div = 0;
	uint32 res = time;
	uint32 newres = res;
	while (true)
	{
		if(div > 2)
			break;

		newres /= divCoef[div];
		
		if(newres < 3)
			break;

		div++;
		res = newres;
	}
	return toString ("%u%s", res, divTable[div]);
}

uint32 fromHumanReadable (const std::string &str)
{
	if (str.size() == 0)
		return 0;

	uint32 val = atoi (str.c_str());

	switch (str[str.size()-1])
	{
	case 's': return val;			// second
	case 'n': return val*60;		// minutes (mn)
	case 'h': return val*60*60;		// hour
	case 'd': return val*60*60*24;	// day
	case 'b':	// bytes
		switch (str[str.size()-2])
		{
		case 'k': return val*1024;
		case 'm': return val*1024*1024;
		case 'g': return val*1024*1024*1024;
		default : return val;
		}
	default: return val;
	}
	return 0;
}


NLMISC_CATEGORISED_COMMAND(nel,stohr, "Convert a second number into an human readable time", "<int>")
{
	if (args.size() != 1)
		return false;
	
	log.displayNL("%s -> %s", args[0].c_str(), secondsToHumanReadable(atoi(args[0].c_str())).c_str());
	
	return true;
}


std::string	toLower(const std::string &str)
{
	string res;
	res.reserve(str.size());
	for(uint i = 0; i < str.size(); i++)
	{
		if( (str[i] >= 'A') && (str[i] <= 'Z') )
			res += str[i] - 'A' + 'a';
		else
			res += str[i];
	}
	return res;
}

void		toLower(char *str)
{
	if (str == 0)
		return;

	while(*str != '\0')
	{
		if( (*str >= 'A') && (*str <= 'Z') )
		{
			*str = *str - 'A' + 'a';
		}
		str++;
	}
}

std::string	toUpper(const std::string &str)
{
	string res;
	res.reserve(str.size());
	for(uint i = 0; i < str.size(); i++)
	{
		if( (str[i] >= 'a') && (str[i] <= 'z') )
			res += str[i] - 'a' + 'A';
		else
			res += str[i];
	}
	return res;
}

void		toUpper(char *str)
{
	if (str == 0)
		return;

	while(*str != '\0')
	{
		if( (*str >= 'a') && (*str <= 'z') )
		{
			*str = *str - 'a' + 'A';
		}
		str++;
	}
}


//
// Exceptions
//

Exception::Exception() : _Reason("Unknown Exception")
{
//	nlinfo("Exception will be launched: %s", _Reason.c_str());
}

Exception::Exception(const std::string &reason) : _Reason(reason)
{
	nlinfo("Exception will be launched: %s", _Reason.c_str());
}

Exception::Exception(const char *format, ...)
{
	NLMISC_CONVERT_VARGS (_Reason, format, NLMISC::MaxCStringSize);
	nlinfo("Exception will be launched: %s", _Reason.c_str());
}

const char	*Exception::what() const throw()
{
	return _Reason.c_str();
}

bool killProgram(uint32 pid)
{
#ifdef NL_OS_UNIX
	int res = kill(pid, SIGKILL);
	if(res == -1)
	{
		char *err = strerror (errno);
		nlwarning("Failed to kill '%d' err %d: '%s'", pid, errno, err);
	}
	return res == 0;
/*#elif defined(NL_OS_WINDOWS)
	// it doesn't work because pid != handle and i don't know how to kill a pid or know the real handle of another service (not -1)
	int res = TerminateProcess((HANDLE)pid, 888);
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
	nlwarning("Failed to kill '%d' err %d: '%s'", pid, GetLastError (), lpMsgBuf);
	LocalFree(lpMsgBuf);
	return res != 0;
*/
#else
	nlwarning("kill not implemented on this OS");
	return false;
#endif
}

bool launchProgram (const std::string &programName, const std::string &arguments)
{

#ifdef NL_OS_WINDOWS
	STARTUPINFO         si;
    PROCESS_INFORMATION pi;
	
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
	
    si.cb = sizeof(si);
	
/*	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof (sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = FALSE;

	STARTUPINFO si;
	si.cb = sizeof (si);
	si.lpReserved = NULL;
	si.lpDesktop = NULL;
	si.lpTitle = NULL;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.cbReserved2 = 0;
	si.wShowWindow = SW_MINIMIZE;
	si.lpReserved2 = NULL;

	PROCESS_INFORMATION pi;
*/

	// Enable nlassert/nlstop to display the error reason & callstack
	const char *SE_TRANSLATOR_IN_MAIN_MODULE = "NEL_SE_TRANS";
	TCHAR envBuf [2];
	if ( GetEnvironmentVariable( SE_TRANSLATOR_IN_MAIN_MODULE, envBuf, 2 ) != 0)
	{
		SetEnvironmentVariable( SE_TRANSLATOR_IN_MAIN_MODULE, NULL );
	}
	
	string arg = " " + arguments;
	BOOL res = CreateProcess(programName.c_str(), (char*)arg.c_str(), 0, 0, FALSE, CREATE_DEFAULT_ERROR_MODE | CREATE_NO_WINDOW, 0, 0, &si, &pi);

	if (res)
	{
		nldebug("LAUNCH: Successful launch '%s' with arg '%s'", programName.c_str(), arguments.c_str());
		return true;
	}
	else
	{
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
		nlwarning("LAUNCH: Failed launched '%s' with arg '%s' err %d: '%s'", programName.c_str(), arguments.c_str(), GetLastError (), lpMsgBuf);
		LocalFree(lpMsgBuf);
	}

#elif defined(NL_OS_UNIX)

	static bool firstLaunchProgram = true;
	if (firstLaunchProgram)
	{
		// The aim of this is to avoid defunct process.
		//
		// From "man signal":
		//------
		// According to POSIX (3.3.1.3) it is unspecified what happens when SIGCHLD is set to SIG_IGN.   Here
		// the  BSD  and  SYSV  behaviours  differ,  causing BSD software that sets the action for SIGCHLD to
		// SIG_IGN to fail on Linux.
		//------
		//
		// But it works fine on my GNU/Linux so I do this because it's easier :) and I don't know exactly
		// what to do to be portable.
		signal(SIGCHLD,SIG_IGN);
		
		firstLaunchProgram = false;
	}

	int status = vfork ();
	if (status == -1)
	{
		char *err = strerror (errno);
		nlwarning("LAUNCH: Failed launched '%s' with arg '%s' err %d: '%s'", programName.c_str(), arguments.c_str(), errno, err);
	}
	else if (status == 0)
    {
		// convert one arg into several args
		vector<string> args;
		char *argv[15];
		
		uint32 pos1 = 0, pos2 = 0;
		
		do
		{
			pos1 = arguments.find_first_not_of (" ", pos2);
			if (pos1 == string::npos) break;
			pos2 = arguments.find_first_of (" ", pos1);
			args.push_back (arguments.substr (pos1, pos2-pos1));
		}
		while (pos2 != string::npos);
		
		nlassert (args.size() < 15);
		
		uint i = 0;
		argv[i] = (char *)programName.c_str();
		for (; i < args.size(); i++)
		{
			argv[i+1] = (char *) args[i].c_str();
		}
		argv[i+1] = NULL;

		status = execvp(programName.c_str(), argv);
		if (status == -1)
		{
			perror("Failed launched");
			_exit(EXIT_FAILURE);
		}
	}
	else
	{
		nldebug("LAUNCH: Successful launch '%s' with arg '%s'", programName.c_str(), arguments.c_str());
		return true;
	}
#else
	nlwarning ("LAUNCH: launchProgram() not implemented");
#endif

	return false;

}

void explode (const std::string &src, const std::string &sep, std::vector<std::string> &res, bool skipEmpty)
{
	string::size_type oldpos = 0, pos;

	res.clear ();

	do
	{
		pos = src.find (sep, oldpos);
		string s;
		if(pos == string::npos)
			s = src.substr (oldpos);
		else
			s = src.substr (oldpos, (pos-oldpos));

		if (!skipEmpty || !s.empty())
			res.push_back (s);

			oldpos = pos+sep.size();
	}
	while(pos != string::npos);

	// debug
/*	nlinfo ("Exploded '%s', with '%s', %d res", src.c_str(), sep.c_str(), res.size());
	for (uint i = 0; i < res.size(); i++)
	{
		nlinfo (" > '%s'", res[i].c_str());
	}
*/
}


/*
 * Display the bits (with 0 and 1) composing a byte (from right to left)
 */
void displayByteBits( uint8 b, uint nbits, sint beginpos, bool displayBegin, NLMISC::CLog *log )
{
	string s1, s2;
	sint i;
	for ( i=nbits-1; i!=-1; --i )
	{
		s1 += ( (b >> i) & 1 ) ? '1' : '0';
	}
	log->displayRawNL( "%s", s1.c_str() );
	if ( displayBegin )
	{
		for ( i=nbits; i>beginpos+1; --i )
		{
			s2 += " ";
		}
		s2 += "^";
		log->displayRawNL( "%s beginpos=%u", s2.c_str(), beginpos );
	}
}


//#define displayDwordBits(a,b,c)

/*
 * Display the bits (with 0 and 1) composing a number (uint32) (from right to left)
 */
void displayDwordBits( uint32 b, uint nbits, sint beginpos, bool displayBegin, NLMISC::CLog *log )
{
	string s1, s2;
	sint i;
	for ( i=nbits-1; i!=-1; --i )
	{
		s1 += ( (b >> i) & 1 ) ? '1' : '0';
	}
	log->displayRawNL( "%s", s1.c_str() );
	if ( displayBegin )
	{
		for ( i=nbits; i>beginpos+1; --i )
		{
			s2 += " ";
		}
		s2 += "^";
		log->displayRawNL( "%s beginpos=%u", s2.c_str(), beginpos );
	}
}


int	nlfseek64( FILE *stream, sint64 offset, int origin )
{
#ifdef NL_OS_WINDOWS
	
	//
	fpos_t pos64 = 0;
	switch (origin)
	{
	case SEEK_CUR:
		if (fgetpos(stream, &pos64) != 0)
			return -1;
	case SEEK_END:
		pos64 = _filelengthi64(_fileno(stream));
		if (pos64 == -1L)
			return -1;
	};
	
	// Seek
	pos64 += offset;
	
	// Set the final position
	return fsetpos (stream, &pos64);
	
#else // NL_OS_WINDOWS
	
	// This code doesn't work under windows : fseek() implementation uses a signed 32 bits offset. What ever we do, it can't seek more than 2 Go.
	// For the moment, i don't know if it works under linux for seek of more than 2 Go.
	
	nlassert ((offset < SINT64_CONSTANT(2147483647)) && (offset > SINT64_CONSTANT(-2147483648)));
	
	bool first = true;
	do
	{
		// Get the size of the next fseek
		sint nextSeek;
		if (offset > 0)
			nextSeek = (sint)std::min (SINT64_CONSTANT(2147483647), offset);
		else
			nextSeek = (sint)std::max (-SINT64_CONSTANT(2147483648), offset);
		
		// Make a seek
		int result = fseek ( stream, nextSeek, first?origin:SEEK_CUR );
		if (result != 0)
			return result;
		
		// Remaining
		offset -= nextSeek;
		first = false;
	}
	while (offset);
	
	return 0;
	
#endif // NL_OS_WINDOWS
}



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/// Commands
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

NLMISC_CATEGORISED_COMMAND(nel, sleep, "Freeze the service for N seconds (for debug purpose)", "<N>")
{
	if(args.size() != 1) return false;
	
	sint32 n = atoi (args[0].c_str());
	
	log.displayNL ("Sleeping during %d seconds", n);
	
	nlSleep(n * 1000);	
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, system, "Execute the command line using system() function call (wait until the end of the command)", "<commandline>")
{
	if(args.size() != 1) return false;
	
	string cmd = args[0];
	log.displayNL ("Executing '%s'", cmd.c_str());
	system(cmd.c_str());
	log.displayNL ("End of Execution of '%s'", cmd.c_str());
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, launchProgram, "Execute the command line using launcProgram() function call (launch in background task without waiting the end of the execution)", "<programName> <arguments>")
{
	if(args.size() != 2) return false;
	
	string cmd = args[0];
	string arg = args[1];
	log.displayNL ("Executing '%s' with argument '%s'", cmd.c_str(), arg.c_str());
	launchProgram(cmd, arg);
	log.displayNL ("End of Execution of '%s' with argument '%s'", cmd.c_str(), arg.c_str());
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, killProgram, "kill a program given the pid", "<pid>")
{
	if(args.size() != 1) return false;
	killProgram(atoi(args[0].c_str()));
	return true;
}

#ifdef NL_OS_WINDOWS
LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
{
    HKEY hkey;
    LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

    if (retval == ERROR_SUCCESS) 
	{
        long datasize = MAX_PATH;
        TCHAR data[MAX_PATH];
        RegQueryValue(hkey, NULL, data, &datasize);
        lstrcpy(retdata,data);
        RegCloseKey(hkey);
    }

    return retval;
}
#endif // NL_OS_WINDOWS

bool openURL (const char *url)
{
#ifdef NL_OS_WINDOWS
    TCHAR key[MAX_PATH + MAX_PATH];
    if (GetRegKey(HKEY_CLASSES_ROOT, ".html", key) == ERROR_SUCCESS) 
	{
        lstrcat(key, "\\shell\\open\\command");

        if (GetRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS) 
		{
            TCHAR *pos;
            pos = strstr(key, "\"%1\"");
            if (pos == NULL) {                     // No quotes found
                pos = strstr(key, "%1");       // Check for %1, without quotes 
                if (pos == NULL)                   // No parameter at all...
                    pos = key+lstrlen(key)-1;
                else
                    *pos = '\0';                   // Remove the parameter
            }
            else
                *pos = '\0';                       // Remove the parameter

            lstrcat(pos, " ");
            lstrcat(pos, url);
            int res = WinExec(key,SW_SHOWDEFAULT);
			return (res>31);
        }
    }
#endif // NL_OS_WINDOWS
	return false;
}

bool openDoc (const char *document)
{
#ifdef NL_OS_WINDOWS
	string ext = CFile::getExtension (document);
    TCHAR key[MAX_PATH + MAX_PATH];

    // First try ShellExecute()
    HINSTANCE result = ShellExecute(NULL, "open", document, NULL,NULL, SW_SHOWDEFAULT);

    // If it failed, get the .htm regkey and lookup the program
    if ((UINT)result <= HINSTANCE_ERROR) 
	{
        if (GetRegKey(HKEY_CLASSES_ROOT, ext.c_str(), key) == ERROR_SUCCESS) 
		{
            lstrcat(key, "\\shell\\open\\command");

            if (GetRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS) 
			{
                TCHAR *pos;
                pos = strstr(key, "\"%1\"");
                if (pos == NULL) {                     // No quotes found
                    pos = strstr(key, "%1");       // Check for %1, without quotes 
                    if (pos == NULL)                   // No parameter at all...
                        pos = key+lstrlen(key)-1;
                    else
                        *pos = '\0';                   // Remove the parameter
                }
                else
                    *pos = '\0';                       // Remove the parameter

                lstrcat(pos, " ");
                lstrcat(pos, document);
                int res = WinExec(key,SW_SHOWDEFAULT);
				return (res>31);
            }
        }
    }
	else
		return true;
#endif // NL_OS_WINDOWS
	return false;
}

std::string trim (const std::string &str)
{
	uint start = 0;
	const uint size = str.size();
	while (start < size && str[start] <= 32)
		start++;
	uint end = size;
	while (end > start && str[end-1] <= 32)
		end--;
	return str.substr (start, end-start);
}

} // NLMISC
