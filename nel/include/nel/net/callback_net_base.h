/** \file callback_net_base.h
 * Network engine, layer 4, base
 *
 * $Id: callback_net_base.h,v 1.6 2001/03/15 15:14:23 coutelas Exp $
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

#include "nel/misc/types_nl.h"

#include "nel/net/message.h"
#include "nel/net/pt_callback_item.h"

#include <map>


namespace NLNET {


typedef uint32 TSockId;

/// Callback function for message processing
typedef void (*TNetCallback) ( TSockId );


/// Callback items. See CMsgSocket::update() for an explanation on how the callbacks are called.
/*typedef struct
{
	/// Key C string. It is a message type name, or "C" for connection or "D" for disconnection
	char			*Key;
	/// The callback function
	TMsgCallback	Callback;

} TCallbackItem;
*/

typedef sint16 TTypeNum;


class CMsgSocket;
class CCallbackNetBase;

// Internal use
typedef std::map<TSockId,CCallbackNetBase*> CSockIdToLayer4Map;


/**
 * Layer 4
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CCallbackNetBase
{
public:

	/**	Append callback array with the specified array (call BEFORE init() or connect()).
	 * arraysize is the number of callback items.
	 */
	void	addCallbackArray( const TCallbackItem *callbackarray, TTypeNum arraysize );

	/// Update the network (call this method evenly)
	void	update();

	/// Sets callback for detecting a disconnection (or NULL to disable callback)
	void	setDisconnectionCallback( TNetCallback cb ) { _DisconnectionCallback = cb; }

	/// Sets timeout for receive() in milliseconds
	void	setTimeout( uint32 ms );

	// Internal use
	friend void cbProcessDisconnectionCallback( CMessage& msg, TSockId id );

protected:

	/// Constructor
	CCallbackNetBase();

	CMsgSocket			*_MsgSocket;

	TNetCallback		_DisconnectionCallback;

	TCallbackItem		*_CallbackArray;

	TTypeNum			_CbArraySize;

	static CSockIdToLayer4Map	_SockIdMap;
};


} // NLNET


#endif // NL_CALLBACK_NET_BASE_H

/* End of callback_net_base.h */
