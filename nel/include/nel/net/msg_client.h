/** \file msg_client.h
 * Network engine, layer 2, client
 *
 * $Id: msg_client.h,v 1.1 2001/05/02 12:36:30 lecroart Exp $
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

#ifndef NL_MSG_CLIENT_H
#define NL_MSG_CLIENT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"

#include "nel/net/stream_client.h"
#include "nel/net/msg_net_base.h"
#include "nel/net/message.h"


namespace NLNET {


/**
 * Client class for layer 1
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CMsgClient : public CStreamClient, public CMsgNetBase
{
public:

	/// Sends a message to the remote host
	virtual void	send (const CMessage &buffer, TSockId hostid = 0);

private:

	/// This function is public in the base class and put it private here because user cannot use it in layer 2
	void	send (const NLMISC::CMemStream &buffer) { nlstop; }

};


} // NLNET


#endif // NL_MSG_CLIENT_H

/* End of msg_client.h */
