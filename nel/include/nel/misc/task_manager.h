/** \file task_manager.h
 * Manage a list of task in a separate thread
 *
 * $Id: task_manager.h,v 1.11 2002/10/10 12:42:11 berenguier Exp $
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

#ifndef NL_TASK_MANAGER_H
#define NL_TASK_MANAGER_H

#include "nel/misc/types_nl.h"

#include <list>

#include "nel/misc/mutex.h"
#include "nel/misc/thread.h"

namespace NLMISC {


/**
 * CTaskManager is a class that manage a list of Task with one Thread
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2000
 */
class CTaskManager : public IRunnable
{
public:

	/// Constructor
	CTaskManager();

	/// Destructeur
	~CTaskManager();

	/// Manage TaskQueue
	void run(void);

	/// Add a task to TaskManager
	void addTask(IRunnable *);

	/// Delete a task, only if task is not running, return true if found and deleted
	bool deleteTask(IRunnable *r);

	/// Sleep a Task
	void sleepTask(void) { nlSleep(10); }

	/// Task list size
	uint taskListSize(void); 

	/// return false if exit() is required. task added with addTask() should test this flag.
	bool	isThreadRunning() {return _ThreadRunning;}


protected:

	/** If any, wait the current running task to complete
	 *	this function MUST be called in a 'accessor to the _TaskQueue' statement because a mutex is required
	 *	eg:
	 *	\begincode
	 *	{
	 *		CSynchronized<list<IRunnable *> >::CAccessor acces(&_TaskQueue);
	 *		waitCurrentTaskToComplete();
	 *	}
	 *	\endcode
	 */
	void	waitCurrentTaskToComplete ();

protected:

	/// queue of tasks, using list container instead of queue for DeleteTask methode
	CSynchronized<std::list<IRunnable *> > _TaskQueue;

	/// thread pointer
	IThread *_Thread;

	/// flag indicate thread loop, if false cause thread exit
	volatile	bool _ThreadRunning;

private:

	volatile	bool _IsTaskRunning;

};


} // NLMISC


#endif // NL_TASK_MANAGER_H

/* End of task_manager.h */
