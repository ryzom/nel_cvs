/** \file task_manager.cpp
 * <File description>
 *
 * $Id: task_manager.cpp,v 1.9 2002/10/10 12:41:50 berenguier Exp $
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

namespace NLMISC {

/*
 * Constructor
 */
CTaskManager::CTaskManager() : _TaskQueue ("")
{
	_IsTaskRunning = false;
	_ThreadRunning = true;
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
			runnableTask->run();
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


} // NLMISC
