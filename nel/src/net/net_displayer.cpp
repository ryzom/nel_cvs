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
 * $Id: net_displayer.cpp,v 1.7 2000/10/10 15:28:15 cado Exp $
 *
 * Implementation of CNetDisplayer
 */

#include "nel/net/net_displayer.h"
#include "nel/net/message.h"

#include <string>


namespace NLNET {


/*
 * Constructor
 */
CNetDisplayer::CNetDisplayer() :
	_Server( true, false ) // disable logging otherwise an infinite recursion may occur
{
}


/*
 * Alt. Constructor
 */
CNetDisplayer::CNetDisplayer( const CInetAddress& logServerAddr ) :
	_Server( true, false ) // disable logging otherwise an infinite recursion may occur
{
	setLogServer( logServerAddr );
}


/*
 * Sets logging server address
 */
void CNetDisplayer::setLogServer( const CInetAddress& logServerAddr )
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
			_Server.connect( _ServerAddr );
		}
		CMessage msg( "", false );
		msg.setType( 0 ); // we don't listen for incoming replies, therefore we must not use a type as string. 0 is the default action for CLogService : "LOG"
		msg.serial( const_cast<std::string&>(str) );
		_Server.send( msg );
	}
	catch( Exception& )
	{
		// Silence
	}
}


} // NLNET
