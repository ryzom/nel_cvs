/** \file system_info.cpp
 * <File description>
 *
 * $Id: system_info.cpp,v 1.10 2002/11/13 17:08:11 coutelas Exp $
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

#include "stdmisc.h"

#ifdef NL_OS_WINDOWS
#include <windows.h>
#include <tchar.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include "nel/misc/system_info.h"
#include "nel/misc/command.h"
#include "nel/misc/heap_allocator.h"

using namespace std;

namespace NLMISC {

string CSystemInfo::getOS ()
{
	string OSString = "Unknown";
#ifdef NL_OS_WINDOWS
	char ver[1024];

	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure,
	// which is supported on Windows 2000.
	//
	// If that fails, try using the OSVERSIONINFO structure.

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
			return "Windows Unknown";
	}

	switch (osvi.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_NT:
		// Test for the product.

		if ( osvi.dwMajorVersion <= 4 )
			OSString = "Microsoft Windows NT ";

		if ( osvi.dwMajorVersion == 5 )
			OSString = "Microsoft Windows 2000 ";

		// Test for workstation versus server.
/* can't access to product type
		if( bOsVersionInfoEx )
		{
			if ( osvi.wProductType == VER_NT_WORKSTATION )
			OSString += "Professional ";

			if ( osvi.wProductType == VER_NT_SERVER )
			OSString += "Server ";
		}
		else
*/		{
			HKEY hKey;
			char szProductType[80];
			DWORD dwBufLen;

			RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\ProductOptions", 0, KEY_QUERY_VALUE, &hKey );
			RegQueryValueEx( hKey, "ProductType", NULL, NULL, (LPBYTE) szProductType, &dwBufLen);
			RegCloseKey( hKey );
			if ( lstrcmpi( "WINNT", szProductType) == 0 )
				OSString += "Workstation ";
			if ( lstrcmpi( "SERVERNT", szProductType) == 0 )
				OSString += "Server ";
		}

		// Display version, service pack (if any), and build number.
		smprintf(ver, 1024, "version %d.%d '%s' (Build %d)", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
		OSString += ver;
		break;

	case VER_PLATFORM_WIN32_WINDOWS:

		if(osvi.dwMinorVersion == 0)
			OSString = "Microsoft Windows 95 ";
		else if (osvi.dwMinorVersion == 10)
			OSString = "Microsoft Windows 98 ";
		else if (osvi.dwMinorVersion == 90)
			OSString = "Microsoft Windows Millenium ";
/* microsoft way
		if ((osvi.dwMajorVersion > 4) || (osvi.dwMajorVersion == 4) && (osvi.dwMinorVersion > 0)))
			OSString += "Microsoft Windows 98 ";
		else
			OSString += "Microsoft Windows 95 ";
*/
		// Display version, service pack (if any), and build number.
		smprintf(ver, 1024, "version %d.%d %s (Build %d)", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
		OSString += ver;
		break;

	case VER_PLATFORM_WIN32s:
		OSString = "Microsoft Win32s";
		break;
	}

#elif defined NL_OS_UNIX

	OSString = "Unix";

#endif	// NL_OS_UNIX

	return OSString;
}

string CSystemInfo::getProc ()
{
	string ProcString = "Unknown";

#ifdef NL_OS_WINDOWS

	LONG result;
	char value[1024];
	DWORD valueSize;
	HKEY hKey;

	result = ::RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Hardware\\Description\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hKey);
	if (result == ERROR_SUCCESS)
	{
		// get processor name
		valueSize = 1024;
		result = ::RegQueryValueEx (hKey, _T("ProcessorNameString"), NULL, NULL, (LPBYTE)&value, &valueSize);
		if (result == ERROR_SUCCESS)
			ProcString = value;
		else
			ProcString = "UnknownProc";

		ProcString += " / ";

		// get processor identifier
		valueSize = 1024;
		result = ::RegQueryValueEx (hKey, _T("Identifier"), NULL, NULL, (LPBYTE)&value, &valueSize);
		if (result == ERROR_SUCCESS)
			ProcString += value;
		else
			ProcString += "UnknownIdentifier";

		ProcString += " / ";

		// get processor vendor
		valueSize = 1024;
		result = ::RegQueryValueEx (hKey, _T("VendorIdentifier"), NULL, NULL, (LPBYTE)&value, &valueSize);
		if (result == ERROR_SUCCESS)
			ProcString += value;
		else
			ProcString += "UnknownVendor";

		ProcString += " / ";
		
		// get processor frequence
		result = ::RegQueryValueEx (hKey, _T("~MHz"), NULL, NULL, (LPBYTE)&value, &valueSize);
		if (result == ERROR_SUCCESS)
		{
			ProcString += itoa (*(int *)value, value, 10);
			ProcString += "MHz";
		}
		else
			ProcString += "UnknownFreq";
	}

	// Make sure to close the reg key

	RegCloseKey (hKey);

#elif defined NL_OS_UNIX


#endif

	return ProcString;
}

string CSystemInfo::getMem ()
{
	string MemString = "Unknown";

#ifdef NL_OS_WINDOWS

	MEMORYSTATUS ms;

	GlobalMemoryStatus (&ms);

	sint extt = 0, extf = 0;
	char *ext2str[] = { "b", "Kb", "Mb", "Gb", "Tb" };

	while (ms.dwTotalPhys > 1024)
	{
		ms.dwTotalPhys /= 1024;
		extt++;
	}
	
	while (ms.dwAvailPhys > 1024)
	{
		ms.dwAvailPhys /= 1024;
		extf++;
	}

	char mem[1024];
	smprintf (mem, 1024, "physical memory: total: %d %s free: %d %s", ms.dwTotalPhys+1, ext2str[extt], ms.dwAvailPhys+1, ext2str[extf]);
	MemString = mem;

#elif defined NL_OS_UNIX


#endif

	return MemString;
}


