/** \file net_displayer.cpp
 * CNetDisplayer class
 *
 * $Id: net_displayer.cpp,v 1.19 2001/05/18 14:46:46 lecroart Exp $
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

#include <sstream>

#include "nel/misc/debug.h"
#include "nel/misc/common.h"
#include "nel/misc/log.h"

#include "nel/net/net_displayer.h"
#include "nel/net/message.h"
#include "nel/net/naming_client.h"


#include <string>

using namespace std;
using namespace NLMISC;

namespace NLNET {


/* This index must correspond to the index for "LOG" in CallbackArray in the Logging Service
 * (see CNetDisplayer::display())
 */
const sint16 LOG_CBINDEX = 0;


/*
 * Constructor
 */
CNetDisplayer::CNetDisplayer(bool autoConnect)/* :
	_Server( true, false ) */// disable logging otherwise an infinite recursion may occur
{
	if (autoConnect) findAndConnect();
}


/*
 * Find the server (using the NS) and connect
 */
void CNetDisplayer::findAndConnect()
{
	if ( CNamingClient::lookupAndConnect( "LOGS", _Server ) )
	{
		nldebug( "Connected to logging service" );
		_ServerNumber = 1;
	}
}

/*
 * Sets logging server address
 */
void CNetDisplayer::setLogServer( const CInetAddress& logServerAddr )
{
	if (_ServerNumber==1 && _Server.connected()) return;
	if (_ServerNumber==2 && _Server2->connected()) return;

	_ServerAddr = logServerAddr;
	try
	{
		_Server.connect( _ServerAddr );
		_ServerNumber = 1;
	}
	catch( ESocket& )
	{
		// Silence
	}
}

void CNetDisplayer::setLogServer( CCallbackClient *server )
{
	if (_ServerNumber==1 && _Server.connected()) return;

	_Server2 = server;
	_ServerNumber = 2;
}


/*
 * Destructor
 */
CNetDisplayer::~CNetDisplayer()
{
	if (_ServerNumber == 1)
		_Server.disconnect();
}


/*
 * Sends the string to the logging server
 *
 * Log format: "2000/01/15 12:05:30 <LogType> <ProcessName>: <Msg>"
 */
void CNetDisplayer::doDisplay ( const TDisplayInfo& args, const char *message)
{
	try
	{
		CCallbackClient	*server;
		if (_ServerNumber == 1) server = &_Server;
		else if (_ServerNumber == 2) server = _Server2;
		else nlstop;

		if ( _ServerNumber == 1 && !_Server.connected() )
		{
			findAndConnect();
		}
		else if (_ServerNumber ==2 && !_Server2->connected())
		{
			return;
		}

		bool needSpace = false;
		stringstream ss;

		if (args.Date != 0)
		{
			ss << dateToHumanString(args.Date);
			needSpace = true;
		}

		if (args.LogType != CLog::LOG_NO)
		{
			if (needSpace) { ss << " "; needSpace = false; }
			ss << logTypeToString(args.LogType);
			needSpace = true;
		}

		if (!args.ProcessName.empty())
		{
			if (needSpace) { ss << " "; needSpace = false; }
			ss << args.ProcessName;
			needSpace = true;
		}
		
		if (needSpace) { ss << ": "; needSpace = false; }

		ss << message;

		CMessage msg(server->getSIDA(), "LOG" );
		string s = ss.str();
		msg.serial( s );
		server->send (msg, 0, false);
	}
	catch( NLMISC::Exception& )
	{
		// Silence
	}
}


} // NLNET
