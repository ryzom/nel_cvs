/** \file main.h
 * mini agent exemple
 *
 * $Id: main.h,v 1.2 2002/03/11 16:58:25 chafik Exp $
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

#include "nel/misc/types_nl.h"
#include "nel/net/service_5.h"
#include "nel/net/unified_network.h"


#ifndef EXMPL_MAIN_H
#define EXMPL_MAIN_H

#include "main_agent.h"

#define AgentServiceName "AgS"


namespace NLMISC
{	
	class CPath;
}
namespace Expl
{	

	/**
	This class allow us to make the service.
	*/
	class CAgentService : public NLNET::IService5
	{
	public:
		static CAgentManager *_Agent;
		static NLMISC::CPath *Path;
	public:
		
		CAgentService();
		/// Init overload
		virtual void		init();
		///Release of the service
		virtual void		release ();
		///Update of the service.
		virtual bool		update ();	

		virtual ~CAgentService()
		{			
		}

	public:
		/**
		This method take a NLNET message who it have the distributed agent message.
		*/
		static void cbProcessAgentMessage(NLNET::CMessage& agtinmsg, const std::string &name, uint16 sid );	
	};
}

#endif
