/** \file stream_client.cpp
 * Network engine, layer 2, client
 *
 * $Id: stream_client.cpp,v 1.6 2002/02/28 15:22:50 lecroart Exp $
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

#include "stdnet.h"

#include "nel/misc/mem_stream.h"

#include "nel/net/stream_client.h"

using namespace NLMISC;

namespace NLNET {


/*
 * Send a message to the specified host
 */
void CStreamClient::send (const CMemStream &buffer)
{
	nlassert (buffer.length() != 0); // no size limit anymore

//	nldebug ("LNETL2C: send()");

	CBufClient::send (buffer.bufferAsVector ());
}

void CStreamClient::receive (NLMISC::CMemStream &buffer)
{
//	nldebug ("LNETL2C: receive()");

	CBufClient::receive (buffer.bufferAsVector ());
	buffer.resetBufPos ();
}

} // NLNET
