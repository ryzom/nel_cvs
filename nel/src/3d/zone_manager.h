/** \file zone_manager.h
 * CZoneManager class
 *
 * $Id: zone_manager.h,v 1.3 2002/04/24 13:47:52 besson Exp $
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

#ifndef NL_ZONE_MANAGER_H
#define NL_ZONE_MANAGER_H

#include <string>
#include <map>
#include "nel/misc/types_nl.h"
#include "3d/zone.h"
//#include "nel/misc/task_manager.h"

#include "3d/async_file_manager.h"
#include "3d/zone_search.h"


namespace NL3D
{


/**
* CLoadZone contained a status of loading for each zone
*/
struct CLoadZone
{
	/// Name of zone
	std::string	NameZone;
	/// Unique ID of each zone
	uint16		IdZone;
	/// true if loading asked
	bool		LoadAsked;
	/// true if loaded
	bool		Loaded;
	/// true if loading in progress (Zone is in TaskManager)
	bool		LoadInProgress;
	/// Runnable instance addrese
	NLMISC::IRunnable *Runnable;
	/// Flag at true for stop to try loading none exist files
	bool		FileNotFound;
    
	CLoadZone() : NameZone (std::string("")), IdZone (0), LoadAsked (false), Loaded (false), LoadInProgress (false), Runnable(NULL), FileNotFound(false) {}
	CLoadZone (std::string name, uint16 idZone , bool loadAsked) : NameZone (name), IdZone (idZone), LoadAsked (loadAsked), Loaded (false), LoadInProgress (false), Runnable(NULL), FileNotFound(false) {}

	std::string	getNameWithoutExtension() const;
};

/**
 * CZoneManager is a class that manage zone loading around of player
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2000
 * sa See Also, CZoneSearch, CTaskManager
 */
class CZoneManager : public CZoneSearch
{
public:
	// Id of zone to remove
	uint16	IdZoneToRemove;
	// Set to true when zone is removed
	bool	ZoneRemoved;
	// Name of the Zone for remove to landscape
	std::string		NameZoneRemoved;

	// Set to true when zone is added
	bool	ZoneAdded;
	// Zone for add to landscape
	CZone	*Zone;
	// Name of the Zone for add to landscape
	std::string		NameZoneAdded;

	/// Constructor
	CZoneManager();

	/// Destructor
	~CZoneManager();

	/**
	* LoadAllZonesAround  Load all zones around a position
	* \param x is axis x coordinate (in meters)
	* \param y is axis y coordinate (in meters)
	* \area is area of zone loading (in meters)
	*/
	void loadAllZonesAround(uint x, uint y, uint area, bool scanAll = false);

	/**
	* GetTask give a pointer on TaskManager
	* \return pointer of class CTaskManager
	*/
	//inline NLMISC::CTaskManager* getTask(void) { 	return _pLoadTask; }

	/**
	* Get Size of TaskList
	* \return number of task in list
	*/
	inline uint getTaskListSize(void) { return CAsyncFileManager::getInstance().taskListSize(); }

	/**
	* Set Path for zone loading
	*/
	inline void setZonePath(std::string zonePath) { _zonePath = zonePath; }

	/**
	* Set Path for zone loading
	*/
	inline std::string getZonePath(void ) { return _zonePath; }

private:
	//Path for zone loading
	std::string _zonePath;
	//Zones loading / unloading taskmanager
	//NLMISC::CTaskManager *_pLoadTask;
	//Zones management with multimap
	std::multimap<uint32, CLoadZone> _LoadZone;

	std::list<std::pair<std::string, uint32> >::iterator _it2;
	std::list<std::pair<std::string, uint32> >::iterator _itEnd;
	std::list<std::pair<std::string, uint32> > _listZone;
	int step;
};


/**
 * CZoneLoadingTask implement run methode for loading a zone for TaskManager
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2000
 * sa See Also, CZoneManager, CTaskManager
 */
class CZoneLoadingTask : public NLMISC::IRunnable
{
public:
	/** Constructor
	* \param lz is a pointer on multimap which contained all managed zones
	* \param zm is a pointer on loading / unloading taskmanager
	*/
	CZoneLoadingTask(CLoadZone *lz, CZoneManager *zm);

	/// Runnable Task
	void run(void);

private:
	CLoadZone				*_Lz;
	CZoneManager			*_Zm;
};


/**
 * CZoneUnloadingTask implemente run methode for unloading a zone for TaskManager
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2000
 * sa See Also, CZoneManager, CTaskManager
 */
class CZoneUnloadingTask : public NLMISC::IRunnable
{
public:
	/** Constructor
	* \param lz is a pointer on multimap which contained all managed zones
	* \param zm is a pointer on loading / unloading taskmanager
	*/
	CZoneUnloadingTask(CLoadZone *lz, CZoneManager *zm);

	/// Runnable Task
	void run(void);

private:
	CLoadZone				*_Lz;
	CZoneManager			*_Zm;
};


} // NL3D


#endif // NL_ZONE_MANAGER_H

/* End of zone_manager.h */
