/** \file main_agent.cpp
 * mini agent exemple
 *
 * $Id: main_agent.cpp,v 1.1 2002/03/04 10:50:32 chafik Exp $
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
}
