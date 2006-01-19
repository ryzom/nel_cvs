/** \file co_task.cpp
 * Coroutine based task.
 *
 * $Id: co_task.cpp,v 1.3.4.5 2006/01/19 13:39:31 boucher Exp $
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
#include "nel/misc/co_task.h"
#include "nel/misc/tds.h"
// Flag to use thread instead of coroutine primitives (i.e windows fibers or gcc context)
#define NL_USE_THREAD_COTASK
// flag to activate debug message
//#define NL_GEN_DEBUG_MSG

#ifdef NL_GEN_DEBUG_MSG
#define NL_CT_DEBUG nldebug
#else
#define NL_CT_DEBUG if(0)nldebug
#endif

#if defined(NL_USE_THREAD_COTASK)
	#pragma message(NL_LOC_MSG "Using threaded coroutine")
	# include "nel/misc/thread.h"
#else //NL_USE_THREAD_COTASK
// some platform specifics
#if defined (NL_OS_WINDOWS)
# define NL_WIN_CALLBACK CALLBACK
// Visual .NET won't allow Fibers for a Windows version older than 2000. However the basic features are sufficient for us, we want to compile them for all Windows >= 95
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0400)
# ifdef _WIN32_WINNT
#  undef _WIN32_WINNT
# endif
# define _WIN32_WINNT 0x0400
#endif

# include <windows.h>
#elif defined (NL_OS_UNIX)
# define NL_WIN_CALLBACK 
# include <ucontext.h>
#else
# error "Coroutine task are not supported yet by your platform, do it ?"
#endif
#endif //NL_USE_THREAD_COTASK

namespace NLMISC
{

	// platform specific data
#if  defined(NL_USE_THREAD_COTASK)
	struct TCoTaskData : public IRunnable
#else //NL_USE_THREAD_COTASK
	struct TCoTaskData
#endif //NL_USE_THREAD_COTASK
	{
#if  defined(NL_USE_THREAD_COTASK)
		/// The thread id for the co task
//		TThreadId	*_TaskThreadId;
		/// The parent thread id
//		TThreadId	*_ParentThreadId;
		/// The mutex of the task task
		CMutex		_TaskMutex;

		CCoTask		*_CoTask;

		bool		_Running;


		TCoTaskData(CCoTask *task)
			: _CoTask(task),
			_Running(false)
		{
		}

		void run();

#else //NL_USE_THREAD_COTASK
#if defined (NL_OS_WINDOWS)
		/// The fiber pointer for the task fiber
		LPVOID	_Fiber;
		/// The fiber pointer of the main (or master, or parent, as you want)
		LPVOID	_ParentFiber;
#elif defined (NL_OS_UNIX)
		/// The coroutine stack pointer (allocated memory)
		uint8		*_Stack;
		/// The task context
		ucontext_t	_Ctx;
		/// The main (or master or parent, as you want) task context
		ucontext_t	_ParentCtx;
#endif
#endif //NL_USE_THREAD_COTASK
		
#if !defined(NL_USE_THREAD_COTASK)
		/** task bootstrap function
		 *	NB : this function is in this structure because of the
		 *	NL_WIN_CALLBACK symbol that need <windows.h> to be defined, so
		 *	to remove it from the header, I moved the function here
		 *	(otherwise, it should be declared in the CCoTask class as
		 *	a private member)
		 */
		static void NL_WIN_CALLBACK startFunc(void* param)
		{
			CCoTask *task = reinterpret_cast<CCoTask*>(param);

			NL_CT_DEBUG("CoTask : task %p start func called", task);

			try
			{
				// run the task
				task->run();
			}
			catch(...)
			{
				nlwarning("CCoTask::startFunc : the task has generated an unhandled exeption and will terminate");
			}

			task->_Finished = true;

			// nothing more to do
			for (;;)
				// return to parent task
				task->yield();

			nlassert(false);
		}
