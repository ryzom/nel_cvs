/** \file callback_net_base.cpp
 * <File description>
 *
 * $Id: callback_net_base.cpp,v 1.2 2001/02/22 18:04:25 cado Exp $
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
#include "nel/net/msg_socket.h"

namespace NLNET {


CSockIdToLayer4Map	CCallbackNetBase::_SockIdMap;


/*
 * cbProcessDisconnectionCallback
 */
void cbProcessDisconnectionCallback( CMessage& msg, TSockId id )
{
	CCallbackNetBase *obj = CCallbackNetBase::_SockIdMap[id];
	if ( obj->_DisconnectionCallback != NULL )
	{
		obj->_DisconnectionCallback( msg, id );
	}
}


/*
 * DisconnectionCallbackArray
 */
TCallbackItem DisconnectionCallbackArray [] =
{
	{ "D", cbProcessDisconnectionCallback }
};


/*
 * Constructor
 */
CCallbackNetBase::CCallbackNetBase() :
	_MsgSocket( NULL ),
	_CallbackArray( NULL ),
	_CbArraySize( 0 ),
	_DisconnectionCallback( NULL )
{
	// Setup disconnection handling
	addCallbackArray( DisconnectionCallbackArray, sizeof(DisconnectionCallbackArray)/sizeof(TCallbackItem) );
}


/*
 *	Append callback array with the specified array
 */
void CCallbackNetBase::addCallbackArray( const TCallbackItem *callbackarray, TTypeNum arraysize )
{
	// Allocate new array
	TTypeNum newarraysize = _CbArraySize + arraysize;
	TCallbackItem *newcbarray = new TCallbackItem [newarraysize];

	// Copy contents
	TTypeNum i;
	for ( i=0; i!=_CbArraySize; i++ )
	{
		//nldebug( "Callback %d: %s", i, oldClientCallbackArray[i].Key );
		newcbarray[i] = _CallbackArray[i];
	}
	//nldebug( "From callbackarray:" );
	for ( i=_CbArraySize; i!=newarraysize; i++ )
	{
		//nldebug( "Callback %d: %s", i, callbackarray[i-oldClientCbaSize].Key );
		newcbarray[i] = callbackarray[i-_CbArraySize];
	}

	// Modify array
	if ( _CallbackArray != NULL )
	{
		delete [] _CallbackArray;
	}
	_CallbackArray = newcbarray;
	_CbArraySize = newarraysize;
}


/*
 * Update the network (call this method evenly)
 */
void CCallbackNetBase::update()
{
	CMsgSocket::update();
}


/*
 * Sets callback for detecting a disconnection
 */
void CCallbackNetBase::setDisconnectionCallback( TNetCallback cb )
{
	_DisconnectionCallback = cb;
}




} // NLNET
