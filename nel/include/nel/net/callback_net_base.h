/** \file callback_net_base.h
 * <File description>
 *
 * $Id: callback_net_base.h,v 1.1 2001/02/22 16:18:35 cado Exp $
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


namespace NLNET {


typedef uint32 TSockId;

/// Callback function for message processing
typedef void (*TNetCallback) ( TSockId, CMessage& );


/// Callback items. See CMsgSocket::update() for an explanation on how the callbacks are called.
typedef struct
{
	/// Key C string. It is a message type name, or "C" for connection or "D" for disconnection
	char			*Key;
	/// The callback function
	TNetCallback	Callback;

} TCallbackItem;


typedef sint16 TTypeNum;


/**
 * <Class description>
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CCallbackNetBase
{
public:

	/// Constructor
	CCallbackNetBase();

	///	Append callback array with the specified array
	void	addCallbackArray( const TCallbackItem *callbackarray, TTypeNum arraysize );

	/// Update the network (call this method evenly)
	void	update();

	/// Sets callback for detecting a disconnection
	void	setDisconnectionCallback( TNetCallback cb );
};


} // NLNET


#endif // NL_CALLBACK_NET_BASE_H

/* End of callback_net_base.h */
