/** \file callback_server.h
 * Network engine, layer 3, server
 *
 * $Id: callback_server.h,v 1.16.14.1 2004/12/22 18:49:43 cado Exp $
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

#ifndef NL_CALLBACK_SERVER_H
#define NL_CALLBACK_SERVER_H

#include "nel/misc/types_nl.h"

#include "nel/net/callback_net_base.h"
#include "nel/net/buf_server.h"


namespace NLNET {


/**
 * Server class for layer 3
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CCallbackServer : public CCallbackNetBase, public CBufServer
{
public:

	/// Constructor
	CCallbackServer( TRecordingState rec=Off, const std::string& recfilename="", bool recordall=true, bool initPipeForDataAvailable=true );

	/// Sends a message to the specified host
	void	send (const CMessage &buffer, TSockId hostid, bool log = true);

	/// Force to send all data pending in the send queue. See comment in CCallbackNetBase.
	bool	flush (TSockId destid, uint *nbBytesRemaining=NULL) { checkThreadId (); nlassert( destid != InvalidSockId ); return CBufServer::flush(destid, nbBytesRemaining); }

	/// Updates the network (call this method evenly)
	void	update (sint32 timeout=0);

	/// Sets callback for incoming connections (or NULL to disable callback)
	void	setConnectionCallback (TNetCallback cb, void *arg) { checkThreadId (); _ConnectionCallback = cb; _ConnectionCbArg = arg; }

	/// Sets callback for disconnections (or NULL to disable callback)
	void	setDisconnectionCallback (TNetCallback cb, void *arg) { checkThreadId (); CCallbackNetBase::setDisconnectionCallback (cb, arg); }

	/// Returns true if the connection is still connected. on server, we always "connected"
	bool	connected () const { checkThreadId (); return true; } 

	/** Disconnect a connection
	 * Set hostid to InvalidSockId to disconnect all connections.
	 * If hostid is not InvalidSockId and the socket is not connected, the method does nothing.
	 * Before disconnecting, any pending data is actually sent.
	 */
	void	disconnect (TSockId hostid);

	/// Returns the address of the specified host
	const CInetAddress& hostAddress (TSockId hostid) { nlassert(hostid!=InvalidSockId); checkThreadId(); return CBufServer::hostAddress (hostid); }

	/// Returns the sockid (cf. CCallbackClient)
	virtual TSockId	getSockId (TSockId hostid = InvalidSockId);

	uint64	getReceiveQueueSize () { return CBufServer::getReceiveQueueSize(); }
	uint64	getSendQueueSize () { return CBufServer::getSendQueueSize(0); }

	void displayReceiveQueueStat (NLMISC::CLog *log = NLMISC::InfoLog) { CBufServer::displayReceiveQueueStat(log); }
	void displaySendQueueStat (NLMISC::CLog *log = NLMISC::InfoLog, TSockId destid = InvalidSockId) { CBufServer::displaySendQueueStat(log, destid); }
	
	void displayThreadStat (NLMISC::CLog *log = NLMISC::InfoLog) { CBufServer::displayThreadStat(log); }

private:

	/// This function is public in the base class and put it private here because user cannot use it in layer 2
	void			send (const NLMISC::CMemStream &buffer, TSockId hostid) { nlstop; }

	bool			dataAvailable ();
	void			receive (CMessage &buffer, TSockId *hostid);

	void			sendAllMyAssociations (TSockId to);

	TNetCallback	_ConnectionCallback;
	void			*_ConnectionCbArg;

	friend void		cbsNewConnection (TSockId from, void *data);

	// ---------------------------------------
#ifdef USE_MESSAGE_RECORDER
	void						noticeConnection( TSockId hostid );
	virtual						bool replaySystemCallbacks();
	std::vector<CBufSock*>		_MR_Connections;
	std::map<TSockId,TSockId>	_MR_SockIds; // first=sockid in file; second=CBufSock*
#endif
	// ---------------------------------------
	
};


} // NLNET


#endif // NL_CALLBACK_SERVER_H

/* End of callback_server.h */
