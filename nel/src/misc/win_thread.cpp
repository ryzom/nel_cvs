/** \file win_thread.cpp
 * class CWinThread
 *
 * $Id: win_thread.cpp,v 1.7 2002/02/27 10:45:47 corvazier Exp $
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

#include "nel/misc/win_thread.h"
#include <windows.h>


namespace NLMISC {

// the IThread static creator
IThread *IThread::create (IRunnable *runnable)
{
	return new CWinThread (runnable);
}

static unsigned long __stdcall ProxyFunc (void *arg)
{
	CWinThread *parent = (CWinThread *) arg;
	parent->Runnable->run();

	return 0;
}



CWinThread::CWinThread (IRunnable *runnable)
{
	this->Runnable = runnable;
	ThreadHandle = NULL;
}

CWinThread::~CWinThread ()
{
	if (ThreadHandle != NULL) terminate();
}

void CWinThread::start ()
{
	ThreadHandle = (void *) CreateThread (NULL, 0, ProxyFunc, this, 0, (DWORD *)&ThreadId);
	if (ThreadHandle == NULL)
	{
		throw EThread ( "Cannot create new thread" );
	}
}

void CWinThread::terminate ()
{
	BOOL i = TerminateThread((HANDLE)ThreadHandle, 0);
	if(!i) 
	{
		DWORD e = GetLastError();		
	}
	i = CloseHandle((HANDLE)ThreadHandle);
	ThreadHandle = NULL;
}

void CWinThread::wait ()
{
	if (ThreadHandle == NULL) return;

	WaitForSingleObject(ThreadHandle, INFINITE);
	CloseHandle(ThreadHandle);
	ThreadHandle = NULL;
}

uint64 CWinThread::getProcessCPUMask()
{
	// Ask the system for number of processor available for this process
	DWORD processAffinityMask;
	DWORD systemAffinityMask;
	if (GetProcessAffinityMask(GetCurrentProcess(), &processAffinityMask, &systemAffinityMask))
	{
		// Return the CPU mask
		return (uint64)processAffinityMask;
	}
	else
		return 1;
}

bool CWinThread::setCPUMask(uint64 cpuMask)
{
	// Thread must exist
	if (ThreadHandle == NULL)
		return false;

	// Ask the system for number of processor available for this process
	return SetThreadAffinityMask ((HANDLE)ThreadHandle, (DWORD)cpuMask) != 0;
}


} // NLMISC

#endif // NL_OS_WINDOWS
