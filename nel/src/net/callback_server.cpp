/** \file callback_server.cpp
 * Network engine, layer 3, server
 *
 * $Id: callback_server.cpp,v 1.10 2001/06/13 10:22:26 lecroart Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#include <string>

#include "nel/misc/string_id_array.h"

#include "nel/net/callback_server.h"

using namespace std;
using namespace NLMISC;

namespace NLNET {

void cbsNewConnection (TSockId from, void *data)
{
	nlassert (data != NULL);
	CCallbackServer *server = (CCallbackServer *)data;

	nldebug("L3S: newConnection()");

	// send all my association to the new client
	server->sendAllMyAssociations (from);

	// call the client callback if necessary
	if (server->_ConnectionCallback != NULL)
		server->_ConnectionCallback (from, server->_ConnectionCbArg);
}


CCallbackServer::CCallbackServer () : _ConnectionCallback(NULL), _ConnectionCbArg(NULL)
{
	CBufServer::setDisconnectionCallback (_NewDisconnectionCallback, this);
	CBufServer::setConnectionCallback (cbsNewConnection, this);

	_IsAServer = true;
}

void CCallbackServer::sendAllMyAssociations (TSockId to)
{
	checkThreadId ();
	nlassert (connected ());

	// he wants all associations
	CMessage msgout (getSIDA(), "RAA");

	CStringIdArray::TStringId size;
	size = _OutputSIDA.size ();

	nldebug ("L3S: Send all (%d) my string association to %s", size, to->asString().c_str());
	
	msgout.serial (size);

	for (CStringIdArray::TStringId i = 0; i < size; i++)
	{
//		nldebug ("L3S:  sending association '%s' -> %d", _OutputSIDA.getString(i).c_str (), i);
		string str(_OutputSIDA.getString(i));
		msgout.serial (str);
		msgout.serial (i);
	}
	send (msgout, to);
}


/*
 * Send a message to the specified host
 */
void CCallbackServer::send (const CMessage &buffer, TSockId hostid, bool log)
{
	checkThreadId ();
	nlassert (connected ());
	nlassert (buffer.length() != 0);
	nlassert (buffer.typeIsSet());

//	if (log)
	{
		nldebug ("L3S: Server: send(%s, %s)", buffer.toString().c_str(), hostid->asString().c_str());
	}

	CStreamServer::send (buffer, hostid);
}


void CCallbackServer::update ( sint32 timeout )
{
	checkThreadId ();
	nlassert (connected ());

	//	nldebug ("L3S: Client: update()");
	baseUpdate ( timeout ); // first receive
	CStreamServer::update (); // then send
}

void CCallbackServer::receive (CMessage &buffer, TSockId *hostid)
{
	checkThreadId ();
	nlassert (connected ());

	CStreamServer::receive (buffer, hostid);
	buffer.readType ();
}

TSockId CCallbackServer::getSockId (TSockId hostid)
{
	checkThreadId ();
	nlassert (connected ());
	nlassert (hostid != NULL);

	return hostid;
}

} // NLNET
