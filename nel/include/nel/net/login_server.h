/** \file login_server.h
 * CLoginServer is the interface used by the front end to accepts authenticate users.
 *
 * $Id: login_server.h,v 1.3 2001/05/03 16:26:27 coutelas Exp $
 * 
 */

/* Copyright, 2000 Nevrax Ltd.
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

#ifndef NL_LOGIN_SERVER_H
#define NL_LOGIN_SERVER_H

#include <string>
#include <vector>

#include "nel/misc/types_nl.h"

#include "nel/net/callback_server.h"
#include "nel/net/login_cookie.h"

namespace NLNET
{

/// Callback function type called when a new client is identified (with the login password procedure)
	typedef void (*TNewClientCallback) (TSockId from, const CLoginCookie &cookie);


class IDisplayer;

/** This class is the server part of the Login System. It is used in the Front End Service.
 * At the begining, it connects to the WS. When a new player comes in and is authenticated, a
 * callback is called to warn the user code that a new player is here.
 * Example:
 * \code
 * \endcode
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CLoginServer {
public:

	/// Create the connection to the Welcome Service and install callbacks to the callback server
	static void init (CCallbackServer &server, TNewClientCallback ncl);

	/// Call this method when a user is disconnected or the server disconnect the user.
	/// This method will warn the login system that the user is not here anymore
	static void clientDisconnected (uint32 userId);

private:

};


} // NLNET

#endif // NL_LOGIN_SERVER_H

/* End of login_server.h */
