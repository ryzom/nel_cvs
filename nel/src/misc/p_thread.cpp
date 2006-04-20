/** \file p_thread.cpp
 * class CPThread (POSIX threads)
 *
 * $Id: p_thread.cpp,v 1.15.4.1.2.1 2006/04/20 15:36:36 boucher Exp $
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

#ifdef NL_OS_UNIX

#include "nel/misc/p_thread.h"


namespace NLMISC {


/*
 * The IThread static creator
 */
IThread *IThread::create( IRunnable *runnable )
{
	return new CPThread( runnable );
}


CPThread CurrentThread(0);

/*
 * Get the current thread
 */
IThread *IThread::getCurrentThread ()
{
	/// \todo: implement this functionality for POSIX thread
	return &CurrentThread;
} 

/*
 * Thread beginning
 */
static void *ProxyFunc( void *arg )
{
	CPThread *parent = (CPThread*)arg;

	// Allow to terminate the thread without cancellation point
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);

	// Run the code of the thread
	parent->Runnable->run();

	// Allow some clean
//	pthread_exit(0);
}



/*
 * Constructor
 */
CPThread::CPThread(IRunnable *runnable) : _State(0), Runnable(runnable)
{}


/*
 * Destructor
 */
CPThread::~CPThread()
{
	if(_State == 1)
		terminate(); // force the end of the thread if not already ended
	
	if(_State > 0)
		pthread_detach(_ThreadHandle); // free allocated resources only if it was created
}

/*
 * start
 */
void CPThread::start()
{
	if(pthread_create(&_ThreadHandle, 0, ProxyFunc, this) != 0)
	{
		throw EThread("Cannot start new thread");
	}
	_State = 1;
}

bool CPThread::isRunning()
{
	// TODO : need a real implementation here that check thread status
	return _State == 1;
}

/*
 * terminate
 */
void CPThread::terminate()
{
	if(_State == 1)
	{
		// cancel only if started
		pthread_cancel(_ThreadHandle);
		_State = 2;	// set to finished
	}
}

/*
 * wait
 */
void CPThread::wait ()
{
	if(_State == 1)
	{
		if(pthread_join(_ThreadHandle, 0) != 0)
		{
			throw EThread( "Cannot join with thread" );
		}
		_State = 2;	// set to finished
	}
}

/*
 * setCPUMask
 */
bool CPThread::setCPUMask(uint64 cpuMask)
{
	/// \todo: handle processor selection under POSIX thread
	return true;
}

/*
 * getCPUMask
 */
uint64 CPThread::getCPUMask()
{
	/// \todo: handle processor selection under POSIX thread
	return 1;
}

/*
 * getUserName
 */
std::string CPThread::getUserName()
{
	/// \todo: return the thread user name
	return "Not implemented";
}


// **** Process

// The current process
CPProcess CurrentProcess;

// Get the current process
IProcess *IProcess::getCurrentProcess ()
{
	return &CurrentProcess;
}

/*
 * getCPUMask
 */
uint64 CPProcess::getCPUMask()
{
	/// \todo: handle processor selection under POSIX thread
	return 1;
}

/// set the CPU mask
bool CPProcess::setCPUMask(uint64 mask)
{
	/// \todo: handle processor selection under POSIX thread
	return 1;
}


} // NLMISC

#else // NL_OS_UNIX

// remove stupid VC6 warnings
void foo_p_thread_cpp() {}

#endif // NL_OS_UNIX
