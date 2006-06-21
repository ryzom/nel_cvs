/** \file welcome_service.cpp
 * Welcome Service (WS)
 *
 * $Id: welcome_service.h,v 1.1.4.7.2.2 2006/06/21 18:01:19 boucher Exp $
 *
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX D.T.C. SYSTEM.
 * NEVRAX D.T.C. SYSTEM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX D.T.C. SYSTEM is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX D.T.C. SYSTEM; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "nel/misc/types_nl.h"
#include "nel/misc/singleton.h"

#include "nel/net/module_manager.h"
#include "nel/net/module_builder_parts.h"

#include "welcome_service_itf.h"

std::string lsChooseShard (const std::string &userName,
						   const NLNET::CLoginCookie &cookie,
							const std::string &userPriv,
							const std::string &userExtended,
							WS::TUserRole userRole,
							uint32 instanceId,
							uint32 charSlot);


bool disconnectClient(uint32 userId);


namespace WS
{
	// welcome service module
	class CWelcomeServiceMod : 
		public NLNET::CEmptyModuleCommBehav<NLNET::CEmptyModuleServiceBehav<NLNET::CEmptySocketBehav<NLNET::CModuleBase> > >,
		public WS::CWelcomeServiceSkel,
		public NLMISC::CManualSingleton<CWelcomeServiceMod>
	{
		/// the ring session manager module (if any)
		NLNET::TModuleProxyPtr		_RingSessionManager;
		/// the login service module (if any)
		NLNET::TModuleProxyPtr		_LoginService;

		void onModuleUp(NLNET::IModuleProxy *proxy);
		void onModuleDown(NLNET::IModuleProxy *proxy);


		////// CWelcomeServiceSkel implementation 

		// ask the welcome service to welcome a user
		virtual void welcomeUser(NLNET::IModuleProxy *sender, uint32 userId, const std::string &userName, const NLNET::CLoginCookie &cookie, const std::string &priviledge, const std::string &exPriviledge, WS::TUserRole mode, uint32 instanceId);

		// ask the welcome service to disconnect a user
		virtual void disconnectUser(NLNET::IModuleProxy *sender, uint32 userId)
		{
			disconnectClient(userId);
		}

	public:
		CWelcomeServiceMod()
		{
			CWelcomeServiceSkel::init(this);
		}

		void reportWSOpenState(bool shardOpen)
		{
			if (_RingSessionManager == NULL) // skip if the RSM is offline
				return;

			CWelcomeServiceClientProxy wscp(_RingSessionManager);
			wscp.reportWSOpenState(this, shardOpen);
		}

		// forward response from the front end for a player slot to play in
		// to the client of this welcome service (usually the Ring Session Manager)
		void frontendResponse(NLNET::IModuleProxy *waiterModule, uint32 userId, const std::string &reason, const NLNET::CLoginCookie &cookie, const std::string &fsAddr)
		{
			CWelcomeServiceClientProxy wscp(waiterModule);
			wscp.welcomeUserResult(this, userId, reason.empty(), fsAddr, reason);
		}

		// send the current number of players on this shard to the Ring Session Manager
		void updateConnectedPlayerCount(uint32 nbOnlinePlayers, uint32 nbPendingPlayers)
		{
			if (_RingSessionManager == NULL) // skip if the RSM is offline
				return;

			CWelcomeServiceClientProxy wscp(_RingSessionManager);
			wscp.updateConnectedPlayerCount(this, nbOnlinePlayers, nbPendingPlayers);
		}

		// inform the LS that a pending client is lost
		void pendingUserLost(const NLNET::CLoginCookie &cookie);
	};

	struct TPendingFEResponseInfo
	{
		CWelcomeServiceMod			*WSMod;
		NLNET::TModuleProxyPtr		WaiterModule;
		uint32						UserId;
	};
	typedef std::map<NLNET::CLoginCookie, TPendingFEResponseInfo>	TPendingFeReponses;
	// the list of cookie string that are pending an
	TPendingFeReponses	PendingFeResponse;

} // namespace WS
