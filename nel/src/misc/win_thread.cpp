/** \file win_thread.cpp
 * class CWinThread
 *
 * $Id: win_thread.cpp,v 1.10 2002/04/17 08:46:32 besson Exp $
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

CWinThread MainThread ((void*)GetCurrentThread (), GetCurrentThreadId());
DWORD TLSThreadPointer = 0xFFFFFFFF;

// the IThread static creator
IThread *IThread::create (IRunnable *runnable)
{
	return new CWinThread (runnable);
}

IThread *IThread::getCurrentThread ()
{
	// TLS alloc must have been done	
	nlassert (TLSThreadPointer != 0xffffffff);

	// Get the thread pointer
	IThread *thread = (IThread*)TlsGetValue (TLSThreadPointer);

	// Return current thread
	return thread;
}
 
static unsigned long __stdcall ProxyFunc (void *arg)
{
	CWinThread *parent = (CWinThread *) arg;

	// TLS alloc must have been done	
	nlassert (TLSThreadPointer != 0xffffffff);

	// Set the thread pointer in TLS memory
	nlverify (TlsSetValue (TLSThreadPointer, (void*)parent));

	// Run the thread
	parent->Runnable->run();
	
	return 0;
}

CWinThread::CWinThread (IRunnable *runnable)
{
	this->Runnable = runnable;
	ThreadHandle = NULL;
	_MainThread = false;
}

CWinThread::CWinThread (void* threadHandle, uint32 threadId)
{
	// Main thread
	_MainThread = true;
	this->Runnable = NULL;
	ThreadHandle = threadHandle;
	ThreadId = threadId;

	// TLS alloc must have been done	
	TLSThreadPointer = TlsAlloc ();
	nlassert (TLSThreadPointer!=0xffffffff);
 
	// Set the thread pointer in TLS memory
	nlverify (TlsSetValue (TLSThreadPointer, (void*)this));
}

CWinThread::~CWinThread ()
{
	// If not the main thread
	if (_MainThread)
	{
		// Free TLS memory
		nlassert (TLSThreadPointer!=0xffffffff);
		TlsFree (TLSThreadPointer);
	}
	else
	{
		if (ThreadHandle != NULL) terminate();
	}
}

void CWinThread::start ()
{
	ThreadHandle = (void *) CreateThread (NULL, 0, ProxyFunc, this, 0, (DWORD *)&ThreadId);
	SetThreadPriorityBoost (ThreadHandle, TRUE); // FALSE == Enable Priority Boost
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

bool CWinThread::setCPUMask(uint64 cpuMask)
{
	// Thread must exist
	if (ThreadHandle == NULL)
		return false;

	// Ask the system for number of processor available for this process
	return SetThreadAffinityMask ((HANDLE)ThreadHandle, (DWORD)cpuMask) != 0;
}

uint64 CWinThread::getCPUMask()
{
	// Thread must exist
	if (ThreadHandle == NULL)
		return 1;

	// Get the current process mask
	uint64 mask=IProcess::getCurrentProcess ()->getCPUMask ();

	// Get thread affinity mask
	DWORD old = SetThreadAffinityMask ((HANDLE)ThreadHandle, (DWORD)mask);
	nlassert (old != 0);
	if (old == 0)
		return 1;

	// Reset it
	SetThreadAffinityMask ((HANDLE)ThreadHandle, (DWORD)old);

	// Return the mask
	return old;
}

// **** Process

// The current process
CWinProcess CurrentProcess ((void*)GetCurrentProcess());

// Get the current process
IProcess *IProcess::getCurrentProcess ()
{
	return &CurrentProcess;
}

CWinProcess::CWinProcess (void *handle)
{
	// Get the current process handle
	_ProcessHandle = handle;
}

uint64 CWinProcess::getCPUMask()
{
	// Ask the system for number of processor available for this process
	DWORD processAffinityMask;
	DWORD systemAffinityMask;
	if (GetProcessAffinityMask((HANDLE)_ProcessHandle, &processAffinityMask, &systemAffinityMask))
	{
		// Return the CPU mask
		return (uint64)processAffinityMask;
	}
	else
		return 1;
}


} // NLMISC

#endif // NL_OS_WINDOWS
