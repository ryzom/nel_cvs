/** \file msg_server.cpp
 * Network engine, layer 2, server
 *
 * $Id: msg_server.cpp,v 1.1 2001/05/02 12:36:31 lecroart Exp $
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

#include "nel/misc/debug.h"

#include "nel/net/message.h"
#include "nel/net/msg_server.h"

using namespace NLMISC;

namespace NLNET {


/*
 * Send a message to the specified host
 */
void CMsgServer::send (const CMessage &buffer, TSockId hostid)
{
	nlassert (buffer.length() > 0 && buffer.length() < 65536);

	CStreamServer::send (buffer, hostid);
}

} // NLNET
