/** \file main_agent.h
 * mini agent exemple
 *
 * $Id: main_agent.h,v 1.2 2002/03/11 17:39:17 chafik Exp $
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

#ifndef EXMPL_MAIN_AGENT_H
#define EXMPL_MAIN_AGENT_H

#include "nel/ai/agent/main_agent_script.h"

namespace Expl
{
	
	/**
	This exception occure when an sid is not managed in this machin.
	*/
	class CExceptionSidError: public NLAIE::IException
	{

	public:
		
		CExceptionSidError(): NLAIE::IException("unexistant agent referency")
		{
		}
			
		const NLAIE::IException *clone() const
		{
			return new CExceptionSidError();
		}

		~CExceptionSidError()
		{
		}
	};

	/**
	The CAgentManager is the top level agent.
	Throw it we can add to it many of ather agent.
	*/
	class CAgentManager: public NLAIAGENT::CMainAgentScript
	{
	public:

		CAgentManager(NLAIC::IIO *io):CMainAgentScript(NULL,io)
		{
		}
	
		void init();
		
		/// Deliver a message to an agent which must be present in this agent service
		/// This function assum the distrubution of messages on the serverver.
		void deliverMsg(NLAIAGENT::IRefrence *, NLNET::CMessage& inmsg);
		NLAIAGENT::IRefrence *agentIsPresent(const NLAIAGENT::CAgentNumber &) const;


		///We have to make this metho concret.
		NLAIAGENT::IObjectIA::CProcessResult sendMessage(const NLAIAGENT::CAgentNumber &, NLAIAGENT::IObjectIA *);
		///We have to make this metho concret.
		NLAIAGENT::IObjectIA::CProcessResult sendMessage(const NLAIAGENT::CAgentNumber &,const NLAIAGENT::IVarName &, NLAIAGENT::IObjectIA *);
			
		/// Destructor
		virtual ~CAgentManager() {}
	};
}
#endif
