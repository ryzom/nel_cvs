/** \file co_task.cpp
 * Coroutine based task.
 *
 * $Id: co_task.cpp,v 1.3.4.4 2006/01/11 15:02:10 boucher Exp $
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

namespace NLMISC
{
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


	// platform specific data
	struct TCoTaskData
	{
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
		/// A Thread depentend storage to hold fiber pointer.
		CTDS	_ThreadMainFiber;
#endif

		CCurrentCoTask()
		{}

	public:
		/// Set the current task for the calling thread
		void setCurrentTask(CCoTask *task)
		{
			_CurrentTaskTDS.setPointer(task);
		}

		/// retrieve the current task for the calling thread
		CCoTask *getCurrentTask()
		{
			return reinterpret_cast<CCoTask*>(_CurrentTaskTDS.getPointer());
		}
#if defined (NL_OS_WINDOWS)
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
		// allocate platform specific data storage
		_PImpl = new TCoTaskData;
#if defined (NL_OS_WINDOWS)
		_PImpl->_Fiber = NULL;
		_PImpl->_ParentFiber = NULL;
#elif defined(NL_OS_UNIX)
		// allocate the stack
		_PImpl->_Stack = new uint8[stackSize];
#endif
	}

	CCoTask::~CCoTask()
	{
		_TerminationRequested = true;

		if (_Started)
		{
			while (!_Finished)
				resume();
		}

#if defined (NL_OS_WINDOWS)
		DeleteFiber(_PImpl->_Fiber);
#elif defined(NL_OS_UNIX)
		// free the stack
		delete [] _PImpl->_Stack;
#endif

		// free platform specific storage
		delete _PImpl;
	}

	void CCoTask::start()
	{
		nlassert(!_Started);

		_Started = true;

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
	}

	void CCoTask::yield()
	{
		nlassert(_Started);
		nlassert(CCurrentCoTask::getInstance().getCurrentTask() == this);
		CCurrentCoTask::getInstance().setCurrentTask(NULL);
#if defined (NL_OS_WINDOWS)
		SwitchToFiber(_PImpl->_ParentFiber);
#elif defined (NL_OS_UNIX)
		// swap to the parent context
		nlverify(swapcontext(&_PImpl->_Ctx, &_PImpl->_ParentCtx) == 0);
#endif
	}

	void CCoTask::resume()
	{
		nlassert(CCurrentCoTask::getInstance().getCurrentTask() != this);
		if (!_Started)
			start();
		else if (!_Finished)
		{
			nlassert(_Started);
			CCurrentCoTask::getInstance().setCurrentTask(this);

#if defined (NL_OS_WINDOWS)
			SwitchToFiber(_PImpl->_Fiber);
#elif defined (NL_OS_UNIX)
			// swap to the parent context
			nlverify(swapcontext(&_PImpl->_ParentCtx, &_PImpl->_Ctx) == 0);
#endif
		}
	}

	/// wait until the task terminate
	void CCoTask::wait()
	{
		// resume the task until termination
		while (!_Finished)
			resume();
	}

} // namespace NLMISC

