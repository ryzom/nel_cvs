/** \file buf_sock.h
 * Network engine, layer 1, helper
 *
 * $Id: buf_sock.h,v 1.16 2002/06/12 10:16:41 lecroart Exp $
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

#ifndef NL_BUF_SOCK_H
#define NL_BUF_SOCK_H

#include "nel/misc/types_nl.h"
#include "nel/misc/hierarchical_timer.h"

#include "nel/net/buf_net_base.h"
#include "nel/net/tcp_sock.h"

//#include <deque>

namespace NLNET {


#define nlnettrace(__msg) //nldebug("LNETL1: %s",__msg);


class CTcpSock;

class CBufNetBase;


/**
 * CBufSock
 * A socket and its sending buffer
 */
class CBufSock
{
public:

	/// Destructor
	virtual ~CBufSock();

	/// Sets the application identifier
	void					setAppId( uint64 id ) { _AppId = id; }

	/// Returns the application identifier
	uint64					appId() const { return _AppId; }

	/// Returns a string with the characteristics of the object
	std::string				asString() const;

	/// Little tricky but this string is used by Layer4 to know which callback is authorized.
	/// This is empty when all callback are authorized.
	std::string				AuthorizedCallback;

	// debug features, we number all packet to be sure that they are all sent and received
	// \todo remove this debug feature when ok
	uint32					SendNextValue, ReceiveNextValue;

protected:

	friend class CBufClient;
	friend class CBufServer;
	friend class CClientReceiveTask;
	friend class CServerReceiveTask;

	friend class CCallbackClient;
	friend class CCallbackServer;
	friend class CCallbackNetBase;

	/// Constructor
	CBufSock( CTcpSock *sock=NULL );

	///@name Sending data
	//@{
	
	/// Update the network sending (call this method evenly). Returns false if an error occured.
	bool	update();

	/** Sets the time flush trigger (in millisecond). When this time is elapsed,
	 * all data in the send queue is automatically sent (-1 to disable this trigger)
	 */
	void	setTimeFlushTrigger( sint32 ms );

	/** Sets the size flush trigger. When the size of the send queue reaches or exceeds this
	 * calue, all data in the send queue is automatically sent (-1 to disable this trigger )
	 */
	void	setSizeFlushTrigger( sint32 size ) { _TriggerSize = size; }

	/** Forces to send all data pending in the send queue
	 * \returns False if an error has occured (e.g. the remote host is disconnected).
	 * To retrieve the reason of the error, call CSock::getLastError() and/or CSock::errorString()
	 */
	bool	flush();

	//@}

	/// Returns "CLT " (client)
	virtual std::string typeStr() const { return "CLT "; }

	/** Pushes a disconnection message into bnb's receive queue, if it has not already been done
	 * (returns true in this case). You can either specify a sockid (for server) or InvalidSockId (for client)
	 */
	bool advertiseDisconnection( CBufNetBase *bnb, TSockId sockid )
	{
#ifdef NL_DEBUG
		if ( sockid != InvalidSockId )
		{
			nlassert( sockid == this );
		}
#endif
		return advertiseSystemEvent( bnb, sockid, _KnowConnected, true, CBufNetBase::Disconnection );
	}

	
	/** Pushes a system message into bnb's receive queue, if the flags meets the condition, then
	 * resets the flag and returns true. You can either specify a sockid (for server) or InvalidSockId (for client).
	 */
	bool advertiseSystemEvent(
		CBufNetBase *bnb, TSockId sockid, bool& flag, bool condition, CBufNetBase::TEventType event )
	{
#ifdef NL_DEBUG
		if ( sockid != InvalidSockId )
		{
			nlassert( sockid == this );
		}
#endif
		// Test flag
		if ( flag==condition )
		{
			nldebug( "Pushing event to %s", asString().c_str() );
			std::vector<uint8> buffer;
			if ( sockid == InvalidSockId )
			{
				// Client: event type only
				buffer.resize( 1 );
				buffer[0] = event;
			}
			else
			{
				// Server: sockid + event type
				buffer.resize( sizeof(TSockId) + 1 );
				memcpy( &*buffer.begin(), &sockid, sizeof(TSockId) );
				buffer[sizeof(TSockId)] = event;
			}
			// Push
			bnb->pushMessageIntoReceiveQueue( buffer );

			// Reset flag
			flag = !condition;
			return true;
		}
		else
		{
			return false;
		}
	}

