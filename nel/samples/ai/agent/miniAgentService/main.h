/** \file main.h
 * mini agent exemple
 *
 * $Id: main.h,v 1.3 2002/03/11 17:39:17 chafik Exp $
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
#include "nel/misc/path.h"

#define AgentServiceName "AgS"


namespace Expl
{	

	//This class is the output and input class for the agent class.
	//Later we could make one with a python interface, very simple but take a lot of time to write it (because nead mult-theard interface).
	class CIO : public NLAIC::IIO
	{
	public:
		static const NLAIC::CIdentType idCIO;
	public:

		CIO() {/*Nothing to do*/}

		virtual void Echo(char *txt, ...) const;
		virtual const std::string InPut() const
		{
			return std::string("\n");
		}

		virtual void save(NLMISC::IStream &os)
		{
			//Nothing to save.
		}		

		void getDebugString(std::string &t) const
		{
			//Nothing to debug.
			t = "this is an CIO";
		}

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new CIO();			
			return m;
		}

		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual void load(NLMISC::IStream &is)
		{
		}

		virtual const NLAIC::CIdentType &getType() const;				
		virtual ~CIO() {/*nothing to delete*/}
	};

	/**
	This class allow us to make the service.
	*/
	class CAgentService : public NLNET::IService5
	{
	public:
		static CAgentManager *Agent;
		static NLMISC::CPath *Path;
		static std::list<std::string> AgScript;
		static CIO *AgIO;
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
