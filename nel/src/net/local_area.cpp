/** \file local_area.cpp
 * The area all around a player
 *
 * $Id: local_area.cpp,v 1.36 2001/01/16 11:23:03 cado Exp $
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
#include "nel/misc/time_nl.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;


//extern void DisplayClientDump(); // temp


// Pointer to the local area singleton
CLocalArea* CLocalArea::Instance = NULL;


// Creates a new remote entity
void NLNET::createRemoteEntity( const IMovingEntity& es )
{
	CRemoteEntity *new_entity = new CRemoteEntity( es );
	CLocalArea::Instance->addNeighbor( new_entity );
	if ( CLocalArea::Instance->_NewEntityCallback != NULL )
	{
		CLocalArea::Instance->_NewEntityCallback( new_entity );
	}
	nlinfo( "New remote entity %s created at %f , %f", new_entity->name().c_str(), new_entity->pos().x, new_entity->pos().y );
}


// Returns true and a valid iterator if the entity is found
bool NLNET::findEntity( TEntityId id, ItRemoteEntities& ire )
{
	ire = CLocalArea::Instance->_Neighbors.find( id );
	return ( ire != CLocalArea::Instance->_Neighbors.end() );
}


// Processes the entity state
void NLNET::processEntityState( IMovingEntity& es )
{	
	// Search id in the entity map
	ItRemoteEntities ire;
	if ( findEntity( es.id(), ire ) )
	{
		if ( CLocalArea::Instance->inRadius( es.pos() ) )
		{
			// Change state smoothly
			(*ire).second->convergeTo( es );
		}
		else
		{
			// Change state quickly
			(*ire).second->changeStateTo( es );
		}
	}
	else
	{	// Not found => create a new remote entity
		createRemoteEntity( es );
	}
}


/*
 * Callback cbProcessEntityState (friend of CLocalArea)
 */
void NLNET::cbProcessEntityState( CMessage& msgin, TSenderId idfrom )
{
	IMovingEntity es;
	msgin.serial( es );
	nldebug( "Entity state received, with id %u", es.id() );
	processEntityState( es );
}


/*
 * Callback cbAssignId (friend of CLocalArea)
 */
void NLNET::cbAssignId( CMessage& msgin, TSenderId idfrom )
{
	// Receive id
	TEntityId id = 0;
	msgin.serial( id );
	CLocalArea::Instance->User.setId( id );
	nldebug( "Local entity %s has id %u", CLocalArea::Instance->User.name().c_str(), id );
	nlinfo( "Entering online mode" );

	// Send entity state and name and type
	uint32 metype = CLocalArea::Instance->User.type();
	CMessage msgout( "NAM" );
	msgout.serial( CLocalArea::Instance->User );
	msgout.serial( const_cast<string&>(CLocalArea::Instance->User.name()) );
	msgout.serial( metype );
	CLocalArea::Instance->ClientSocket->send( msgout );

	// Create other remote entities
	uint32 number;
	string name;
	msgin.serial( number );
	IMovingEntity es;
	nldebug( "Retrieving %d player names:", number );
	for ( uint i=0; i!=number; i++ )
	{
		msgin.serial( es );
		msgin.serial( name );
		msgin.serial( metype );
		DebugLog.displayRaw( " %s (type %u)", name.c_str(), metype );
		es.setName( name );
		es.setType( metype );
		createRemoteEntity( es );
	}
	DebugLog.displayRawNL( "." );
}


/*
 * Callback cbRemoveEntity (friend of CLocalArea)
 */
void NLNET::cbRemoveEntity( CMessage& msgin, TSenderId idfrom )
{
	TEntityId id = 0;
	msgin.serial( id );
	/*nldebug( "Removing entity %u", id );
	DisplayClientDump();*/
	CRemoteEntities::iterator ire = CLocalArea::Instance->_Neighbors.find( id );
	if ( ire != CLocalArea::Instance->_Neighbors.end() )
	{
		if ( CLocalArea::Instance->_EntityRemovedCallback != NULL )
		{
			CLocalArea::Instance->_EntityRemovedCallback( id );
		}
		delete (*ire).second;
		CLocalArea::Instance->_Neighbors.erase( ire );
		nldebug( "Removed entity %u", id );
	}
	// else: it is the case when a shooter deletes his weapon (because blocked or too far)
	//       and so does a shooted entity
}


