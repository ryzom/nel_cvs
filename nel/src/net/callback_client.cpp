/** \file callback_client.cpp
 * Network engine, layer 3, client
 *
 * $Id: callback_client.cpp,v 1.6 2001/06/01 13:42:30 lecroart Exp $
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

#include "nel/net/callback_net_base.h"
#include "nel/net/callback_client.h"

#include "nel/net/stream_client.h"


namespace NLNET {

static void cbcMessageRecvAllAssociations (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	netbase.getSIDA().ignoreAllUnknownId (false);
	cbnbMessageRecvAssociations (msgin, from, netbase);
}

static TCallbackItem ClientMessageAssociationArray[] =
{
	{ "RAA", cbcMessageRecvAllAssociations },
};

CCallbackClient::CCallbackClient ()
{
	CBufClient::setDisconnectionCallback (_NewDisconnectionCallback, this);

	// add the callback needed to associate messages with id
	addCallbackArray (ClientMessageAssociationArray, sizeof (ClientMessageAssociationArray) / sizeof (ClientMessageAssociationArray[0]));

	_InputSIDA.ignoreAllUnknownId (true);

	_IsAServer = false;
}


/*
 * Send a message to the specified host
 */
void CCallbackClient::send (const CMessage &buffer, TSockId hostid, bool log)
{
	// no size limit anymore
	nlassert (buffer.length() != 0);
	nlassert (buffer.typeIsSet());

//	if (log)
	{
		nldebug ("L3C: Client: send(%s)", buffer.toString().c_str());
	}

	CStreamClient::send (buffer);
}


void CCallbackClient::update ( sint32 timeout )
{
//	nldebug ("L3C: Client: update()");

	CStreamClient::update ();
	baseUpdate (timeout);
}

void CCallbackClient::receive (CMessage &buffer, TSockId *hostid)
{
	*hostid = NULL;
	CStreamClient::receive (buffer);
	buffer.readType ();
}

TSockId	CCallbackClient::getSockId (TSockId hostid)
{
	nlassert (hostid == NULL);
	return id ();
}

} // NLNET
