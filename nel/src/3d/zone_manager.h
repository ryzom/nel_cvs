/** \file zone_manager.h
 * CZoneManager class
 *
 * $Id: zone_manager.h,v 1.8 2003/06/03 13:05:02 corvazier Exp $
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

#include "3d/async_file_manager_3d.h"
#include "3d/zone_search.h"


namespace NL3D
{


typedef	volatile	CZone	*TVolatileZonePtr;


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

	/**
	 * A Work is a removed zone or a loaded zone
	 */
	struct SZoneManagerWork
	{
		// Set to true when zone is removed
		bool	ZoneRemoved;
		// Id of zone to remove
		uint16	IdZoneToRemove;
		// Name of the Zone for remove to landscape
		std::string	NameZoneRemoved;

		// Set to true when zone is added
		bool		ZoneAdded;
		// Zone for add to landscape
		CZone		*Zone;
		// Name of the Zone for add to landscape
		std::string	NameZoneAdded;
	};

public:

	/// Constructor
	CZoneManager();

	/// Destructor
	~CZoneManager();

	/// checkZonesAround : Add/Remove all zomes around a certain point
	/// If a work is currently completed remove it and began another one
	void checkZonesAround (uint x, uint y, uint area);

	/// Is a work has been completed ? 
	bool isWorkComplete (SZoneManagerWork &rWork);

	/// Does the manager is loading ?
	bool isLoading () const {return _LoadingZones.size () != 0;}

	/// Return the count of zone left to load
	uint getNumZoneLeftToLoad ();

	/**
	 * Accessors
	 */

	/// setZonePath : Set Path for zone loading
	inline void setZonePath (std::string zonePath) { _zonePath = zonePath; }

	/// getZonePath : Get Path for zone loading
	inline std::string getZonePath (void) { return _zonePath; }

private:

	/// Path for zone loading
	std::string _zonePath;

	std::vector<uint16> _LoadedZones;

	std::vector<uint16> _ZoneList; // Zone set at a given position
	uint32 _LastX, _LastY;

	// Object for a zone loading process
	class CLoadingZone
	{
	public:
		// Zone ID
		uint16				ZoneToAddId;
		
		// Zone name
		std::string			ZoneToAddName;

		// The pointer on the loaded zone
		TVolatileZonePtr	Zone;
	};

	// The synchronized list of zone waiting to be loaded
	std::list<CLoadingZone> _LoadingZones;

	// Removing Task
	bool _RemovingZone;
	uint16 _IdZoneToRemove;
};


/**
 * CZoneLoadingTask implement run methode for loading a zone for TaskManager
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2000
 * sa See Also, CZoneManager, CTaskManager
 */
class CZoneLoadingTask : public NLMISC::IRunnablePos
{
public:
	/// Constructor
	CZoneLoadingTask (const std::string &sZoneName, TVolatileZonePtr *ppZone, CVector &position);

	/// Runnable Task
	void run (void);
	void getName (std::string &result) const;
	
private:

	TVolatileZonePtr	*_Zone;
	std::string _ZoneName;
};


} // NL3D


#endif // NL_ZONE_MANAGER_H

/* End of zone_manager.h */