/*
 * Callback cbCreateNewEntity (friend of CLocalArea)
 */
void NLNET::cbCreateNewEntity( CMessage& msgin, TSenderId idfrom )
{
	// Receive name
	IMovingEntity es;
	string name;
	uint32 metype;
	msgin.serial( es );
	msgin.serial( name );
	msgin.serial( metype );
	
	// Create remote entity and set name
	nldebug( "Retrieving new player name: %s", name.c_str() );
	es.setName( name );
	es.setType( metype );
	createRemoteEntity( es );
	nldebug( "Entity %u is %s", es.id(), name.c_str() );
}


/*
 * Callback cbHandleDisconnection (friend of CLocalArea)
 */
void NLNET::cbHandleDisconnection( CMessage& msgin, TSenderId idfrom )
{
	nlinfo( "Disconnection: entering off-line mode" );
	// Now CLocalArea::Instance->ClientSocket->connected() is false
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
	{ "ES", cbProcessEntityState },
	{ "ID", cbAssignId },
	{ "RM", cbRemoveEntity },
	{ "CRE", cbCreateNewEntity },
	{ "D", cbHandleDisconnection },
	{ "O", cbHandleUnknownMessage }
};


namespace NLNET {


/*
 * Constructor
 */
CLocalArea::CLocalArea( const CMsgSocket *clientsocket, const CVector& userpos, const CVector& userhdg ) :
	_Radius( 200 ),
	_NewEntityCallback( NULL ),
	_EntityRemovedCallback( NULL ),
	_UnknownMessagesCallback( NULL )
{
	nlassert( CLocalArea::Instance == NULL );
	CLocalArea::Instance = this;
	User.resetPos( userpos );
	User.resetBodyHeading( userhdg );
	ClientSocket = const_cast<CMsgSocket*>(clientsocket);
	if ( ClientSocket != NULL )
	{
		ClientSocket->addClientCallbackArray( CbArray, sizeof(CbArray)/sizeof(TCallbackItem) );
	}
}

	
/*
 * Destructor
 */
CLocalArea::~CLocalArea()
{
//	delete ClientSocket;
	ClientSocket = NULL;
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
	_PreviousTime = CTime::getLocalTime();
}


/*
 * Update the entity states
 */
void CLocalArea::update()
{
	// Compute time difference
 	TTime actualtime = CTime::getLocalTime();
	TDuration deltatime = (TDuration)(sint64)(actualtime - _PreviousTime) / 1000.0f;
	_PreviousTime = actualtime;
	//deltatime = 0.020;

	// Update local entity
	User.update( deltatime ); // note: you have to call CLocalArea.User.commitPos() after CLocalArea::update()
	User.keepAlive( actualtime );

	// Update remote entities
	CRemoteEntities::iterator ipe;
	for ( ipe=_Neighbors.begin(); ipe!=_Neighbors.end(); ++ipe )
	{
		//bool erased = false;

		CVector previoushdg = (*ipe).second->bodyHeading();

		// Update neighbor
		(*ipe).second->update( deltatime );

		/*nldebug( "Previous: %f\t%f\t%f", (*ipe).second->previousPos().x, (*ipe).second->previousPos().y, (*ipe).second->previousPos().z );
		nldebug( "Current:  %f\t%f\t%f", (*ipe).second->pos().x, (*ipe).second->pos().y, (*ipe).second->pos().z );*/

		if ( _EntityMovedCallback != NULL )
		{
			if ( ((*ipe).second->pos() != (*ipe).second->previousPos())
			  || ((*ipe).second->bodyHeading() != previoushdg) )
			{
				_EntityMovedCallback( (*ipe).second );
				(*ipe).second->setFull3d( false ); // go back into 2d mode after callback
			}
		}
	}
}


/*
 * Returns the name of an entity in the local area
 */
const string *CLocalArea::nameFromId( TEntityId id ) const
{
	if ( id == User.id() )
	{
		return &(User.name());
	}
	else
	{
		CRemoteEntities::const_iterator ipr = _Neighbors.find( id );
		if ( ipr != _Neighbors.end() )
		{
			return &((*ipr).second->name());
		}
		else
		{
			return NULL;
		}
	}
}


/*
 * Returns true if the specified position is within the radius of the local area
 */
bool CLocalArea::inRadius( const CVector& pos ) const
{
	return (pos - User.pos()).norm() < _Radius;
}


} // NLNET

