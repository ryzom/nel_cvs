/** \file co_task.h
 * Coroutine based task.
 *
 * $Id: co_task.h,v 1.1 2005/10/03 10:08:04 boucher Exp $
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

#ifndef NL_CO_TASK_H
#define NL_CO_TASK_H

#include "types_nl.h"
#include <vector>

namespace NLMISC
{
#if defined (NL_OS_WINDOWS)
# define NL_WIN_CALLBACK CALLBACK
#elif defined (NL_OS_UNIX)
# define NL_WIN_CALLBACK 
#endif
	
	// Default to 8KB stack for tasks
	const unsigned int	NL_TASK_STACK_SIZE = 8*1024;

	// forward def for platform specific data
	struct TCoTaskData;

	/** The coroutine task class encapsulate the coroutines detail and provide
	 *	an easy to use simple master/slave coroutine model.
	 *	The concept is that the main thread is the 'master' (or parent) coroutine
	 *	and that the task is run by a slave coroutine.
	 *	Therefore, you can 'start' the task, 'yield' the focus from the task to it's
	 *	parent, 'resume' the task from the parent, check for termination of the task,
	 *	and/or wait for it.
	 *
	 *	If you don't know about coroutines, a short description folow :
	 *		* Coroutines are some sort of multithreading
	 *		* Coroutines are not preemptive, it's the application code that choose 
	 *			task swaping point
	 *		* thus, coroutines don't need heavy synchronisation (like mutexes)
	 *		* coroutines are said to be lighter than thread during context switch
	 *		* coroutines don't replace preemptives threads, they have their own application domain
	 *
	 *
	 *	Please note that this class is realy simple (compared to what can be done with coroutines)
	 *	but match the need for a very simple mean to have two task running side by side with
	 *	predefined sync point.
	 *	You can build the same think using thread and mutex, but it will be a lot more complex
	 *	to build and debug.
	 *
	 *  A simple sample :
	 *	CMyTask : public CCoTask
	 *	{
	 *	public:
	 *		void run()
	 *		{
	 *			for (uint i=0; i<7; ++i)
	 *			{
	 *				printf("CoTask : %i\n", i)
	 *				// leave control to parent task
	 *				yield();
	 *			}
	 *		}
	 *	};
	 *
	 *	unt main()
	 *	{
	 *		CMyTask task;
	 *		// start the task, block until task terminate or call 'yield'
	 *		task.resume();
	 *
	 *		for (uint i=0; i<5; ++i)
	 *		{
	 *			printf("Main : %i\n", i);
	 *			// let the tak run a bit
	 *			task.resume();
	 *		}
	 *
	 *		// wait fot task completion
	 *		task.wait();
	 *	}
	 *
	 *	This little proggy will output the following :
	 *	*********** Output *************
	 *	CoTask : 1
	 *	Main : 1
	 *	CoTask : 2
	 *	Main : 2
	 *	CoTask : 3
	 *	Main : 3
	 *	CoTask : 4
	 *	Main : 4
	 *	CoTask : 5
	 *	CoTask : 6
	 *	*********** End of output ********
	 *
	 *
	 */
	class CCoTask
	{
		/// The task is started or not
		bool	_Started;
		/// The task should terminate as soon as possible
		bool	_TerminationRequested;
		/// The task is finished (run() have returned)
		bool	_Finished;

		/// Pointer on internal platform specific data
		TCoTaskData	*_PImpl;

		friend struct TCoTaskData;

		/// start the task (must not be started before)
		void start();
	public:

		/** Get the current task object. 
		 *	Return NULL if the current thread context is not in a task
		 */
		static CCoTask *getCurrentTask();

		/// Constructor with stack size for the task
		CCoTask(uint stackSize = NL_TASK_STACK_SIZE);
		/** Destructor. If the task is running, wait until the
		 *	task terminate.
		 */
		virtual ~CCoTask();

		/// to call from the parent task, start or resume task execution
		void resume();

		/// to call from the task, yield execution focus to parent task
		void yield();

		/// check if task started
		bool isStarted()
		{
			return _Started;
		}
		/// check for task completion
		bool isFinished()
		{
			return _Finished;
		}

		/// parent task ask for task ending (run function should check this to terminate)
		void requestTerminate()
		{
			_TerminationRequested = true;
		}

		/** check if termination request have been called (mainly used by task user code
		 *	to check for termination the task on request).
		 */
		bool isTerminationRequested()
		{
			return _TerminationRequested;
		}

		/** Called by parent task, wait until the task terminate. Note obviously that this call can lead to an
		 *	infinite wait if the task function is not willing to terminate itself.
		 */
		void wait();

		virtual void run() =0;
	};
	

} // namespace NLMISC

#endif // NL_CO_TASK_H
