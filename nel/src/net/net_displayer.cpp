/* net_displayer.cpp
 *
 * Copyright, 2000 Nevrax Ltd.
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

/*
 * $Id: net_displayer.cpp,v 1.9 2000/10/13 08:57:37 cado Exp $
 *
 * Implementation of CNetDisplayer
 */

#include "nel/net/net_displayer.h"
#include "nel/net/message.h"
#include "nel/net/naming_client.h"

#include <string>


namespace NLNET {


/* This index must correspond to the index for "LOG" in CallbackArray in the Logging Service
 * (see CNetDisplayer::display())
 */
const sint16 LOG_CBINDEX = 0;


/*
 * Constructor
 */
CNetDisplayer::CNetDisplayer() :
	_Server( true, false ) // disable logging otherwise an infinite recursion may occur
{
	findAndConnect();
}


/*
 * Find the server (using the NS) and connect
 */
void CNetDisplayer::findAndConnect()
{
	CInetAddress servaddr;
	if ( CNamingClient::lookup( "LOGS", servaddr ) )
	{
		setLogServer( servaddr );
	}
}

/*
 * Sets logging server address
 */
void CNetDisplayer::setLogServer( const CInetAddress& logServerAddr )
{
	if ( ! _Server.connected() )
	{
		_ServerAddr = logServerAddr;
		try
		{
			_Server.connect( _ServerAddr );
		}
		catch( ESocket& )
		{
			// Silence
		}
	}
}



/*
 * Destructor
 */
CNetDisplayer::~CNetDisplayer()
{
	_Server.close();
}


/*
 * Sends the string to the logging server
 */
void CNetDisplayer::display( const std::string& str )
{
	try {
		if ( ! _Server.connected() )
		{
			findAndConnect();
		}
		CMessage msg( "", false );
		msg.setType( LOG_CBINDEX ); // we don't listen for incoming replies, therefore we must not use a type as string. 0 is the default action for CLogService : "LOG"
		msg.serial( const_cast<std::string&>(str) );
		_Server.send( msg );
	}
	catch( Exception& )
	{
		// Silence
	}
}


} // NLNET
