/** \file callback_net_base.h
 * Network engine, layer 3, base
 *
 * $Id: callback_net_base.h,v 1.25 2003/02/07 16:07:56 lecroart Exp $
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

#ifndef NL_CALLBACK_NET_BASE_H
#define NL_CALLBACK_NET_BASE_H

#undef USE_MESSAGE_RECORDER

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"

#include "nel/net/buf_net_base.h"
#include "nel/net/message.h"
#include "nel/net/inet_address.h"

#ifdef USE_MESSAGE_RECORDER
#include "nel/net/message_recorder.h"
#include <queue>
#endif

#include <vector>


namespace NLNET {

class CCallbackNetBase;

/** Callback function type for message processing
 *
 * msgin contains parameters of the message
 * from is the SockId of the connection, for a client, from is always the same value
 */
typedef void (*TMsgCallback) (CMessage &msgin, TSockId from, CCallbackNetBase &netbase);


/// Callback items. See CMsgSocket::update() for an explanation on how the callbacks are called.
typedef struct
{
	/// Key C string. It is a message type name, or "C" for connection or "D" for disconnection
	char			*Key;
	/// The callback function
	TMsgCallback	Callback;

} TCallbackItem;


/**
 * Layer 3
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CCallbackNetBase
{
public:
	virtual ~CCallbackNetBase() {}


	/** Sends a message to special connection.
	 * On a client, the hostid isn't used.
	 * On a server, you must provide a hostid. If you hostid = InvalidSockId, the message will be sent to all connected client.
	 */
	virtual void	send (const CMessage &buffer, TSockId hostid = InvalidSockId, bool log = true) = 0;

	uint64	getBytesSent () { return _BytesSent; }
	uint64	getBytesReceived () { return _BytesReceived; }

	virtual uint64	getReceiveQueueSize () = 0;
	virtual uint64	getSendQueueSize () = 0;

	virtual void displayReceiveQueueStat (NLMISC::CLog *log = NLMISC::InfoLog) = 0;
	virtual void displaySendQueueStat (NLMISC::CLog *log = NLMISC::InfoLog, TSockId destid = InvalidSockId) = 0;

	virtual void displayThreadStat (NLMISC::CLog *log = NLMISC::InfoLog) = 0;
	
	/** Force to send all data pending in the send queue.
	 * On a client, the hostid isn't used and must be InvalidSockId
	 * On a server, you must provide a hostid.
	 */
	virtual bool	flush (TSockId hostid = InvalidSockId) = 0;
	
	/**	Appends callback array with the specified array. You can add callback only *after* adding the server or the client.
	 * \param arraysize is the number of callback items.
	 */
	void	addCallbackArray (const TCallbackItem *callbackarray, NLMISC::CStringIdArray::TStringId arraysize);

	/// Sets default callback for unknown message types
	void	setDefaultCallback(TMsgCallback defaultCallback) { _DefaultCallback = defaultCallback; }

	/// Sets callback for disconnections (or NULL to disable callback)
	void	setDisconnectionCallback (TNetCallback cb, void *arg) { checkThreadId ();  _DisconnectionCallback = cb; _DisconnectionCbArg = arg; }

	/// returns the sockid of a connection. On a server, this function returns the parameter. On a client, it returns the connection.
	virtual TSockId	getSockId (TSockId hostid = InvalidSockId) = 0;

	/** Sets the callback that you want the other side calls. If it didn't call this callback, it will be disconnected
	 * If cb is NULL, we authorize *all* callback.
	 * On a client, the hostid must be InvalidSockId (or ommited).
	 * On a server, you must provide a hostid.
	 */
	void	authorizeOnly (const char *callbackName, TSockId hostid = InvalidSockId);

	/// Returns true if this is a CCallbackServer
	bool	isAServer () const { checkThreadId (); return _IsAServer; }

	/// Use this function to get the String ID Array needed when you want to create a message
	NLMISC::CStringIdArray	&getSIDA () { return _InputSIDA; }

	/// This function is implemented in the client and server class
	virtual bool	dataAvailable () { nlstop; return false; }
	/// This function is implemented in the client and server class
	virtual void	update ( sint32 timeout=0 ) { nlstop; }
	/// This function is implemented in the client and server class
	virtual bool	connected () const { nlstop; return false; }
	/// This function is implemented in the client and server class
	virtual void	disconnect (TSockId hostid = InvalidSockId) { nlstop; }

	/// Returns the address of the specified host
	virtual const	CInetAddress& hostAddress (TSockId hostid);

	/// Used by client and server class
	void	sendAllMyAssociations (TSockId to);

	/** Gives some association of the other side. The goal is, in specific case, we don't want to
	 * ask associations to the other side (client is not secure for example). In this case, we can
	 * set other side associations by hand using this functions.
	 */
	void	setOtherSideAssociations (const char **associationarray, NLMISC::CStringIdArray::TStringId arraysize);

	void	displayAllMyAssociations ();

	/// If you ignore all unknown id, the net will not ask for other side to know new association.
	/// It's used in the naming service for example because the naming client will never answer.
	/// In this case, it will always send the message with the full string name (slower)
	void	ignoreAllUnknownId(bool b)
	{
		_InputSIDA.ignoreAllUnknownId (b);
	}

	// Defined even when USE_MESSAGE_RECORDER is not defined
	enum TRecordingState { Off, Record, Replay };