static bool DetectMMX()
{		
	#ifdef NL_OS_WINDOWS		
		if (!CSystemInfo::hasCPUID()) return false; // cpuid not supported ...

		uint32 result = 0;
		__asm
		{
			 mov  eax,1
			 cpuid
			 test edx,0x800000  // bit 23 = MMX instruction set
			 je   noMMX
			 mov result, 1	
			noMMX:
		}

		return result == 1;
 
		// printf("mmx detected\n");

	#else
		return false;
	#endif
}


static bool DetectSSE()
{	
	#ifdef NL_OS_WINDOWS
		if (!CSystemInfo::hasCPUID()) return false; // cpuid not supported ...

		uint32 result = 0;
		__asm
		{			
			mov eax, 1   // request for feature flags
			cpuid 							
			test EDX, 002000000h   // bit 25 in feature flags equal to 1
			je noSSE
			mov result, 1  // sse detected
		noSSE:
		}


		if (result)
		{
			// check OS support for SSE
			try 
			{
				__asm
				{
					xorps xmm0, xmm0  // Streaming SIMD Extension
				}
			}
			catch(...)
			{
				return false;
			}
		
			// printf("sse detected\n");

			return true;
		}
		else
		{
			return false;
		}
	#else
		return false;
	#endif
}

static bool HaveMMX = DetectMMX ();
static bool HaveSSE = DetectSSE ();

bool CSystemInfo::hasCPUID ()
{
	#ifdef NL_OS_WINDOWS
		 uint32 result;
		 __asm
		 {
			 pushad
			 pushfd
			 //	 If ID bit of EFLAGS can change, then cpuid is available
			 pushfd
			 pop  eax					// Get EFLAG
			 mov  ecx,eax
			 xor  eax,0x200000			// Flip ID bit
			 push eax
			 popfd						// Write EFLAGS
			 pushfd      
			 pop  eax					// read back EFLAG
			 xor  eax,ecx
			 je   noCpuid				// no flip -> no CPUID instr.
			 
			 popfd						// restore state
			 popad
			 mov  result, 1
			 jmp  CPUIDPresent
		
			noCpuid:
			 popfd					    // restore state
			 popad
			 mov result, 0
			CPUIDPresent:
		 }
		 return result == 1;
	#else
		 return false;
	#endif
}
bool CSystemInfo::hasMMX () { return HaveMMX; }
bool CSystemInfo::hasSSE () { return HaveSSE; }


bool CSystemInfo::isNT()
{
#ifdef NL_OS_WINDOWS
	OSVERSIONINFO ver;
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&ver);
	return ver.dwPlatformId == VER_PLATFORM_WIN32_NT;
#else
	return false;
#endif
}


#ifdef NL_OS_UNIX

static inline char *skipWS(const char *p)
{
    while (isspace(*p)) p++;
    return (char *)p;
}

static inline char *skipToken(const char *p)
{
    while (isspace(*p)) p++;
    while (*p && !isspace(*p)) p++;
    return (char *)p;
}

// col: 0=total used free shared buffers cached
uint32 getSystemMemory (uint col)
{
	if (col > 5)
		return 0;

    char buffer[4096+1];
    int fd, len;
    char *p;
	
    /* get system wide memory usage */
    {
		char *p;
		
		fd = open("/proc/meminfo", O_RDONLY);
		len = read(fd, buffer, sizeof(buffer)-1);
		close(fd);
		buffer[len] = '\0';
		
		/* be prepared for extra columns to appear be seeking
		to ends of lines */
		p = strchr(buffer, '\n');
		p = skipToken(p);			/* "Mem:" */
		for (uint i = 0; i < col; i++)
		{
			p = skipToken(p);
		}
		return strtoul(p, &p, 10);
	}
}

#endif // NL_OS_UNIX




uint32 CSystemInfo::availablePhysicalMemory ()
{
#ifdef NL_OS_WINDOWS

	MEMORYSTATUS ms;
	GlobalMemoryStatus (&ms);
	return ms.dwAvailPhys;

#elif defined NL_OS_UNIX

	return getSystemMemory (2);

#endif

	return 0;
}

uint32 CSystemInfo::totalPhysicalMemory ()
{
#ifdef NL_OS_WINDOWS

	MEMORYSTATUS ms;
	GlobalMemoryStatus (&ms);
	return ms.dwTotalPhys;

#elif defined NL_OS_UNIX

	return getSystemMemory (0);
	
#endif

	return 0;
}


NLMISC_DYNVARIABLE(uint32, AvailablePhysicalMemory, "Physical memory available on this computer in bytes")
{
	if (get) *pointer = CSystemInfo::availablePhysicalMemory ();
}

NLMISC_DYNVARIABLE(uint32, TotalPhysicalMemory, "Total physical memory on this computer in bytes")
{
	if (get) *pointer = CSystemInfo::totalPhysicalMemory ();
}

NLMISC_DYNVARIABLE(uint32, ProcessUsedMemory, "Memory used by this process in bytes")
{
	if (get) *pointer = CHeapAllocator::getAllocatedSystemMemory ();
}


} // NLMISC
