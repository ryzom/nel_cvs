/** \file socket.h
 * Implementation for CSocket.
 * Thanks to Daniel Bellen <huck@pool.informatik.rwth-aachen.de> for libsock++,
 * from which I took some ideas
 *
 * $Id: socket.h,v 1.22 2000/12/13 14:36:19 cado Exp $
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

#ifndef NL_SOCKET_H
#define NL_SOCKET_H


#include "nel/net/base_socket.h"
#include "nel/net/message.h"
#include "nel/net/pt_callback_item.h"
#include <map>
#include <string>

namespace NLNET
{


/// Map for msg name and msg type number association
typedef std::map<std::string,TTypeNum> CMsgMap;


/// Elements of CMsgMap
typedef std::pair<std::string,TTypeNum> TMsgMapItem;


/**
 * Socket for message transfer. Allows to send/receive CMessage objects.
 * Note: there are two methods for receiving : receive() which is blocking, and
 * received() which is non-blocking.
 *
 * For message handling, use CMsgSocket.
 * Each socket object contains a message map used to retrieve a message type code
 * from a message name (the key), because the remote host is the one who decides about
 * the binding.
 *
 * The "logging" boolean value is necessary because in this implementation we always log
 * to one single global CLog object : there is not one CLog object per socket. Therefore
 * we must prevent the socket used in CNetDisplayer from logging itself... otherwise we
 * would have an infinite recursion.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CSocket : public CBaseSocket
{
public:

	/// @name Socket setup
	//@{

	/**
	 * Constructor.
	 * \param reliable If true, creates a connected socket using TCP, otherwise an unconnected socket using UDP
	 * \param logging Disable logging if the server socket object is used by the logging system, to avoid infinite recursion
	 */
	CSocket( bool reliable = true, bool logging = true );

	/// Construct a CSocket object using an already connected socket and its associated address
	CSocket( SOCKET sock, const CInetAddress& remoteaddr ) throw (ESocket);

	/// Closure
	void	close();

	//@}


	/// @name Receiving a message
	//@{

	/// Checks if there are some data to receive
	bool	dataAvailable() throw (ESocket);

	/// Receives data, or blocks if !dataAvailable()). Returns false if !connected().
	bool	receive( CMessage& message ) throw (ESocket);

	/// Receives data (returns false if !dataAvailable() and does not block).
	bool	received( CMessage& message ) throw (ESocket);

	/** Receives a message (returns false if !dataAvailable()) (unreliable sockets only).
	 * The socket must have been bound before, by calling either bind() or sendTo().
	 * \param buffer [in] Address of buffer
	 * \param len [in] Length of buffer
	 * \param addr [out] Address of sender
	 */
	 bool	receivedFrom( CMessage& message, CInetAddress& addr );

	//@}


	/// @name Sending a message
	//@{

	/// Sends a message
	void	send( CMessage& message ) throw (ESocket);

	/// Sends a message to the specified host (unreliable sockets only)
	void	sendTo( CMessage& message, const CInetAddress& addr ) throw (ESocket);

	//@}

	/// CMsgSocket calls a few protected methods
	friend class CMsgSocket;

protected:

	/// Returns an output message with header encoded in the payload buffer
	CMessage	encode( CMessage& msg ) throw (ESocket);

	/// Returns an input message with header extracted from the payload buffer
	CMessage	decode( CMessage& alldata ) throw (ESocket);

	/// Helper method for receive() and received()
	void		doReceive( CMessage& message ) throw (ESocket);

	/// Process an incoming bind message
	void		processBindMessage( CMessage& message );

	/// Transforms a message replacing its string type by the corresponding num type if it is bound
	void		packMessage( CMessage& message );

	///@name These methods are provided only to be called by CMsgSocket (friend)
	//@{

	void			setDataAvailableFlag( bool da )			{ _DataAvailable = da; }
	void			setListening( bool l )					{ _IsListening = l; }
	void			setSenderId( TSenderId id )				{ _SenderId = id; }
	void			setOwnerClient( CMsgSocket *owner )		{ _OwnerClient = owner; }
	void			setAuthorizedCallback( TMsgCallback cb ){ _AuthCallback = cb; }

	bool			dataAvailableFlag() const				{ return _DataAvailable; }
	bool			isListening() const						{ return _IsListening; }
	TSenderId		senderId() const						{ return _SenderId;	}
	CMsgSocket		*ownerClient() const					{ return _OwnerClient; }
	TMsgCallback	authorizedCallback() const				{ return _AuthCallback; }

	//@}

private:

	CMsgMap			_MsgMap;
	uint8			_CurrentMsgNumberSend;

	bool			_DataAvailable; // can be modified only by CMsgSocket
	TSenderId		_SenderId;		// the same
	bool			_IsListening;	// the same
	CMsgSocket		*_OwnerClient;	// the same
	TMsgCallback	_AuthCallback;	// the same

};

} // NLNET

#endif // NL_SOCKET_H

/* End of socket.h */
