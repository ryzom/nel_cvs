/** \file main_agent.cpp
 * mini agent exemple
 *
 * $Id: main_agent.cpp,v 1.2 2002/03/11 17:39:17 chafik Exp $
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

#include "nel/ai/nl_ai.h"
#include "nel/ai/agent/agent.h"
//for script
#include "nel/ai/script/codage.h"
#include "nel/ai/script/interpret_object_agent.h"
#include "main.h"
#include "main_agent.h"

namespace Expl
{
	void CAgentManager::deliverMsg(NLAIAGENT::IRefrence *ag, NLNET::CMessage& inmsg)
	{
		uint8 compN;
		
		inmsg.serial(compN);

		if(compN)
		{
			NLAIC::CIdentTypeAlloc msgFactory;
			inmsg.serial( msgFactory );

			NLAIAGENT::IVarName *name  = (NLAIAGENT::IVarName *)msgFactory.allocClass();;
			inmsg.serial( *name );
			inmsg.serial( msgFactory );
			NLAIAGENT::IMessageBase *msg = (NLAIAGENT::IMessageBase *) msgFactory.allocClass();
			inmsg.serial( *msg );
			msg->setReceiver(ag);
			ag->sendMessage(*name,msg);			
			name->release();
		}
		else
		{

			NLAIC::CIdentTypeAlloc msgFactory;
			inmsg.serial( msgFactory );
			NLAIAGENT::IMessageBase *msg = (NLAIAGENT::IMessageBase *) msgFactory.allocClass();
			inmsg.serial( *msg );			
			msg->setReceiver(ag);
			ag->sendMessage(msg);
		}
	}

	NLAIAGENT::IRefrence *CAgentManager::agentIsPresent( const NLAIAGENT::CAgentNumber & id) const
	{		
		return NLAIAGENT::CLocWordNumRef::getRef(NLAIAGENT::CNumericIndex(id));
		
	}

	NLAIAGENT::IObjectIA::CProcessResult CAgentManager::sendMessage(const NLAIAGENT::CAgentNumber &ref,const NLAIAGENT::IVarName &name, NLAIAGENT::IObjectIA *m)
	{		

		NLAIAGENT::CAgentNumber &id = (NLAIAGENT::CAgentNumber &)((const NLAIAGENT::IWordNumRef &)*this).getNumIdent().getId();
		if(ref.DynamicId == id.CreatorId)
		{			
			NLAIAGENT::IRefrence *r = NLAIAGENT::CLocWordNumRef::getRef(NLAIAGENT::CNumericIndex(ref));
			if(r != NULL) r->sendMessage(name,m);
		}
		else
		{
			NLNET::CMessage msg = NLNET::CMessage("AGT");

			uint8 compN = true;

			NLAIAGENT::CAgentNumber &nref = (NLAIAGENT::CAgentNumber &)ref;
			msg.serial(nref);

			msg.serial(compN);	

			msg.serial( (NLAIC::CIdentType &)name.getType() );
			msg.serial( (NLAIC::CIdentType &)name );

			msg.serial( (NLAIC::CIdentType &)m->getType() );
			msg.serial(*m);
			
			NLNET::CUnifiedNetwork::getInstance()->send((NLNET::TServiceId)ref.DynamicId, msg);
			m->release();
		}
		return NLAIAGENT::IObjectIA::CProcessResult();
	}

	NLAIAGENT::IObjectIA::CProcessResult CAgentManager::sendMessage(const NLAIAGENT::CAgentNumber &ref,NLAIAGENT::IObjectIA *m)
	{
		NLAIAGENT::CAgentNumber &id = (NLAIAGENT::CAgentNumber &)((const NLAIAGENT::IWordNumRef &)*this).getNumIdent().getId();
		if(ref.DynamicId == id.CreatorId)
		{
			NLAIAGENT::IRefrence *c = NLAIAGENT::CLocWordNumRef::getRef(NLAIAGENT::CNumericIndex(ref));
			if(c != NULL) c->sendMessage(m);
		}
		else
		{			
			NLNET::CMessage msg = NLNET::CMessage("AGT");
			uint8 compN = false;

			NLAIAGENT::CAgentNumber &nref = (NLAIAGENT::CAgentNumber &)ref;
			msg.serial(nref);

			msg.serial(compN);			

			msg.serial( (NLAIC::CIdentType &)m->getType() );
			msg.serial(*m);
						
			NLNET::CUnifiedNetwork::getInstance()->send((NLNET::TServiceId)ref.DynamicId, msg);
			m->release();
		}
		return NLAIAGENT::IObjectIA::CProcessResult();
	}	

	void CAgentManager::init()
	{
		NLAISCRIPT::IScriptDebugSource* sourceCode;
		std::string sourceString;

		uint8 erreur = 0;

		while(CAgentService::AgScript.begin() != CAgentService::AgScript.end())
		{
			std::string sourceString = CAgentService::Path->lookup(CAgentService::AgScript.front());
			if(sourceString != "")
			{
				_CodeContext->InputOutput->Echo("#########################\n");
				_CodeContext->InputOutput->Echo("#########################\n");
				_CodeContext->InputOutput->Echo("compiling '%s' file\n",sourceString.c_str());
				_CodeContext->InputOutput->Echo("#########################\n");
				_CodeContext->InputOutput->Echo("#########################\n");
				sourceCode = new NLAISCRIPT::CScriptDebugSourceFile(sourceString.c_str());
				sourceString = sourceCode->getSourceBuffer();

				NLAIAGENT::IObjectIA::CProcessResult r;
				NLAISCRIPT::CCompilateur *comp = new NLAISCRIPT::CCompilateur(*_CodeContext->InputOutput, sourceString.c_str(), sourceString.size(), sourceCode);	
				r = comp->Compile();
				delete comp;

				erreur |= (r.ResultState == NLAIAGENT::processError);

				if(!erreur)
				{
					if( r.Result != NULL)
					{
						NLAIAGENT::CMessageList *m = new NLAIAGENT::CMessageList();
						m->setPerformatif(NLAIAGENT::IMessageBase::PExec);
						NLAIAGENT::CMessageGroup group(1);
						m->setGroup(group);				
						m->push(r.Result);
						m->setSender(this);
						((NLAIAGENT::IObjectIA *)this)->sendMessage((NLAIAGENT::IObjectIA *)m);
					}
				}
				else
				{
					if( r.Result != NULL) r.Result->release();
				}
				delete sourceCode;
			}
			CAgentService::AgScript.pop_front();
		}
	}
}
