/** \file msg_net_base.h
 * Network engine, layer 1, base
 *
 * $Id: msg_net_base.h,v 1.1 2001/05/02 12:36:30 lecroart Exp $
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

#ifndef NL_MSG_NET_BASE_H
#define NL_MSG_NET_BASE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/string_id_array.h"

#include "nel/net/message.h"

#include "nel/net/buf_net_base.h"

namespace NLNET {


/**
 * Layer 3
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CMsgNetBase
{
public:

	// it's the layer4 that full the buffer association because it's based on callback system
	// this is message association used when you want to send a message to a socket
	NLMISC::CStringIdArray OutputSIDA;

	// it's the layer4 that full the buffer association because it's based on callback system
	// this is message association used when received a message number from the socket to know the
	// associated message name
	NLMISC::CStringIdArray InputSIDA;

	virtual void	send (const CMessage &buffer, TSockId hostid) = 0;

protected:


	/// Constructor
	CMsgNetBase() {}

	/// Destructor
	virtual ~CMsgNetBase() {}

};


} // NLNET


#endif // NL_MSG_NET_BASE_H

/* End of msg_net_base.h */
