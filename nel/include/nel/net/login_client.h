/** \file login_client.h
 * CLoginClient is the interface used by the client to identifies itself to the login_sytem and
 * connects to the shard.
 *
 * $Id: login_client.h,v 1.3 2001/06/13 10:21:33 lecroart Exp $
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

#ifndef NL_LOGIN_CLIENT_H
#define NL_LOGIN_CLIENT_H

#include <string>
#include <vector>

#include "nel/misc/types_nl.h"

#include "nel/net/callback_client.h"

namespace NLNET
{

class IDisplayer;

/**
 * Example:
 * \code
	CLoginClient::connect ("itsalive.nevrax.org:49999");
	string res = CLoginClient::authenticate ("login", "password");
	if (!res.empty ())
	{
		nlerror ("Authentification failed: %s", res.c_str());
	}
	CLoginClient::connectToShard (0, Connection);
 * \endcode
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CLoginClient {
public:

	struct CShardEntry
	{
		std::string ShardName;
		std::string WSAddr;
	};

	typedef std::vector<CShardEntry> TShardList;


	/** Set the graphics informations (card name, etc...) that will be send to the Login Service
	 *	ex: CLoginClient::setInformations (CNELU::Driver->getVideocardInformation ());
	 */
	static void setInformations (std::string gfxInfos) { _GfxInfos = gfxInfos; }


	/** Tries to connect to the authentification server.
	 * Generates a ESocketConnectionFailed if it can't establish the connection.
	 */
	/** Tries to login with login and password.
	 * If the authentification is ok, the function return an empty string else it returns the reason of the failure.
	 */
	static std::string authenticate (const std::string &loginServiceAddr, const std::string &login, const std::string &password, uint32 clientVersion);


	/** Try to connect to the shard and return the connection to the shard.
	 */
	static std::string connectToShard (uint32 shardListIndex, CCallbackClient &cnx);

	static TShardList ShardList;

private:

	static std::string _GfxInfos;

	static CCallbackClient *_CallbackClient;

};


} // NLNET

#endif // NL_LOGIN_CLIENT_H

/* End of login_client.h */