protected:

	uint64	_BytesSent, _BytesReceived;

	/// Used by client and server class
	TNetCallback _NewDisconnectionCallback;

	/// Constructor.
	CCallbackNetBase( TRecordingState rec=Off, const std::string& recfilename="", bool recordall=true );

	/// Used by client and server class
	void baseUpdate ( sint32 timeout=0 );

	/// Read a message from the network and process it
	void processOneMessage ();

	/// On this layer, you can't call directly receive, It s the update() function that receive and call your callaback
	virtual void	receive (CMessage &buffer, TSockId *hostid) = 0;

	/* It's the layer4 that full the buffer association because it's based on callback system
	 * this is message association used when received a message number from the socket to know the
	 * associated message name
	 */
	NLMISC::CStringIdArray _InputSIDA;

	/* It's the layer4 that full the buffer association because it's based on callback system
	 * this is message association used when you want to send a message to a socket
	 */
	NLMISC::CStringIdArray _OutputSIDA;

	// contains callbacks
	std::vector<TCallbackItem>	_CallbackArray;
	
	// called if the received message is not found in the callback array
	TMsgCallback				_DefaultCallback;

	bool _IsAServer;
	bool _FirstUpdate;

	// ---------------------------------------
#ifdef USE_MESSAGE_RECORDER
	bool			replayDataAvailable();
	virtual bool	replaySystemCallbacks() = 0;
	void			noticeDisconnection( TSockId hostid );

	TRecordingState						_MR_RecordingState;
	sint64								_MR_UpdateCounter;

	CMessageRecorder					_MR_Recorder;
#endif
	// ---------------------------------------

private:

	NLMISC::TTime		_LastUpdateTime;
	NLMISC::TTime		_LastMovedStringArray;
	
	TNetCallback		 _DisconnectionCallback;
	void				*_DisconnectionCbArg;

	friend void cbnbMessageAskAssociations (CMessage &msgin, TSockId from, CCallbackNetBase &netbase);
	friend void cbnbMessageRecvAssociations (CMessage &msgin, TSockId from, CCallbackNetBase &netbase);

	friend void cbnbNewDisconnection (TSockId from, void *data);

protected:
	/// \todo ace: debug feature that we should remove one day nefore releasing the game
	uint	_ThreadId;
	void	checkThreadId () const;
};


} // NLNET


#endif // NL_CALLBACK_NET_BASE_H

/* End of callback_net_base.h */
