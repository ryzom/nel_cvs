/** \file callback_client.h
 * Network engine, layer 3, client
 *
 * $Id: callback_client.h,v 1.16.14.1 2004/12/22 18:49:43 cado Exp $
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
#include "nel/net/buf_client.h"

namespace NLNET {


class CInetAddress;


/**
 * Client class for layer 3
 * \author Vianney Lecroart, Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CCallbackClient : public CCallbackNetBase, public CBufClient
{
public:

	/// Constructor
	CCallbackClient( TRecordingState rec=Off, const std::string& recfilename="", bool recordall=true, bool initPipeForDataAvailable=true );

	/// Sends a message to the remote host (the second parameter isn't used)
	void	send (const CMessage &buffer, TSockId hostid = InvalidSockId, bool log = true);

	/// Force to send all data pending in the send queue. hostid must be InvalidSockId here. See comment in CCallbackNetBase.
	bool	flush (TSockId hostid = InvalidSockId, uint *nbBytesRemaining=NULL);
	
	/// Updates the network (call this method evenly)
	void	update ( sint32 timeout=0 );

	/// Connects to the specified host
	void	connect( const CInetAddress& addr );

	/** Returns true if the connection is still connected (changed when a disconnection
	 * event has reached the front of the receive queue, just before calling the disconnection callback
	 * if there is one)
	 */
	virtual bool	connected () const { checkThreadId (); return CBufClient::connected (); } 

	virtual const CInetAddress&	hostAddress( TSockId hostid ) { return remoteAddress(); }

	/** Disconnect a connection
	 * Unlike in CCallbackClient, you can call disconnect() on a socket that is already disconnected
	 * (it will do nothing)
	 */
	void	disconnect (TSockId hostid = InvalidSockId);

	/// Sets callback for disconnections (or NULL to disable callback)
	void	setDisconnectionCallback (TNetCallback cb, void *arg) { checkThreadId (); CCallbackNetBase::setDisconnectionCallback (cb, arg); }

	/// Returns the sockid
	virtual TSockId	getSockId (TSockId hostid = InvalidSockId);

	uint64	getReceiveQueueSize () { return CBufClient::getReceiveQueueSize(); }
	uint64	getSendQueueSize () { return CBufClient::getSendQueueSize(); }

	void displayReceiveQueueStat (NLMISC::CLog *log = NLMISC::InfoLog) { CBufClient::displayReceiveQueueStat(log); }
	void displaySendQueueStat (NLMISC::CLog *log = NLMISC::InfoLog, TSockId destid = InvalidSockId) { CBufClient::displaySendQueueStat(log); }

	void displayThreadStat (NLMISC::CLog *log = NLMISC::InfoLog) { CBufClient::displayThreadStat(log); }
	
private:

	/// These function is public in the base class and put it private here because user cannot use it in layer 2
	void	send (const NLMISC::CMemStream &buffer) { nlstop; }

	/// Returns true if there are messages to read
	bool	dataAvailable ();

	void	receive (CMessage &buffer, TSockId *hostid = NULL);

	// ---------------------------------------
#ifdef USE_MESSAGE_RECORDER
	virtual bool replaySystemCallbacks();
#endif
	

	// debug features, we number all packet to be sure that they are all sent and received
	// \todo remove this debug feature when ok
	uint32 SendNextValue, ReceiveNextValue;
};


} // NLNET


#endif // NL_CALLBACK_CLIENT_H

/* End of callback_client.h */
