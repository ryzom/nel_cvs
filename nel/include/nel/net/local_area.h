/** \file local_area.h
 * Local Area
 *
 * $Id: local_area.h,v 1.4 2000/11/07 16:44:44 cado Exp $
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

#ifndef NL_LOCAL_AREA_H
#define NL_LOCAL_AREA_H

#include "nel/misc/types_nl.h"
#include "nel/net/local_entity.h"
#include "nel/net/remote_entity.h"
#include "nel/net/socket.h"
#include <map>


namespace NLNET {


typedef std::map<TEntityId,CRemoteEntity*> CRemoteEntities;
typedef CRemoteEntities::iterator ItRemoteEntities;

class CMessage;


/**
 * Neighborhood of a player
 * \warning This class is test code and is highly subject to change.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CLocalArea
{
public:

	/// Constructor
	CLocalArea();

	/// Destructor
	~CLocalArea();
	
	/// Initialization
	void	init();

	/// Update the entity states
	void	update();

	/// Adds a replica of a remote entity to the area. It will be automatically removed when it exists.
	void	addNeighbor( CRemoteEntity *neighbor )
	{
		_Neighbors.insert( std::make_pair(neighbor->id(),neighbor) );
	}

	/// Read access to the neighbors
	const CRemoteEntities& neighbors() const
	{
		return _Neighbors;
	}

	/// The entity controlled by the player
	CLocalEntity	User;

	// Friend helper functions
	friend inline void createRemoteEntity( const IMovingEntity& es );
	friend inline bool findEntity( TEntityId id, ItRemoteEntities& ire );
	friend void processEntityState( const IMovingEntity& es );

	// Callbacks
	friend void cbProcessEntityStateInGroundMode( CMessage& msgin, TSenderId idfrom );
	friend void cbProcessEntityStateFull( CMessage& msgin, TSenderId idfrom );
	friend void cbAssignId( CMessage& msgin, TSenderId idfrom );
	friend void cbRemoveEntity( CMessage& msgin, TSenderId idfrom );

	/// Singleton
	static CLocalArea *Instance;

private:

	TPosUnit		_Radius;
	CRemoteEntities	_Neighbors;

#ifdef NL_OS_WINDOWS
	uint32			_PreviousTime;
#endif
};


} // NLNET


#endif // NL_LOCAL_AREA_H

/* End of local_area.h */
