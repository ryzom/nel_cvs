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
 * $Id: net_displayer.cpp,v 1.4 2000/10/04 14:34:10 cado Exp $
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
CNetDisplayer::CNetDisplayer( const CInetAddress& logServerAddr ) :
	_ServerAddr( logServerAddr ),
	_Server( false ) // disable logging otherwise an infinite recursion may occur
{
	try
	{
		_Server.connect( _ServerAddr );
		// No check if the server is a logging server
		//if ( ! handshake() )
		//{
		//	_Server.close();
		//}
	}
	catch( ESocket& )
	{
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
			//if ( ! handshake() )
			//{
			//	_Server.close();
			//}
		}
		CMessage msg( false );
		msg.setType( 0 ); // we don't listen for incoming replies, therefore we must not use a type as string. 0 is the default action for CLogService : "LOG"
		msg.serial( const_cast<std::string&>(str) );
		_Server.send( msg );
	}
	catch( Exception& )
	{
		// Silence...
	}
}


/*
 * Handshake between us and the server. Returns true if the connected server is a logging server.
 * At the moment, it does nothing and always returns true.
 */
//bool CNetDisplayer::handshake()
//{
//	return true;
//}


} // NLNET
