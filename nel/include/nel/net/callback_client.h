/** \file callback_client.h
 * Network engine, layer 3, client
 *
 * $Id: callback_client.h,v 1.5 2001/05/17 15:39:54 cado Exp $
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

#ifndef NL_CALLBACK_CLIENT_H
#define NL_CALLBACK_CLIENT_H

#include "nel/misc/types_nl.h"

#include "nel/net/callback_net_base.h"
#include "nel/net/stream_client.h"

namespace NLNET {


class CInetAddress;


/**
 * Client class for layer 3
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CCallbackClient : public CCallbackNetBase, public CStreamClient
{
public:

	CCallbackClient ();

	/// Sends a message to the remote host (the second parameter isn't used
	void	send (const CMessage &buffer, TSockId hostid = 0, bool log = true);

	/// Force to send all data pending in the send queue.
	bool	flush (TSockId hostid = 0) { return CStreamClient::flush(); }
	
	/// Updates the network (call this method evenly)
	void	update ( sint32 timeout=0 );

	/// Returns true if the connection is still connected
	virtual bool	connected () const { return CStreamClient::connected (); } 

	/// Disconnect a connection
	void	disconnect (TSockId hostid = 0) { CStreamClient::disconnect (); }

	/// Sets callback for disconnections (or NULL to disable callback)
	void	setDisconnectionCallback (TNetCallback cb, void *arg) { CCallbackNetBase::setDisconnectionCallback (cb, arg); }

	virtual TSockId	getSockId (TSockId hostid = 0);

private:

	/// These function is public in the base class and put it private here because user cannot use it in layer 2
	void	send (const NLMISC::CMemStream &buffer) { nlstop; }

	bool	dataAvailable () { return CStreamClient::dataAvailable (); }
	void	receive (CMessage &buffer, TSockId *hostid = NULL);

};


} // NLNET


#endif // NL_CALLBACK_CLIENT_H

/* End of callback_client.h */
