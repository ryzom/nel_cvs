/** \file callback_server.cpp
 * <File description>
 *
 * $Id: callback_server.cpp,v 1.1 2001/02/22 16:18:35 cado Exp $
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

#include "nel/net/callback_server.h"


namespace NLNET {


/*
 * Constructor
 */
CCallbackServer::CCallbackServer()
{
}


/*
 * Listens on the specified port
 */
void CCallbackServer::init( uint16 port )
{

}


/*
 * Disconnect the specified host
 */
void CCallbackServer::disconnect( TSockId hostid )
{

}


/*
 * Send a message to the specified host
 */
void CCallbackServer::send( TSockId hostid, CMessage& outmsg )
{

}


/*
 * Sets callback for incoming connections
 */
void CCallbackServer::setConnectionCallback( TNetCallback cb )
{

}


} // NLNET
