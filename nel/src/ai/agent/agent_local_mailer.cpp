/** \file agent_local_mailer.cpp
 *
 * $Id: agent_local_mailer.cpp,v 1.8 2003/01/21 11:24:39 chafik Exp $
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

#include "nel/ai/agent/agent_local_mailer.h"
#include "nel/ai/script/interpret_object_message.h"
#include "nel/ai/agent/agent_digital.h"

namespace NLAIAGENT
{
	CLocalAgentMail::CLocalAgentMail(IBasicAgent *host):IAgent(NULL)
	{
		_HostAgent = host;
		if(_HostAgent != NULL) connect(_HostAgent);
		//
	}

	CLocalAgentMail::~CLocalAgentMail()
	{
		/*if(_HostAgent) 
		{
			//_HostAgent->onKill(this);			
			Kill();

			//_HostAgent->release();

		}
		else Kill();*/
	}

	void CLocalAgentMail::runChildren()
	{

	}
	void CLocalAgentMail::processMessages()
	{
	}
	void CLocalAgentMail::onKill(IConnectIA *a)
	{
		if(a == _HostAgent)
		{
			_HostAgent = NULL;
			IAgent::onKill(a);
			release();
		}
		else
		{
			IAgent::onKill(a);
		}
		
	}

	TQueue CLocalAgentMail::isMember(const IVarName *h,const IVarName *m,const IObjectIA &p) const
	{
		if(_HostAgent)
		{
			return _HostAgent->isMember(h,m,p);
		}
		else
		{
			const NLAIAGENT::IObjectIA *cl = (const NLAIAGENT::IObjectIA *)(IAgent::IdAgent->getFactory())->getClass();
			return cl->isMember(h,m,p);
		}
	}
	

	IObjectIA::CProcessResult CLocalAgentMail::sendMessage(IObjectIA *m)
	{
		IMessageBase *msg = (IMessageBase *)m;

		if(NLAISCRIPT::CMsgNotifyParentClass::IdMsgNotifyParentClass == msg->getType() )
		{			
			const INombreDefine *n = (const INombreDefine *)msg->getFront();
			if(n->getNumber() != 0.0)
			{
				const CLocalAgentMail *parent = (const CLocalAgentMail *)msg->get();
				setParent((const IWordNumRef *)*parent->getHost());
			}
			return IObjectIA::CProcessResult();			
		}
		else
		{
			return ((IObjectIA  *)_HostAgent)->sendMessage(msg);
		}
	}	
}
