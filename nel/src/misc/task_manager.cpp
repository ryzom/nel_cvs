/** \file task_manager.cpp
 * <File description>
 *
 * $Id: task_manager.cpp,v 1.10 2003/05/09 12:46:07 corvazier Exp $
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

#include "nel/misc/task_manager.h"

using namespace std;

#define NLMISC_DONE_TASK_SIZE 20

namespace NLMISC {

/*
 * Constructor
 */
CTaskManager::CTaskManager() : _RunningTask (""), _TaskQueue (""), _DoneTaskQueue ("")
{
	_IsTaskRunning = false;
	_ThreadRunning = true;
	CSynchronized<string>::CAccessor currentTask(&_RunningTask);
	currentTask.value () = "";
	_Thread = IThread::create(this);
	_Thread->start();
}

/*
 * Destructeur
 */
CTaskManager::~CTaskManager()
{
	_ThreadRunning = false;
	while(!_ThreadRunning)
		nlSleep(10);
}

// Manage TaskQueue
void CTaskManager::run(void)
{
	IRunnable *runnableTask;
	while(_ThreadRunning)
	{
		{
			CSynchronized<list<IRunnable *> >::CAccessor acces(&_TaskQueue);
			if(acces.value().empty())
			{
				runnableTask = NULL;
			}
			else
			{
				_IsTaskRunning = true;
				runnableTask = acces.value().front();
				acces.value().pop_front();
			}
		}
		if(runnableTask)
		{
			{
				CSynchronized<string>::CAccessor currentTask(&_RunningTask);
				string temp;
				runnableTask->getName(temp);
				currentTask.value () = temp;
			}
			string taskName;
			runnableTask->getName (taskName);
			runnableTask->run();
			{
				CSynchronized<string>::CAccessor currentTask(&_RunningTask);
				currentTask.value () = "";
			}
			{
				CSynchronized<list<string> >::CAccessor doneTask(&_DoneTaskQueue);
				doneTask.value().push_front (taskName);
				if (doneTask.value().size () > NLMISC_DONE_TASK_SIZE)
					doneTask.value().resize (NLMISC_DONE_TASK_SIZE);
			}

			_IsTaskRunning = false;
		}
		else
		{
			sleepTask();
		}
	}
	_ThreadRunning = true;
}

// Add a task to TaskManager
void CTaskManager::addTask(IRunnable *r)
{
	CSynchronized<std::list<IRunnable *> >::CAccessor acces(&_TaskQueue);
	acces.value().push_back(r);
}

/// Delete a task, only if task is not running, return true if found and deleted
bool CTaskManager::deleteTask(IRunnable *r)
{
	CSynchronized<list<IRunnable *> >::CAccessor acces(&_TaskQueue);
	for(list<IRunnable *>::iterator it = acces.value().begin(); it != acces.value().end(); it++)
	{
		if(*it == r)
		{
			acces.value().erase(it);
			return true;
		}
	}
	return false;
}

/// Task list size
uint CTaskManager::taskListSize(void)
{
	CSynchronized<list<IRunnable *> >::CAccessor acces(&_TaskQueue);
	return acces.value().size();
}


void	CTaskManager::waitCurrentTaskToComplete ()
{
	while (_IsTaskRunning)
		sleepTask();
}

// ***************************************************************************

void CTaskManager::dump (std::vector<std::string> &result)
{
	CSynchronized<string>::CAccessor accesCurrent(&_RunningTask);
	CSynchronized<list<IRunnable *> >::CAccessor acces(&_TaskQueue);
	CSynchronized<list<string> >::CAccessor accesDone(&_DoneTaskQueue);

	const list<IRunnable *> &taskList = acces.value();
	const list<string> &taskDone = accesDone.value();
	const string &taskCurrent = accesCurrent.value();
	
	// Resize the destination array
	result.clear ();
	result.reserve (taskList.size () + taskDone.size () + 1);

	// Add the waiting strings
	list<string>::const_reverse_iterator iteDone = taskDone.rbegin ();
	while (iteDone != taskDone.rend ())
	{
		result.push_back ("Done : " + *iteDone);
		
		// Next task
		iteDone++;
	}
	
	// Add the current string
	if (!taskCurrent.empty())
	{
		result.push_back ("Current : " + taskCurrent);
	}

	// Add the waiting strings
	list<IRunnable *>::const_iterator ite = taskList.begin ();
	while (ite != taskList.end ())
	{
		string name;
		(*ite)->getName (name);
		result.push_back ("Waiting : " + name);
	
		// Next task
		ite++;
	}
}

} // NLMISC
