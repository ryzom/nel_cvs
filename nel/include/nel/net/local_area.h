/** \file local_area.h
 * The area all around a player
 *
 * $Id: local_area.h,v 1.14 2000/12/20 10:08:17 cado Exp $
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
#include "nel/misc/time_nl.h"
#include "nel/net/pt_callback_item.h"
#include "nel/misc/vector.h"
#include <map>


namespace NLNET {


/// Type of the map from entity identifies to remote entities
typedef std::map<TEntityId,CRemoteEntity*> CRemoteEntities;

/// Iterator on CRemoteEntities
typedef CRemoteEntities::iterator ItRemoteEntities;

/// Callbacks for creating and moving remote entities
typedef void (*TEntityCallback) ( CRemoteEntity* );

/// Callbacks for processing a remote entity using id (e.g. deletion)
typedef void (*TEntityIdCallback) ( TEntityId );


class CMessage;


/**
 * Neighborhood of a player.
 * The local area handles messages for entity moves / dead reckoning.
 * \warning This class is test code and is highly subject to change.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CLocalArea
{
public:

	/** Constructor. \e clientsocket is a pointer to a constructed client CMsgSocket object.
	 */
	CLocalArea( const CMsgSocket *clientsocket, const NLMISC::CVector& userpos=NLMISC::CVector(), const NLMISC::CVector& userhdg=NLMISC::CVector() );

	/// Destructor
	~CLocalArea();
	
	/// Initialization
	void					init();

	/// Update the entity states (does not call CMsgSocket::update(), you must call it before)
	void					update();

	/// Adds a replica of a remote entity to the area. It will be automatically removed when it exists.
	void					addNeighbor( CRemoteEntity *neighbor )
	{
		_Neighbors.insert( std::make_pair(neighbor->id(),neighbor) );
	}

	/// Read/write access to the neighbors
	CRemoteEntities&		neighbors()
	{
		return _Neighbors;
	}

	/// Provides a callback function to be called after creating a new remote entity
	void					setNewEntityCallback( TEntityCallback cb )
	{
		_NewEntityCallback = cb;
	}

	/// Provides a callback function to be called after deletion of a remote entity
	void					setEntityRemovedCallback( TEntityIdCallback cb )
	{
		_EntityRemovedCallback = cb;
	}

	/// Provides a callback function to be called after moving a remote entity
	void					setEntityMovedCallback( TEntityCallback cb )
	{
		_EntityMovedCallback = cb;
	}

	/// Returns the callback function to be called after moving a remote entity
	TEntityCallback			entityMovedCallback() const
	{
		return _EntityMovedCallback;
	}

	/// Provides a callback function to be called when receiving unknown messages
	void					setProcessUnknownMessagesCallback( TMsgCallback cb )
	{
		_UnknownMessagesCallback = cb;
	}

	/// Returns the name of an entity in the local area
	const std::string		*nameFromId( TEntityId id );	

	/// The entity controlled by the player
	CLocalEntity			User;

	/// Pointer to the client msg socket
	CMsgSocket				*ClientSocket;

	// Friend helper functions
	friend inline void createRemoteEntity( const IMovingEntity& es );
	friend inline bool findEntity( TEntityId id, ItRemoteEntities& ire );
	friend void processEntityState( IMovingEntity& es );

	// Callbacks
	friend void cbProcessEntityStateInGroundMode( CMessage& msgin, TSenderId idfrom );
	friend void cbProcessEntityStateFull( CMessage& msgin, TSenderId idfrom );
	friend void cbAssignId( CMessage& msgin, TSenderId idfrom );
	friend void cbRemoveEntity( CMessage& msgin, TSenderId idfrom );
	friend void cbCreateNewEntity( CMessage& msgin, TSenderId idfrom );
	friend void cbHandleDisconnection( CMessage& msgin, TSenderId idfrom );
	friend void cbHandleUnknownMessage( CMessage& msgin, TSenderId idfrom );
	
	/// Singleton
	static CLocalArea	*Instance;

private:

	TPosUnit			_Radius;
	CRemoteEntities		_Neighbors;
	TEntityCallback		_NewEntityCallback;
	TEntityIdCallback	_EntityRemovedCallback;
	TEntityCallback		_EntityMovedCallback;
	TMsgCallback		_UnknownMessagesCallback;

	NLMISC::TTime		_PreviousTime;
};


} // NLNET


#endif // NL_LOCAL_AREA_H

/* End of local_area.h */
