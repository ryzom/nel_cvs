/** \file stream_client.h
 * Network engine, layer 2, client
 *
 * $Id: stream_client.h,v 1.3 2002/05/21 16:38:21 lecroart Exp $
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

#ifndef NL_STREAM_CLIENT_H
#define NL_STREAM_CLIENT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/mem_stream.h"
#include "nel/net/buf_client.h"


namespace NLNET {


/**
 * Client class for layer 2
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
/*class CStreamClient___ : public CBufClient
{
public:

	/// Constructor
	CStreamClient( bool nodelay=true, bool replay=false ) : CBufClient( nodelay, replay ) {}

	/// Sends a message to the remote host
	void	send (const NLMISC::CMemStream &buffer);

	/// Receives next block of data in the specified buffer (resizes the vector)
	void	receive (NLMISC::CMemStream &buffer);

private:

	/// This function is public in the base class and we put it private here because user cannot use it in layer 2
	void	send (const std::vector<uint8> &buffer) { nlstop; }

	/// This function is public in the base class and we put it private here because user cannot use it in layer 2
	void	receive (std::vector<uint8> &buffer) { nlstop; }
};
*/

} // NLNET


#endif // NL_STREAM_CLIENT_H

/* End of stream_client.h */
