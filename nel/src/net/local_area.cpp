/** \file local_area.cpp
 * <File description>
 *
 * $Id: local_area.cpp,v 1.3 2000/10/24 16:39:42 cado Exp $
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

#ifdef NL_OS_WINDOWS
#include <windows.h>
#endif

using namespace NLNET;
using namespace std;


// Pointer to the local area singleton
CLocalArea* CLocalArea::Instance;


// Callback (friend of CLocalArea)
void NLNET::cbProcessEntityState( CMessage& msgin, TSenderId idfrom )
{
	IMovingEntity es;
	msgin.serial( es );
	nlinfo( "Entity state received, with id %u", es.id() );

	// Search id in the entity map
	CRemoteEntities::iterator ire;
	ire = CLocalArea::Instance->_Neighbors.find( es.id() );
	if ( ire == CLocalArea::Instance->_Neighbors.end() )
	{
		// Not found => create a new remote entity
		CRemoteEntity *new_entity = new CRemoteEntity( es );
		CLocalArea::Instance->addNeighbor( new_entity );
		nlinfo( "New remote entity created");
	}
	else
	{
		// Change state
		(*ire).second->changeStateTo( es );
	}
}


// Callback (friend of CLocalArea)
void NLNET::cbAssignId( CMessage& msgin, TSenderId idfrom )
{
	TEntityId id;
	msgin.serial( id );
	CLocalArea::Instance->User.setId( id );
}


TCallbackItem CbArray [] =
{
	{ "ES", cbProcessEntityState },
	{ "ID", cbAssignId }
};


CMsgSocket *ClientSocket;


namespace NLNET {


/*
 * Constructor
 */
CLocalArea::CLocalArea() :
	_Radius( 20 )
{
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
#ifdef NL_OS_WINDOWS
	_PreviousTime = timeGetTime();
#else
	throw Exception(); // not implemented
#endif
}


/*
 * Update the entity states
 */
void CLocalArea::update()
{
	CMsgSocket::update();

	// Compute time difference
#ifdef NL_OS_WINDOWS
	uint32 actualtime = timeGetTime(); // TODO: test resolution settings with timeBeginPeriod()
	TDuration deltatime = (TDuration)(actualtime - _PreviousTime) / 1000.0;
	_PreviousTime = actualtime;
#else
	throw Exception(); // not implemented
#endif

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
			float norm = (v2-v1).norm();
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
