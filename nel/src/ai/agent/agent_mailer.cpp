/** \file agent_mailer.cpp
 *
 * $Id: agent_mailer.cpp,v 1.4 2001/01/17 10:42:55 chafik Exp $
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

#include "nel/ai/agent/agent_mailer.h"

namespace NLAIAGENT
{
	CLocalAgentMail::CLocalAgentMail(IBasicAgent *host):IBasicAgent(NULL)
	{
		_HostAgent = host;
		_HostAgent->connect(this);
	}

	CLocalAgentMail::~CLocalAgentMail()
	{
		if(_HostAgent) 
		{
			//_HostAgent->onKill(this);			
			Kill();

			//_HostAgent->release();

		}
		else Kill();
	}

	void CLocalAgentMail::runChildren()
	{

	}
	void CLocalAgentMail::processMessages()
	{
	}
	void CLocalAgentMail::onKill(IConnectIA *a)
	{
		//if(a == _HostAgent) _HostAgent->release();

	}

	tQueue CLocalAgentMail::isMember(const IVarName *h,const IVarName *m,const IObjectIA &p) const
	{
		if(_HostAgent)
		{
			return _HostAgent->isMember(h,m,p);
		}
		else
		{
			const NLAIAGENT::IObjectIA *cl = (const NLAIAGENT::IObjectIA *)(IAgent::IdAgent.getFactory())->getClass();
			return cl->isMember(h,m,p);
		}
	}
}
