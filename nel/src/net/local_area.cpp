/** \file local_area.cpp
 * The area all around a player
 *
 * $Id: local_area.cpp,v 1.13 2000/11/30 17:03:10 cado Exp $
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

#include "nel/net/local_area.h"
#include "nel/net/msg_socket.h"
#include "nel/misc/vector.h"
#include "nel/misc/debug.h"
#include "nel/net/remote_entity.h"
#include "nel/net/unitime.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;


// Pointer to the local area singleton
CLocalArea* CLocalArea::Instance = NULL;


// Creates a new remote entity
inline void NLNET::createRemoteEntity( const IMovingEntity& es )
{
	CRemoteEntity *new_entity = new CRemoteEntity( es );
	CLocalArea::Instance->addNeighbor( new_entity );
	if ( CLocalArea::Instance->_NewEntityCallback != NULL )
	{
		CLocalArea::Instance->_NewEntityCallback( new_entity );
	}
	nlinfo( "New remote entity created at %f , %f", new_entity->pos().x, new_entity->pos().y );
}


// Returns true and a valid iterator if the entity is found
inline bool NLNET::findEntity( TEntityId id, ItRemoteEntities& ire )
{
	ire = CLocalArea::Instance->_Neighbors.find( id );
	return ( ire != CLocalArea::Instance->_Neighbors.end() );
}


// Processes the entity state
void NLNET::processEntityState( const IMovingEntity& es )
{
	// Search id in the entity map
	ItRemoteEntities ire;
	if ( findEntity( es.id(), ire ) )
	{
		// Change state
		(*ire).second->convergeTo( es );
	}
	else
	{	// Not found => create a new remote entity
		createRemoteEntity( es );
	}
}


/*
 * Callback cbProcessEntityStateInGroundMode (friend of CLocalArea)
 */
void NLNET::cbProcessEntityStateInGroundMode( CMessage& msgin, TSenderId idfrom )
{
	IMovingEntity es;
	es.setGroundMode( true );
	msgin.serial( es );
	nldebug( "Entity state in ground mode received, with id %u", es.id() );

	// Search id in the entity map
	ItRemoteEntities ire;
	if ( findEntity( es.id(), ire ) )
	{
		// Change state
		(*ire).second->convergeTo( es );
	}
	else
	{	// Not found => create a new remote entity
		createRemoteEntity( es );
	}
}


/*
 * Callback cbProcessEntityStateFull (friend of CLocalArea)
 */
void NLNET::cbProcessEntityStateFull( CMessage& msgin, TSenderId idfrom )
{
	IMovingEntity es;
	es.setGroundMode( false );
	msgin.serial( es );
	nldebug( "Full entity state received, with id %u", es.id() );

	processEntityState( es );
}


/*
 * Callback cbAssignId (friend of CLocalArea)
 */
void NLNET::cbAssignId( CMessage& msgin, TSenderId idfrom )
{
	TEntityId id = 0;
	msgin.serial( id );
	CLocalArea::Instance->User.setId( id );
	nldebug( "Local entity has id %u", id );
}


/*
 * Callback cbRemoveEntity (friend of CLocalArea)
 */
void NLNET::cbRemoveEntity( CMessage& msgin, TSenderId idfrom )
{
	TEntityId id = 0;
	msgin.serial( id );
	CLocalArea::Instance->_Neighbors.erase( id );
	if ( CLocalArea::Instance->_EntityRemovedCallback != NULL )
	{
		CLocalArea::Instance->_EntityRemovedCallback( id );
	}
	nldebug( "Removed entity %u", id );
}


/*
 * Callback cbHandleDisconnection (friend of CLocalArea)
 */
void NLNET::cbHandleDisconnection( CMessage& msgin, TSenderId idfrom )
{
	nldebug( "Disconnection: local area goes off-line" );
	// Now ClientSocket->connected() is false
}


/*
 * Callback cbHandleUnknownMessage (friend of CLocalArea)
 */
void NLNET::cbHandleUnknownMessage( CMessage& msgin, TSenderId idfrom )
{
	if ( CLocalArea::Instance->_UnknownMessagesCallback != NULL )
	{
		CLocalArea::Instance->_UnknownMessagesCallback( msgin, idfrom );
	}
}


/*
 * Callback array
 */
TCallbackItem CbArray [] =
{
	{ "GES", cbProcessEntityStateInGroundMode },
	{ "FES", cbProcessEntityStateFull },
	{ "ID", cbAssignId },
	{ "RM", cbRemoveEntity },
	{ "D", cbHandleDisconnection },
	{ "O", cbHandleUnknownMessage }
};


CMsgSocket *ClientSocket;


namespace NLNET {


/*
 * Constructor
 */
CLocalArea::CLocalArea() :
	_Radius( 400 ),
	_NewEntityCallback( NULL ),
	_EntityRemovedCallback( NULL ),
	_UnknownMessagesCallback( NULL )
{
	nlassert( CLocalArea::Instance == NULL );
	CLocalArea::Instance = this;
	ClientSocket = new CMsgSocket( CbArray, sizeof(CbArray)/sizeof(CbArray[0]), "DRServer" );
	ClientSocket->setTimeout( 0 );
}


/*
 * Destructor
 */
CLocalArea::~CLocalArea()
{
	delete ClientSocket;
	for ( CRemoteEntities::iterator ire=_Neighbors.begin(); ire!=_Neighbors.end(); ++ire )
	{
		if ( (*ire).second != NULL ) 
		{
			delete (*ire).second;
		}
	}
}


/*
 * Initialization
 */
void CLocalArea::init()
{
	CUniTime::syncUniTimeFromService();
	_PreviousTime = CUniTime::getUniTime();
}


/*
 * Update the entity states
 */
void CLocalArea::update()
{
	if ( ClientSocket->connected() )
	{
		ClientSocket->update();
	}

	// Compute time difference
	TTime actualtime = CUniTime::getUniTime();
	TDuration deltatime = (TDuration)(sint64)(actualtime - _PreviousTime) / 1000.0f;
	_PreviousTime = actualtime;

	// Update all entities
	User.update( deltatime );
	CRemoteEntities::iterator ipe;
	for ( ipe=_Neighbors.begin(); ipe!=_Neighbors.end(); )
	{
		bool erased = false;

		// Update neighbor
		(*ipe).second->update( deltatime );

		// Remove neighbor if it exits from the local area
		if ( ((*ipe).second->pos()-User.pos()).norm() > _Radius )
		{
			NLMISC::CVector v1 = User.pos();
			NLMISC::CVector v2 = (*ipe).second->pos();
			delete (*ipe).second;
			CRemoteEntities::iterator ipebis = ipe;
			ipe++;
			_Neighbors.erase( ipebis );
			erased = true;
		}

		if ( !erased )
		{
			ipe++;
		}
	}
}


} // NLNET
