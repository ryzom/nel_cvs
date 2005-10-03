/** \file co_task.cpp
 * Coroutine based task.
 *
 * $Id: co_task.cpp,v 1.1 2005/10/03 10:08:28 boucher Exp $
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
# include <windows.h>
#elif defined (NL_OS_UNIX)
# define NL_WIN_CALLBACK 
# include <ucontext.h>
#else
#error "Coroutine task are not supported yet by you platform, do it ?"
#endif


	// platform specific data
	struct TCoTaskData
	{
#if defined (NL_OS_WINDOWS)
		LPVOID	_Fiber;
		LPVOID	_ParentFiber;
#elif defined (NL_OS_UNIX)
		uint8		*_Stack;
		ucontext_t	_Ctx;
		ucontext_t	_ParentCtx;
#endif

		// task bootstrap function
		// NB : this function is in this structure because of the
		// NL_WIN_CALLBACK symbol that need <windows.h> to be defined, so
		// to remove it, I moved the function here.
		static void NL_WIN_CALLBACK startFunc(void* param)
		{
			CCoTask *task = reinterpret_cast<CCoTask*>(param);

			// run the task
			task->run();

			task->_Finished = true;

			// nothing more to do
			for (;;)
				// return to parent task
				task->yield();

			nlassert(false);
		}

	};

	class CCurrentCoTask
	{
		NLMISC_SAFE_SINGLETON_DECL(CCurrentCoTask);

		CTDS	_CurrentTaskTDS;

		CCurrentCoTask()
		{}

	public:
		void setCurrentTask(CCoTask *task)
		{
			_CurrentTaskTDS.setPointer(task);
		}

		CCoTask *getCurrentTask()
		{
			return reinterpret_cast<CCoTask*>(_CurrentTaskTDS.getPointer());
		}
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
		static			initialized = false;
		static LPVOID	mainFiber = NULL;
		if (!initialized)
		{
			mainFiber = ConvertThreadToFiber(NULL);
			initialized = true;
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
		if (!_Started)
			start();
		else
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

