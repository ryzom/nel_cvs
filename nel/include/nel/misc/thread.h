/** \file misc/thread.h
 * Base OS independant class interface for the thread management
 *
 * $Id: thread.h,v 1.19.8.1 2004/09/28 10:21:15 berenguier Exp $
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

#ifndef NL_THREAD_H
#define NL_THREAD_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"


namespace NLMISC {


/**
 * Thread callback interface.
 * When a thread is created, it will call run() in its attached IRunnable interface.
 *
 *\code

	#include "nel/misc/thread.h"

	class HelloLoopThread : public IRunnable
	{
		void run ()
		{
			while(true)	printf("Hello World\n");
		}

	};

	IThread *thread = IThread::create (new HelloLoopThread);
	thread->start ();

 *\endcode
 *
 *
 *
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class IRunnable
{
public:
	// Called when a thread is run.
	virtual void run()=0;
	virtual ~IRunnable()
	{
	}
	// Return the runnable name
	virtual void getName (std::string &result) const
	{
		result = "NoName";
	}
};

/**
 * Thread base interface, must be implemented for all OS
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class IThread
{
public:

	/** 
	  * Create a new thread.
	  * Implemented in the derived class.
	  */
	static IThread *create(IRunnable *runnable);

	/** 
	  * Return a pointer on the current thread.
	  * Implemented in the derived class.
	  * Not implemented under Linux.
	  */
	static IThread *getCurrentThread ();

	virtual ~IThread () { }
	
	// Starts the thread.
	virtual void start()=0;

	// Terminate the thread. (use with caution under win98)
	virtual void terminate()=0;

	// In the calling program, wait until the specified thread has exited (use with caution under win98)
	virtual void wait()=0;

	/// Return a pointer to the runnable object
	virtual IRunnable *getRunnable()=0;

	/**
	  * Set the CPU mask of this thread. Thread must have been started before.
	  * The mask must be a subset of the CPU mask returned by IProcess::getCPUMask() thread process.
	  * Not implemented under Linux.
	  */
	virtual bool setCPUMask(uint64 cpuMask)=0;

	/**
	  * Get the CPU mask of this thread. Thread must have been started before.
	  * The mask should be a subset of the CPU mask returned by IProcess::getCPUMask() thread process.
	  * Not implemented under Linux.
	  */
	virtual uint64 getCPUMask()=0;

	/**
	  * Get the thread user name.
	  * Notimplemented under linux, under windows return the name of the logon user.
	  */
	virtual std::string getUserName()=0;
};


/*
 * Thread exception
 */
struct EThread : public Exception
{
	EThread (const char* message) : Exception (message) {};
};


/**
 * Process base interface, must be implemented for all OS
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2000
 */
class IProcess
{
public:

	/** 
	  * Return a pointer on the current process.
	  * Implemented in the derived class.
	  */
	static IProcess *getCurrentProcess ();

	/**
	  * Return process CPU mask. Each bit stand for a CPU usable by the process threads.
	  * Not implemented under Linux.
	  */
	virtual uint64 getCPUMask()=0;

	/**
	  * Set the process CPU mask. Each bit stand for a CPU usable by the process threads.
	  * Not implemented under Linux.
	  */
	virtual bool setCPUMask(uint64 mask)=0;
};


} // NLMISC


#endif // NL_THREAD_H

/* End of thread.h */
