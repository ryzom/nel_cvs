/** \file zone_manager.cpp
 * CZoneManager class
 *
 * $Id: zone_manager.cpp,v 1.5 2002/02/28 12:59:52 besson Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX D.T.C. SYSTEM.
 * NEVRAX D.T.C. SYSTEM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX D.T.C. SYSTEM is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX D.T.C. SYSTEM; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "std3d.h"

#include <iostream>

#include "3d/zone_manager.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"

using namespace std;
using namespace NLMISC;


namespace NL3D
{


/*
 * Constructor
 */
CZoneManager::CZoneManager()
{
	ZoneRemoved = true;
	ZoneAdded = true;
	Zone = NULL;
	_pLoadTask = new CTaskManager;
	step = 0;
}

/// Destructeur
CZoneManager::~CZoneManager()
{
	// kill thread.
	delete _pLoadTask;
	// After thread exit, delete any zone remainining.
	if(!ZoneAdded)
	{
		delete	Zone;
	}
	_listZone.clear();
}

/**
* LoadAllZonesAround  Load all zones around a position
* \param x is axis x coordinate (in meters)
* \param y is axis y coordinate (in meters)
* \area is area of zone loading (in meters)
*/
void CZoneManager::loadAllZonesAround(uint x, uint y, uint area, bool scanAll)
{
	multimap<uint32, CLoadZone>::iterator it;
	
	if(scanAll)
	{
		step = 0;
	}

	switch(step)
	{
		case 0:
			//Clear LoadAsked flag
			for(it = _LoadZone.begin(); it != _LoadZone.end(); it++)
			{
				(*it).second.LoadAsked = false;
			}
			
			_listZone.clear();
			getListZoneName(x, y, area, _listZone);
			
			_it2 = _listZone.begin();
			if(scanAll)
			{
				_itEnd = _listZone.end();
			}
			else
			{
				_itEnd = _it2;
				if( _itEnd != _listZone.end() )
				{
					++_itEnd;
				}
			}
			step = 1;

		case 1:
			//Set LoadAsked flag for each zone when load is requested
			for(; _it2 != _itEnd; ++_it2)
			{
				///Find on multimap not work, because the key can't be finded, only used for shorted insert strategy
				it = _LoadZone.begin();
				while( it != _LoadZone.end() )
				{
					if((*it).second.NameZone == (*_it2).first)
					{
						break;
					}
					it++;
				}

				if( (it != _LoadZone.end()) && ((*it).second.NameZone == (*_it2).first) )
				{
					(*it).second.LoadAsked = true;
				}
				else
				{
					/// New zone found, add it to multimap of managed zones
					_LoadZone.insert(multimap<uint32, CLoadZone>::value_type((*_it2).second, CLoadZone((*_it2).first, 0, true)));
				}
			}

			if( _itEnd != _listZone.end() )
			{
				++_itEnd;
				return;
			}
			else
			{
				//Add loading / unloading zone in TaskManager for all zones when needed
				for(it = _LoadZone.begin(); it != _LoadZone.end(); )
				{
					if((*it).second.LoadAsked)
					{
						if( !( (*it).second.Loaded || (*it).second.LoadInProgress || (*it).second.FileNotFound ) )
						{
							//Add loading to TaskManager
							CZoneLoadingTask *tsk = new CZoneLoadingTask(&((*it).second), this);
							_pLoadTask->addTask(tsk);
							(*it).second.Runnable = tsk;
							(*it).second.LoadInProgress = true;
						}
						it++;
					}
					// Remove task on task manager
			/*		else if( (*it).second.LoadInProgress )
					{
						if(_pLoadTask->deleteTask((*it).second.Runnable))
						{
							//STLPort not return an iterator for erase...
							multimap<uint32, CLoadZone>::iterator itPrev = it;
							it++;
							_LoadZone.erase(itPrev);
						}
						else
						{
							it++;
						}
					}
			*/		else if( (*it).second.Loaded )
					{
						if(!ZoneAdded && (Zone->getZoneId() == (*it).second.IdZone) )
						{
							it++;
						}
						else
						{
							CLoadZone *pLoadZone = new CLoadZone;
							*pLoadZone = (*it).second;
							CZoneUnloadingTask *tsk = new CZoneUnloadingTask(pLoadZone, this);
							_pLoadTask->addTask(tsk);
							//STLPort not return an iterator for erase...
							multimap<uint32, CLoadZone>::iterator itPrev = it;
							it++;
							_LoadZone.erase(itPrev);
						}
					}
					else
					{
						it++;
					}
				}
				step = 0;
			}
	}

}

/// Constructor
CZoneLoadingTask::CZoneLoadingTask(CLoadZone *lz, CZoneManager *zm)
{
	_Lz = lz;
	_Zm = zm;
}

/// Zone loading task
void CZoneLoadingTask::run(void)
{
	while(!_Zm->ZoneAdded)
	{
		_Zm->getTask()->sleepTask();
		// must test if thread wants to exit.
		if(!_Zm->getTask()->isThreadRunning())
		{
			delete this;
			return;
		}
	}

	_Zm->Zone = new CZone;
	string zonePath = _Zm->getZonePath() + _Lz->NameZone;
	CIFile file;
	if(file.open(zonePath))
	{
		_Zm->Zone->serial(file);
		file.close();

		_Lz->IdZone = _Zm->Zone->getZoneId();
		_Zm->NameZoneAdded= _Lz->getNameWithoutExtension();
		_Zm->ZoneAdded = false;
		_Lz->LoadInProgress = false;
		_Lz->Loaded = true;
	}
	else
	{
		_Lz->FileNotFound = true;
		delete _Zm->Zone;
	}
	delete this;
}

/// Constructor
CZoneUnloadingTask::CZoneUnloadingTask(CLoadZone *lz, CZoneManager *zm)
{
	_Lz = lz;
	_Zm = zm;
}

/// Zone loading task
void CZoneUnloadingTask::run(void)
{
	while(!_Zm->ZoneRemoved)
	{
		_Zm->getTask()->sleepTask();
		// must test if thread wants to exit.
		if(!_Zm->getTask()->isThreadRunning())
		{
			delete this;
			return;
		}
	}
	_Zm->IdZoneToRemove = _Lz->IdZone;
	_Zm->NameZoneRemoved= _Lz->getNameWithoutExtension();
	_Zm->ZoneRemoved = false;

	delete _Lz;
	delete this;
}


std::string	CLoadZone::getNameWithoutExtension() const
{
	return NameZone.substr(0, NameZone.find('.'));
}




} // NL3D

