/** \file p_thread.cpp
 * <File description>
 *
 * $Id: p_thread.cpp,v 1.2 2001/02/13 18:25:48 corvazier Exp $
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


#include "nel/misc/types_nl.h"

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


/*
 * Thread beginning
 */
static void ProxyFunc( void *arg )
{
	CPThread *parent = (CPThread*)arg;

	// Allow to terminate the thread without cancellation point
	pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, NULL );

	// Run the code of the thread
	parent->Runnable->run();
}



/*
 * Constructor
 */
CPThread::CPThread( IRunnable *runnable ) : Runnable( runnable ), ThreadHandle( -1 )
{}


/*
 * Destructor
 */
CPThread::~CPThread()
{
	pthread_detach( &ThreadHandle ); // free allocated resources after termination
	if ( ThreadHandle != -1 )
	{
		terminate();
	}
}


/*
 * start
 */
void CPThread::start()
{
	if ( pthread_create( &ThreadHandle, NULL, ProxyFunc, this ) != 0 )
	{
		throw EThread( "Cannot start new thread" );
	}
}


/*
 * terminate
 */
void CPThread::terminate()
{
	pthread_cancel( ThreadHandle );
	ThreadHandle = -1;
}


/*
 * wait
 */
void CPThread::wait ()
{
	if (ThreadHandle != -1)
	{
		if ( pthread_join( ThreadHandle ) != 0 )
		{
			throw EThread( "Cannot join with thread" );
		}
		ThreadHandle = -1;
	}
}


} // NLMISC

#endif // NL_OS_UNIX