	/** Pushes a buffer to the send queue and update,
	 * or returns false if the socket is not physically connected the or an error occured during sending
	 */
	bool pushBuffer( const NLMISC::CMemStream& buffer )
	{
		nlassert (this != InvalidSockId);	// invalid bufsock
//		nldebug( "LNETL1: Pushing buffer to %s", asString().c_str() );

		static uint32 biggerBufferSize = 64000;
		if (buffer.length() > biggerBufferSize)
		{
			biggerBufferSize = buffer.length();
			nlwarning ("LNETL1: new record! bigger network message pushed (sent) is %u bytes", biggerBufferSize);
		}

		if ( Sock->connected() )
		{
			// Push into host's send queue
			SendFifo.push( buffer );

			// Update sending
			bool res = update ();
			return res; // not checking the result as in CBufServer::update()
		}
		return false;
	}

	/*bool pushBuffer( const std::vector<uint8>& buffer )
	{
		nlassert (this != InvalidSockId);	// invalid bufsock
//		nldebug( "LNETL1: Pushing buffer to %s", asString().c_str() );

		static uint32 biggerBufferSize = 64000;
		if (buffer.size() > biggerBufferSize)
		{
			biggerBufferSize = buffer.size();
			nlwarning ("LNETL1: new record! bigger network message pushed (sent) is %u bytes", biggerBufferSize);
		}

		if ( Sock->connected() )
		{
			// Push into host's send queue
			SendFifo.push( buffer );

			// Update sending
			bool res = update ();
			return res; // not checking the result as in CBufServer::update()
		}
		return false;
	}*/


	/// Connects to the specified addr; set connectedstate to true if no connection advertising is needed
	void connect( const CInetAddress& addr, bool nodelay, bool connectedstate );

	/// Disconnects; set connectedstate to false if no disconnection advertising is needed
	void disconnect( bool connectedstate );

	/// Sets the "logically connected" state (changed when processing a connection/disconnection callback)
	void setConnectedState( bool connectedstate ) { _ConnectedState = connectedstate; } 

	/// Returns the "logically connected" state (changed when processing a connection/disconnection callback)
	bool connectedState() const { return _ConnectedState; }

	// Send queue
	NLMISC::CBufFIFO	SendFifo;

	// Socket (pointer because it can be allocated by an accept())
	CTcpSock			*Sock;

	// Prevents from pushing a connection/disconnection event twice
	bool				_KnowConnected;

private:

#ifdef NL_DEBUG
	enum TFlushTrigger { FTTime, FTSize, FTManual };
	TFlushTrigger		_FlushTrigger;
#endif

	NLMISC::TTime		_LastFlushTime; // updated only if time trigger is enabled (TriggerTime!=-1)
	NLMISC::TTime		_TriggerTime;
	sint32				_TriggerSize;

	NLMISC::CObjectVector<uint8> _ReadyToSendBuffer;
	TBlockSize			_RTSBIndex;

	uint64				_AppId;

	// Connected state (from the user's point of view, i.e. changed when the connection/disconnection event is at the front of the receive queue)
	bool				_ConnectedState;
};


class CBufServer;


/**
 * CServerBufSock
 * A socket, its send buffer plus a nonblocking receiving system
 */
class CServerBufSock : public CBufSock
{
protected:

	friend class CBufServer;
	friend class CListenTask;
	friend class CServerReceiveTask;

	/** Constructor with an existing socket (created by an accept()).
	 * Don't forget to call setOwnerTask().
	 */
	CServerBufSock( CTcpSock *sock );

	/// Sets the task that "owns" the CServerBufSock object
	void						setOwnerTask( CServerReceiveTask* owner ) { _OwnerTask = owner; }

	/// Returns the task that "owns" the CServerBufSock object
	CServerReceiveTask			*ownerTask() { return _OwnerTask; }

	/** Receives a part of a message (nonblocking socket only)
	 * \return True if the message has been completely received
	 */
	bool						receivePart();

	/// Returns the filled buffer (call after receivePart() returns true)
	const std::vector<uint8>	receivedBuffer() const { nlnettrace( "CServerBufSock::receivedBuffer" ); return _ReceiveBuffer; }

	/** Pushes a connection message into bnb's receive queue, if it has not already been done
	 * (returns true in this case).
	 */
	bool						advertiseConnection( CBufServer *bnb )
	{
		return advertiseSystemEvent( (CBufNetBase*)bnb, this, _KnowConnected, false, CBufNetBase::Connection );
	}

	/// Returns "SRV " (server)
	virtual std::string			typeStr() const { return "SRV "; }

private:

	/// True after a connection callback has been sent to the user, for this connection
	bool				_Advertised;

	// True if the length prefix has already been read
	bool				_NowReadingBuffer;

	// Counts the number of bytes read for the current element (length prefix or buffer)
	TBlockSize			_BytesRead;

	// Length of buffer to read
	TBlockSize			_Length;

	// Buffer for nonblocking receives
	std::vector<uint8>	_ReceiveBuffer;

	// The task that "owns" the CServerBufSock object
	CServerReceiveTask	*_OwnerTask;

};


} // NLNET

#endif // NL_BUF_SOCK_H

/* End of buf_sock.h */
