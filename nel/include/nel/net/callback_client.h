/** \file callback_client.h
 * Network engine, layer 4, client
 *
 * $Id: callback_client.h,v 1.3 2001/02/23 09:48:38 cado Exp $
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

#ifndef NL_CALLBACK_CLIENT_H
#define NL_CALLBACK_CLIENT_H

#include "nel/misc/types_nl.h"
#include "nel/net/callback_net_base.h"


namespace NLNET {


class CInetAddress;


/**
 * Client class for layer 4
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CCallbackClient : public CCallbackNetBase
{
public:

	/// Constructor
	CCallbackClient();

	/// Connects to the specified host (call addCallbackArray() before)
	void	connect( const CInetAddress& addr );

	/// Disconnect the remote host
	void	disconnect();

	/// Send a message to the remote host
	void	send( CMessage& outmsg );
};


} // NLNET


#endif // NL_CALLBACK_CLIENT_H

/* End of callback_client.h */
