/** \file main.cpp
 * mini agent exemple
 *
 * $Id: main.cpp,v 1.2 2002/03/11 16:58:25 chafik Exp $
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

#include "main.h"

//For init lib
#include "nel/ai/nl_ai.h"
#include "nel/misc/path.h"

namespace Expl
{
	CAgentManager *CAgentService::_Agent = NULL;
	NLMISC::CPath *CAgentService::Path = NULL;


	CAgentService::CAgentService()
	{
	}

	void CAgentService::init()
	{
		NLAILINK::setLocalServerID((uint8)serviceId());
		//This for uinitialize the ai lib
		NLAILINK::initIALib();
		//This for uinitialize the manager how deliver distruibuted message.
		NLAILINK::setMainManager(CAgentService::_Agent);
		//This time is allow message transfer.
		setUpdateTimeout(10);

		CAgentService::Path = new NLMISC::CPath;


		NLMISC::CConfigFile cf;

		try
		{
			//read the special agent var in the ag_s.cfg file.
			cf.load ("ag_s.cfg");
			NLMISC::CConfigFile::CVar &ScriptPath = cf.getVar ("ScriptPath");
			sint i;
			for(i = 0; i < ScriptPath.size(); i ++)
			{
				std::string s = ScriptPath.asString(i);
				CAgentService::Path->addSearchPath(s, true,true);
				//_ScriptPath.push_back(new std::string(s));
			}
			
		}
		catch (NLMISC::EConfigFile &e)
		{
			nlerror("%s",e.what());
		}
	}

	void CAgentService::release()
	{
		_Agent->release();
		//This lines is to run the agent internal timer.
		{
			NLMISC::CSynchronized<NLAIAGENT::CAgentScript *>::CAccessor accessor(NLAIAGENT::CAgentManagerTimer::TimerManager);
			accessor.value()->run();
		}
	}

	bool CAgentService::update()
	{
		_Agent->run();
		return true;
	}


	/**
	 * Callback that processes a message aimed at an agent
	 *
	 * Message expected : AGT
	 */
	void CAgentService::cbProcessAgentMessage(NLNET::CMessage& agtinmsg, const std::string &name, uint16 sid )
	{	
		// Agent header
		std::string msgname;

		///The sid of the agent
		NLAIAGENT::CAgentNumber dest_aid;
		agtinmsg.serial( dest_aid );		
		
		NLAIAGENT::IRefrence *ag;
		if ( (ag = _Agent->agentIsPresent( dest_aid )) != NULL )
		{		
			_Agent->deliverMsg( ag, agtinmsg );
		}
		else
		{

			throw CExceptionSidError();	
		}
	}
	
}

NLNET::TUnifiedCallbackItem CallbackArray [] =
{		
	///AGT is the name of the message of the NLNET message who it have the distributed agent message.
	{ "AGT", Expl::CAgentService::cbProcessAgentMessage }	
};

NLNET_SERVICE_MAIN( Expl::CAgentService, AgentServiceName, "ag_s", 0, CallbackArray );