#endif //NL_USE_THREAD_COTASK
	};

	/** Management of current task in a thread.
	 *	This class is used to store and retrieve the current
	 *	CCoTask pointer in the current thread.
	 *	It is build upon the SAFE_SINGLETON paradigm, making it
	 *	safe to use with NeL DLL.
	 *	For windows platform, this singleton also hold the
	 *	fiber pointer of the current thread. This is needed because
	 *	of the bad design the the fiber API before Windows XP.
	 */
	class CCurrentCoTask
	{
		NLMISC_SAFE_SINGLETON_DECL(CCurrentCoTask);

		/// A thread dependent storage to hold by thread coroutine info
		CTDS	_CurrentTaskTDS;

#if defined (NL_OS_WINDOWS)
		/// A Thread dependent storage to hold fiber pointer.
		CTDS	_ThreadMainFiber;
#endif

		CCurrentCoTask()
		{}

	public:
		/// Set the current task for the calling thread
		void setCurrentTask(CCoTask *task)
		{
			NL_CT_DEBUG("CoTask : setting current co task to %p", task);
			_CurrentTaskTDS.setPointer(task);
		}

		/// retrieve the current task for the calling thread
		CCoTask *getCurrentTask()
		{
			return reinterpret_cast<CCoTask*>(_CurrentTaskTDS.getPointer());
		}
#if defined (NL_OS_WINDOWS) && !defined(NL_USE_THREAD_COTASK)
		void setMainFiber(LPVOID fiber)
		{
			_ThreadMainFiber.setPointer(fiber);
		}

		/** Return the main fiber for the calling thread. Return NULL if 
		 *	the thread has not been converted to fiber.
		 */
		LPVOID getMainFiber()
		{
			return _ThreadMainFiber.getPointer();
		}
#endif
	};

	NLMISC_SAFE_SINGLETON_IMPL(CCurrentCoTask);

	CCoTask *CCoTask::getCurrentTask()
	{
		return CCurrentCoTask::getInstance().getCurrentTask();
	}


	CCoTask::CCoTask(uint stackSize)
		: _Started(false),
		_TerminationRequested(false),
		_Finished(false)
	{
		NL_CT_DEBUG("CoTask : creating task %p", this);
#if defined(NL_USE_THREAD_COTASK)
		// allocate platform specific data storage
		_PImpl = new TCoTaskData(this);
//		_PImpl->_TaskThreadId = 0;
//		_PImpl->_ParentThreadId = 0;
#else //NL_USE_THREAD_COTASK
		// allocate platform specific data storage
		_PImpl = new TCoTaskData;
#if defined (NL_OS_WINDOWS)
		_PImpl->_Fiber = NULL;
		_PImpl->_ParentFiber = NULL;
#elif defined(NL_OS_UNIX)
		// allocate the stack
		_PImpl->_Stack = new uint8[stackSize];
#endif
#endif //NL_USE_THREAD_COTASK
	}

	CCoTask::~CCoTask()
	{
		NL_CT_DEBUG("CoTask : deleting task %p", this);
		_TerminationRequested = true;

		if (_Started)
		{
			while (!_Finished)
				resume();
		}

#if defined(NL_USE_THREAD_COTASK)

#else //NL_USE_THREAD_COTASK 
#if defined (NL_OS_WINDOWS)
		DeleteFiber(_PImpl->_Fiber);
#elif defined(NL_OS_UNIX)
		// free the stack
		delete [] _PImpl->_Stack;
#endif
#endif //NL_USE_THREAD_COTASK 

		// free platform specific storage
		delete _PImpl;
	}

	void CCoTask::start()
	{
		NL_CT_DEBUG("CoTask : Starting task %p", this);
		nlassert(!_Started);

		_Started = true;

#if defined(NL_USE_THREAD_COTASK)

		// create the thread
		IThread *taskThread = IThread::create(_PImpl);
		// start the thread
		taskThread->start();

		// wait until the task is affectively started
		while (!_PImpl->_Running)
			nlSleep(0);

		// get the mutex (this is locking)
		_PImpl->_TaskMutex.enter();

		// clear the running flag
		_PImpl->_Running = false;

		// in the treaded mode, there is no need to call resume() inside start()

#else //NL_USE_THREAD_COTASK
#if defined (NL_OS_WINDOWS)

		LPVOID mainFiber = CCurrentCoTask::getInstance().getMainFiber();

		if (mainFiber == NULL)
		{
			// we need to convert this thread to a fiber
			mainFiber = ConvertThreadToFiber(NULL);

			CCurrentCoTask::getInstance().setMainFiber(mainFiber);
		}

		_PImpl->_ParentFiber = mainFiber;
		_PImpl->_Fiber = CreateFiber(NL_TASK_STACK_SIZE, TCoTaskData::startFunc, this);
		nlassert(_PImpl->_Fiber != NULL);
#elif defined (NL_OS_UNIX)
		// store the parent ctx
		nlverify(getcontext(&_PImpl->_ParentCtx) == 0);
		// build the task context
		nlverify(getcontext(&_PImpl->_Ctx) == 0);

		// change the task context
 		_PImpl->_Ctx.uc_stack.ss_sp = _PImpl->_Stack;
		_PImpl->_Ctx.uc_stack.ss_size = NL_TASK_STACK_SIZE;

		_PImpl->_Ctx.uc_link = NULL;
		_PImpl->_Ctx.uc_stack.ss_flags = 0;

		makecontext(&_PImpl->_Ctx, reinterpret_cast<void (*)()>(TCoTaskData::startFunc), 1, this);
#endif
		resume();
#endif //NL_USE_THREAD_COTASK
	}

	void CCoTask::yield()
	{
		NL_CT_DEBUG("CoTask : task %p yield", this);
		nlassert(_Started);
		nlassert(CCurrentCoTask::getInstance().getCurrentTask() == this);
#if defined(NL_USE_THREAD_COTASK)
		// Release the thread mutex
		_PImpl->_TaskMutex.leave();
		// Wait until the main thread as taken control
		while (_PImpl->_Running)
			nlSleep(0);
		// And get back the mutex for waiting for next resume (this should lock)
		_PImpl->_TaskMutex.enter();
		// Set the task as running
		_PImpl->_Running = true;
#else //NL_USE_THREAD_COTASK 
		CCurrentCoTask::getInstance().setCurrentTask(NULL);
#if defined (NL_OS_WINDOWS)
		SwitchToFiber(_PImpl->_ParentFiber);
#elif defined (NL_OS_UNIX)
		// swap to the parent context
		nlverify(swapcontext(&_PImpl->_Ctx, &_PImpl->_ParentCtx) == 0);
#endif
#endif //NL_USE_THREAD_COTASK 
	}

	void CCoTask::resume()
	{
		NL_CT_DEBUG("CoTask : resuming task %p", this);
		nlassert(CCurrentCoTask::getInstance().getCurrentTask() != this);
		if (!_Started)
			start();
		else if (!_Finished)
		{
			nlassert(_Started);

#if defined(NL_USE_THREAD_COTASK)
			// Release the mutex
			_PImpl->_TaskMutex.leave();
			// Wait until the task as effectively resumed
			while (!_PImpl->_Running)
			{
				nlSleep(0);
			}
			// Get back the mutex
			_PImpl->_TaskMutex.enter();
			// clear the running flag
			_PImpl->_Running = false;

#else // NL_USE_THREAD_COTASK
			CCurrentCoTask::getInstance().setCurrentTask(this);
#if defined (NL_OS_WINDOWS)
			SwitchToFiber(_PImpl->_Fiber);
#elif defined (NL_OS_UNIX)
			// swap to the parent context
			nlverify(swapcontext(&_PImpl->_ParentCtx, &_PImpl->_Ctx) == 0);
#endif
#endif //NL_USE_THREAD_COTASK
		}
	}

	/// wait until the task terminate
	void CCoTask::wait()
	{
		NL_CT_DEBUG("CoTask : waiting for task %p to terminate", this);
		// resume the task until termination
		while (!_Finished)
			resume();
	}


	void TCoTaskData::run()
	{
		// set the current task
		CCurrentCoTask::getInstance().setCurrentTask(_CoTask);
		// Set the task as running
		_Running = true;
		// Acquire the task mutex
		_TaskMutex.enter();
		// run the task
		_CoTask->run();

		_CoTask->_Finished = true;

		// Release the parent mutex
		_TaskMutex.leave();

		// nothing more to do, just return to terminate the thread
	}

} // namespace NLMISC

