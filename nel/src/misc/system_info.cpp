/** \file system_info.cpp
 * <File description>
 *
 * $Id: system_info.cpp,v 1.24 2004/05/03 09:25:42 corvazier Exp $
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
#	include <windows.h>
#	include <tchar.h>
#else
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <fcntl.h>
#	include <unistd.h>
#	include <errno.h>
#endif

#include "nel/misc/system_info.h"
#include "nel/misc/command.h"
#include "nel/misc/heap_allocator.h"
#include "nel/misc/variable.h"

using namespace std;

namespace NLMISC {


string CSystemInfo::getOS()
{
	string OSString = "Unknown";

#ifdef NL_OS_WINDOWS

	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;
	const int BUFSIZE = 80;

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	// If that fails, try using the OSVERSIONINFO structure.

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
		return OSString+" Can't GetVersionEx()";
	}

	switch (osvi.dwPlatformId)
	{
	// Test for the Windows NT product family.
	case VER_PLATFORM_WIN32_NT:

		// Test for the specific product family.
		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
			OSString = "Microsoft Windows Server 2003 family ";

		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
			OSString = "Microsoft Windows XP ";

		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
			OSString = "Microsoft Windows 2000 ";

		if ( osvi.dwMajorVersion <= 4 )
			OSString = "Microsoft Windows NT ";

         // Test for specific product on Windows NT 4.0 SP6 and later.
         if( bOsVersionInfoEx )
         {
			 // not available on visual 6 SP4, then comment it

/*            // Test for the workstation type.
            if ( osvi.wProductType == VER_NT_WORKSTATION )
            {
               if( osvi.dwMajorVersion == 4 )
                  printf ( "Workstation 4.0 " );
               else if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
                  printf ( "Home Edition " );
               else
                  printf ( "Professional " );
            }
            
            // Test for the server type.
            else if ( osvi.wProductType == VER_NT_SERVER )
            {
               if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
               {
                  if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                     printf ( "Datacenter Edition " );
                  else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                     printf ( "Enterprise Edition " );
                  else if ( osvi.wSuiteMask == VER_SUITE_BLADE )
                     printf ( "Web Edition " );
                  else
                     printf ( "Standard Edition " );
               }

               else if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
               {
                  if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                     printf ( "Datacenter Server " );
                  else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                     printf ( "Advanced Server " );
                  else
                     printf ( "Server " );
               }

               else  // Windows NT 4.0 
               {
                  if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                     printf ("Server 4.0, Enterprise Edition " );
                  else
                     printf ( "Server 4.0 " );
               }
            }*/
		}
		else  // Test for specific product on Windows NT 4.0 SP5 and earlier
		{
			HKEY hKey;
			char szProductType[BUFSIZE];
			DWORD dwBufLen=BUFSIZE;
			LONG lRet;

			lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\ProductOptions", 0, KEY_QUERY_VALUE, &hKey );
			if( lRet != ERROR_SUCCESS )
				return OSString + " Can't RegOpenKeyEx";

			lRet = RegQueryValueEx( hKey, "ProductType", NULL, NULL, (LPBYTE) szProductType, &dwBufLen);
			if( (lRet != ERROR_SUCCESS) || (dwBufLen > BUFSIZE) )
				return OSString + " Can't ReQueryValueEx";

			RegCloseKey( hKey );

			if ( lstrcmpi( "WINNT", szProductType) == 0 )
				OSString += "Workstation ";
			if ( lstrcmpi( "LANMANNT", szProductType) == 0 )
				OSString += "Server ";
			if ( lstrcmpi( "SERVERNT", szProductType) == 0 )
				OSString += "Advanced Server ";
         }

		// Display service pack (if any) and build number.

		if( osvi.dwMajorVersion == 4 && lstrcmpi( osvi.szCSDVersion, "Service Pack 6" ) == 0 )
		{
			HKEY hKey;
			LONG lRet;

			// Test for SP6 versus SP6a.
			lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009", 0, KEY_QUERY_VALUE, &hKey );
			if( lRet == ERROR_SUCCESS )
				OSString += toString("Service Pack 6a (Build %d) ", osvi.dwBuildNumber & 0xFFFF );
			else // Windows NT 4.0 prior to SP6a
			{
				OSString += toString("%s (Build %d) ", osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
			}

			RegCloseKey( hKey );
		}
		else // Windows NT 3.51 and earlier or Windows 2000 and later
		{
			OSString += toString("%s (Build %d) ", osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
		}

		break;

		// Test for the Windows 95 product family.
		case VER_PLATFORM_WIN32_WINDOWS:

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
			{
				OSString = "Microsoft Windows 95 ";
				if ( osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B' )
					OSString += "OSR2 ";
			} 

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
			{
				OSString = "Microsoft Windows 98 ";
				if ( osvi.szCSDVersion[1] == 'A' )
					OSString += "SE ";
			} 

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
			{
				OSString = "Microsoft Windows Millennium Edition ";
			} 
		break;

		case VER_PLATFORM_WIN32s:

			OSString = "Microsoft Win32s ";
		break;
	}

	OSString += toString( "(%d.%d %d)", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber & 0xFFFF);

#elif defined NL_OS_UNIX
	
	int fd = open("/proc/version", O_RDONLY);
	if (fd == -1)
	{
		nlwarning ("SI: Can't get OS from /proc/version: %s", strerror (errno));
	}
	else
	{
		char buffer[4096+1];
		int len = read(fd, buffer, sizeof(buffer)-1);
		close(fd);
		
		// remove the \n and set \0
		buffer[len-1] = '\0';
		
		OSString = buffer;
	}
	
#endif	// NL_OS_UNIX
	
	return OSString;
}


#if 0 // old getOS() function

string /*CSystemInfo::*/_oldgetOS()
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

		if (osvi.dwMajorVersion <= 4)
			OSString = "Microsoft Windows NT ";
		else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
			OSString = "Microsoft Windows 2000 ";
		else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
		{
			OSString = "Microsoft Windows XP ";
		}
		else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
			OSString = "Microsoft Windows Server 2003 family ";
		
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

		if(osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
		{
			OSString = "Microsoft Windows 95 ";
			if(osvi.szCSDVersion[0] == 'B' || osvi.szCSDVersion[0] == 'C')
				OSString += "OSR2 ";
		}
		else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
		{
			OSString = "Microsoft Windows 98 ";
			if(osvi.szCSDVersion[0] == 'A')
				OSString += "SE ";
		}
		else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
			OSString = "Microsoft Windows Me ";
		else
			OSString = "Microsoft Unknown dwMajorVersion="+toString((int)osvi.dwMajorVersion)+" dwMinorVersion="+toString((int)osvi.dwMinorVersion);


		// Display version, service pack (if any), and build number.
		smprintf(ver, 1024, "version %d.%d %s (Build %d)", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
		OSString += ver;
		break;

	case VER_PLATFORM_WIN32s:
		OSString = "Microsoft Win32s";
		break;
	}

#elif defined NL_OS_UNIX

	int fd = open("/proc/version", O_RDONLY);
	if (fd == -1)
	{
		nlwarning ("SI: Can't get OS from /proc/version: %s", strerror (errno));
	}
	else
	{
		char buffer[4096+1];
		int len = read(fd, buffer, sizeof(buffer)-1);
		close(fd);
		
		// remove the \n and set \0
		buffer[len-1] = '\0';
		
		OSString = buffer;
	}
	
#endif	// NL_OS_UNIX

	return OSString;
}
#endif // old getOS() function

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

uint64 CSystemInfo::getProcessorFrequency(bool quick)
{
	static uint64 freq = 0;
#ifdef	NL_CPU_INTEL
	static bool freqComputed = false;	
	if (freqComputed) return freq;
	
	if (!quick)
	{
		TTicks bestNumTicks   = 0;
		uint64 bestNumCycles;
		uint64 numCycles;
		const uint numSamples = 5;
		const uint numLoops   = 50000000;
		
		volatile uint k; // prevent optimisation for the loop
		for(uint l = 0; l < numSamples; ++l)
		{	
			TTicks startTick = NLMISC::CTime::getPerformanceTime();
			uint64 startCycle = rdtsc();
			volatile uint dummy = 0;
			for(k = 0; k < numLoops; ++k)
			{		
				++ dummy;
			}		
			numCycles = rdtsc() - startCycle;
			TTicks numTicks = NLMISC::CTime::getPerformanceTime() - startTick;
			if (numTicks > bestNumTicks)
			{		
				bestNumTicks  = numTicks;
				bestNumCycles = numCycles;
			}
		}
		freq = (uint64) ((double) bestNumCycles * 1 / CTime::ticksToSecond(bestNumTicks));
	}
	else
	{
		TTicks timeBefore = NLMISC::CTime::getPerformanceTime();
		uint64 tickBefore = rdtsc();
		nlSleep (100);
		TTicks timeAfter = NLMISC::CTime::getPerformanceTime();
		TTicks tickAfter = rdtsc();
		
		double timeDelta = CTime::ticksToSecond(timeAfter - timeBefore);
		TTicks tickDelta = tickAfter - tickBefore;
		
		freq = (uint64) ((double)tickDelta / timeDelta);
	}
	
	nlinfo ("SI: CSystemInfo: Processor frequency is %.0f MHz", (float)freq/1000000.0);
	freqComputed = true;
#endif // NL_CPU_INTEL
	return freq;
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

bool CSystemInfo::_HaveMMX = DetectMMX ();
bool CSystemInfo::_HaveSSE = DetectSSE ();

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
	
	int fd = open("/proc/meminfo", O_RDONLY);
	if (fd == -1)
	{
		nlwarning ("SI: Can't get OS from /proc/meminfo: %s", strerror (errno));
		return 0;
	}
	else
	{
		char buffer[4096+1];
		int len = read(fd, buffer, sizeof(buffer)-1);
		close(fd);
		buffer[len] = '\0';
			
		/* be prepared for extra columns to appear be seeking to ends of lines */
		char *p = strchr(buffer, '\n');
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

	return getSystemMemory (2) + getSystemMemory (4) + getSystemMemory (5);

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


NLMISC_DYNVARIABLE(string, AvailablePhysicalMemory, "Physical memory available on this computer in bytes")
{
	if (get) *pointer = bytesToHumanReadable(CSystemInfo::availablePhysicalMemory ());
}

NLMISC_DYNVARIABLE(string, TotalPhysicalMemory, "Total physical memory on this computer in bytes")
{
	if (get) *pointer = bytesToHumanReadable(CSystemInfo::totalPhysicalMemory ());
}

NLMISC_DYNVARIABLE(string, ProcessUsedMemory, "Memory used by this process in bytes")
{
	if (get) *pointer = bytesToHumanReadable(CHeapAllocator::getAllocatedSystemMemory ());
}

NLMISC_DYNVARIABLE(string, OS, "OS used")
{
	if (get) *pointer = CSystemInfo::getOS();
}

#ifdef NL_OS_WINDOWS
struct DISPLAY_DEVICE_EX
{
    DWORD  cb;
    CHAR  DeviceName[32];
    CHAR  DeviceString[128];
    DWORD  StateFlags;
    CHAR  DeviceID[128];
    CHAR  DeviceKey[128];
};
#endif // NL_OS_WINDOWS

bool CSystemInfo::getVideoInfo (std::string &deviceName, uint64 &driverVersion)
{
#ifdef NL_OS_WINDOWS
	/* Get the device name with EnumDisplayDevices (doesn't work under win95).
	 * Look for driver information for this device in the registry
	 *
	 * Follow the recommandations in the news group comp.os.ms-windows.programmer.nt.kernel-mode : "Get Video Driver ... Need Version"
	 */

	bool debug = false;
#ifdef _DEBUG 
	debug = true;
#endif _DEBUG 

	HMODULE hm = GetModuleHandle(TEXT("USER32"));
	if (hm)
	{
		BOOL (WINAPI* EnumDisplayDevices)(LPCTSTR lpDevice, DWORD iDevNum, PDISPLAY_DEVICE lpDisplayDevice, DWORD dwFlags) = NULL;
		*(FARPROC*)&EnumDisplayDevices = GetProcAddress(hm, "EnumDisplayDevicesA");
		if (EnumDisplayDevices)
		{
			DISPLAY_DEVICE_EX DisplayDevice;
			uint device = 0;
			DisplayDevice.cb = sizeof (DISPLAY_DEVICE_EX);
			bool found = false;
			while (EnumDisplayDevices(NULL, device, (DISPLAY_DEVICE*)&DisplayDevice, 0))
			{
				// Main board ?
				if ((DisplayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) &&
					(DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) &&
					((DisplayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) == 0) &&
					(DisplayDevice.DeviceKey[0] != 0))
				{
					found = true;

					// The device name
					deviceName = DisplayDevice.DeviceString;

					string keyPath = DisplayDevice.DeviceKey;
					string keyName;

					// Get the window version
					OSVERSIONINFO ver;
					ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
					GetVersionEx(&ver);
					bool atleastNT4 = (ver.dwMajorVersion  > 3) && (ver.dwPlatformId == VER_PLATFORM_WIN32_NT);
					bool winXP = ((ver.dwMajorVersion == 5) && (ver.dwMinorVersion == 1)) || (ver.dwMajorVersion > 5);

					// * Get the registry entry for driver

					// OSversion >= osWin2k
					if (atleastNT4)
					{
						if (winXP)
						{
							int pos = keyPath.rfind ('\\');
							if (pos != string::npos)
								keyPath = keyPath.substr (0, pos+1);
							keyPath += "Video";
							keyName = "Service";
						}
						else
						{
							int pos = strlwr (keyPath).find ("\\device");
							if (pos != string::npos)
								keyPath = keyPath.substr (0, pos+1);
							keyName = "ImagePath";
						}
					}
					else // Win 9x
					{
						keyPath += "\\default";
						keyName = "drv";
					}

					// Format the key path
					if (strlwr (keyPath).find ("\\registry\\machine") == 0)
					{
						keyPath = "HKEY_LOCAL_MACHINE" + keyPath.substr (strlen ("\\registry\\machine"));
					}

					// Get the root key
					static const char *rootKeys[]=
					{
						"HKEY_CLASSES_ROOT\\",
						"HKEY_CURRENT_CONFIG\\",
						"HKEY_CURRENT_USER\\",
						"HKEY_LOCAL_MACHINE\\",
						"HKEY_USERS\\",
						"HKEY_PERFORMANCE_DATA\\",
						"HKEY_DYN_DATA\\"
					};
					static const HKEY rootKeysH[]=
					{
						HKEY_CLASSES_ROOT,
						HKEY_CURRENT_CONFIG,
						HKEY_CURRENT_USER,
						HKEY_LOCAL_MACHINE,
						HKEY_USERS,
						HKEY_PERFORMANCE_DATA,
						HKEY_DYN_DATA,
					};
					uint i;
					HKEY keyRoot = HKEY_LOCAL_MACHINE;
					for (i=0; i<sizeof(rootKeysH)/sizeof(HKEY); i++)
					{
						if (strupr (keyPath).find (rootKeys[i]) == 0)
						{
							keyPath = keyPath.substr (strlen (rootKeys[i]));
							keyRoot = rootKeysH[i];
							break;
						}
					}

					// * Read the registry 
					HKEY baseKey;
					if (RegOpenKeyEx(keyRoot, keyPath.c_str(), 0, KEY_READ, &baseKey) == ERROR_SUCCESS)
					{
						DWORD valueType;
						char value[512];
						DWORD size = 512;
						if (RegQueryValueEx(baseKey, keyName.c_str(), NULL, &valueType, (unsigned char *)value, &size) == ERROR_SUCCESS)
						{
							// Null ?
							if (value[0] != 0)
							{
								bool ok = !winXP;
								if (winXP)
								{
									// In Windows'XP we got service name -> not real driver name, so
									string xpKey = string ("System\\CurrentControlSet\\Services\\")+value;
									if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, xpKey.c_str(), 0, KEY_READ, &baseKey) == ERROR_SUCCESS)
									{
										size = 512;
										if (RegQueryValueEx(baseKey, "ImagePath", NULL, &valueType, (unsigned char *)value, &size) == ERROR_SUCCESS)
										{
											if (value[0] != 0)
											{
												ok = true;
											}
											else
												nlwarning ("CSystemInfo::getVideoInfo : empty value ImagePath in key %s", xpKey.c_str());
										}
										else
											nlwarning ("CSystemInfo::getVideoInfo : can't query ImagePath in key %s", xpKey.c_str());
									}
									else
										nlwarning ("CSystemInfo::getVideoInfo : can't open key %s", xpKey.c_str());
								}

								// Version dll link
								HMODULE hmVersion = LoadLibrary ("version");
								if (hmVersion)
								{
									BOOL (WINAPI* _GetFileVersionInfo)(LPTSTR, DWORD, DWORD, LPVOID) = NULL;
									DWORD (WINAPI* _GetFileVersionInfoSize)(LPTSTR, LPDWORD) = NULL;
									BOOL (WINAPI* _VerQueryValue)(const LPVOID, LPTSTR, LPVOID*, PUINT) = NULL;
									*(FARPROC*)&_GetFileVersionInfo = GetProcAddress(hmVersion, "GetFileVersionInfoA");
									*(FARPROC*)&_GetFileVersionInfoSize = GetProcAddress(hmVersion, "GetFileVersionInfoSizeA");
									*(FARPROC*)&_VerQueryValue = GetProcAddress(hmVersion, "VerQueryValueA");
									if (_VerQueryValue && _GetFileVersionInfoSize && _GetFileVersionInfo)
									{
										// value got the path to the driver
										string driverName = value;
										nlverify (GetWindowsDirectory(value, 512));
										driverName = string (value) + "\\" + driverName;

										DWORD dwHandle;
										DWORD size = _GetFileVersionInfoSize ((char*)driverName.c_str(), &dwHandle);
										if (size)
										{
											vector<uint8> buffer;
											buffer.resize (size);
											if (_GetFileVersionInfo((char*)driverName.c_str(), dwHandle, size, &buffer[0]))
											{
												VS_FIXEDFILEINFO *info;
												UINT len;
												if (_VerQueryValue(&buffer[0], "\\", (VOID**)&info, &len))
												{
													driverVersion = (((uint64)info->dwFileVersionMS)<<32)|info->dwFileVersionLS;
													return true;
												}
												else
													nlwarning ("CSystemInfo::getVideoInfo : VerQueryValue fails (%s)", driverName.c_str());
											}
											else
												nlwarning ("CSystemInfo::getVideoInfo : GetFileVersionInfo fails (%s)", driverName.c_str());
										}
										else
											nlwarning ("CSystemInfo::getVideoInfo : GetFileVersionInfoSize == 0 (%s)", driverName.c_str());
									}
									else
										nlwarning ("CSystemInfo::getVideoInfo : No VerQuery, GetFileVersionInfoSize, GetFileVersionInfo functions");
								}
								else
									nlwarning ("CSystemInfo::getVideoInfo : No version dll");
							}
							else
								nlwarning ("CSystemInfo::getVideoInfo : empty value %s in key %s", keyName.c_str(), keyPath.c_str());
						}
						else
							nlwarning ("CSystemInfo::getVideoInfo : can't query value %s in key %s", keyName.c_str(), keyPath.c_str());
					}
					else
						nlwarning ("CSystemInfo::getVideoInfo : can't open key %s", keyPath.c_str());
				}
				device++;
			}
			if (!found)
				nlwarning ("CSystemInfo::getVideoInfo : No primary display device found");
		}
		else
			nlwarning ("CSystemInfo::getVideoInfo : No EnumDisplayDevices function");
	}
	else
		nlwarning ("CSystemInfo::getVideoInfo : No user32 dll");

#endif // NL_OS_WINDOWS

	// Fails
	return false;
}

uint32 CSystemInfo::virtualMemory ()
{
#ifdef NL_OS_WINDOWS

	MEMORYSTATUS ms;
	GlobalMemoryStatus (&ms);
	return ms.dwTotalVirtual - ms.dwAvailVirtual;

#endif

	return 0;
}

} // NLMISC
