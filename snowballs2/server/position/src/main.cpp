/*
 * This file contain the Snowballs Position Service.
 *
 * $Id: main.cpp,v 1.1 2001/07/23 13:30:41 valignat Exp $
 */

/*
 * Copyright, 2000 - 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */


// This include is mandatory to use NeL. It include NeL types.
#include <nel/misc/types_nl.h>

#include <nel/misc/vector.h>

// We're using the NeL Service framework, and layer 4
#include <nel/net/service.h>
#include <nel/net/net_manager.h>


using namespace NLMISC;
using namespace NLNET;


/****************************************************************************
 * Function:   cbAddEntity
 *             Callback function called when the Position Service receive a
 *             "ADD_ENTITY" message
 * Arguments:
 *             - msgin:  the incomming message
 *             - from:   the "sockid" of the sender (usually useless for a
 *                       CCallbackClient)
 *             - server: the CCallbackNetBase object (which really is a
 *                       CCallbackServer object, for a server)
 ****************************************************************************/
void cbAddEntity ( CMessage& msgin, TSockId from, CCallbackNetBase& server )
{
	uint32  id;
	string  name;
	uint8   race;
	CVector startPoint;

	// Extract the incomming message content from the Frontend and print it
	msgin.serial( id );
	msgin.serial( name );
	msgin.serial( race );
	msgin.serial( startPoint );
	nlinfo( "Received ADD_ENTITY line" );

	// Prepare to send back the message.
	CMessage msgout( CNetManager::getSIDA( "POS" ), "ADD_ENTITY" );
	msgout.serial( id );
	msgout.serial( name );
	msgout.serial( race );
	msgout.serial( startPoint );

	/*
	 * Send the message to all the connected Frontend. If we decide to send
	 * it back to the sender, that last argument should be 'from' inteed of '0'
	 */
	CNetManager::send( "POS", msgout, 0 );

	nlinfo( "Send back ADD_ENTITY line" );
}


/****************************************************************************
 * Function:   cbPosition
 *             Callback function called when the Position Service receive a
 *             "ENTITY_POS" message
 * Arguments:
 *             - msgin:  the incomming message
 *             - from:   the "sockid" of the sender (usually useless for a
 *                       CCallbackClient)
 *             - server: the CCallbackNetBase object (which really is a
 *                       CCallbackServer object, for a server)
 ****************************************************************************/
void cbPosition ( CMessage& msgin, TSockId from, CCallbackNetBase& server )
{
	uint32  id;
	CVector pos;
	float   angle;
	uint32  state;

	// Extract the incomming message content from the Frontend and print it
	msgin.serial( id );
	msgin.serial( pos );
	msgin.serial( angle );
	msgin.serial( state );
	nlinfo( "Received ENTITY_POS line" );

	// Prepare to send back the message.
	CMessage msgout( CNetManager::getSIDA( "POS" ), "ENTITY_POS" );
	msgout.serial( id );
	msgout.serial( pos );
	msgout.serial( angle );
	msgout.serial( state );

	/*
	 * Send the message to all the connected Frontend. If we decide to send
	 * it back to the sender, that last argument should be 'from' inteed of '0'
	 */
	CNetManager::send( "POS", msgout, 0 );

	nlinfo( "Send back ENTITY_POS line" );
}


/****************************************************************************
 * Function:   cbRemoveEntity
 *             Callback function called when the Position Service receive a
 *             "REMOVE_ENTITY" message
 * Arguments:
 *             - msgin:  the incomming message
 *             - from:   the "sockid" of the sender (usually useless for a
 *                       CCallbackClient)
 *             - server: the CCallbackNetBase object (which really is a
 *                       CCallbackServer object, for a server)
 ****************************************************************************/
void cbRemoveEntity ( CMessage& msgin, TSockId from, CCallbackNetBase& server )
{
	uint32 id;

	// Extract the incomming message content from the Frontend and print it
	msgin.serial( id );
	nlinfo( "Received REMOVE_ENTITY line" );

	// Prepare to send back the message.
	CMessage msgout( CNetManager::getSIDA( "POS" ), "REMOVE_ENTITY" );
	msgout.serial( id );

	/*
	 * Send the message to all the connected Frontend. If we decide to send
	 * it back to the sender, that last argument should be 'from' inteed of '0'
	 */
	CNetManager::send( "POS", msgout, 0 );

	nlinfo( "Send back REMOVE_ENTITY line" );
}


/****************************************************************************
 * CallbackArray
 *
 * It define the functions to call when receiving a specific message
 ****************************************************************************/
TCallbackItem CallbackArray[] =
{
	{ "ADD_ENTITY",    cbAddEntity    },
	{ "ENTITY_POS",    cbPosition     },
	{ "REMOVE_ENTITY", cbRemoveEntity }
};


/****************************************************************************
 * SNOWBALLS POSITION SERVICE MAIN Function
 *
 * This call create a main function for the POSITION service:
 *
 *    - based on the base service class "IService", no need to inherit from it
 *    - having the short name "POSITION"
 *    - having the long name "position_service"
 *    - listening on an automatically allocated port (0) by the naming service
 *    - and callback actions set to "CallbackArray"
 *
 ****************************************************************************/
NLNET_SERVICE_MAIN( IService,
					"POS",
					"position_service",
					0,
					CallbackArray )


/* end of file */
