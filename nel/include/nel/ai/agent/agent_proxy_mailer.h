/** \file agent_server_mailer.h
 * Sevral class for mailing message to an agent.
 *
 * $Id: agent_proxy_mailer.h,v 1.7 2001/02/05 10:36:24 chafik Exp $
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
#ifndef NL_AGENT_SERVER_MAILER_H
#define NL_AGENT_SERVER_MAILER_H
#include "nel/ai/agent/agent.h"

namespace NLAIAGENT
{
	class IProxy
	{
	public:
		virtual void send(IWordNumRef *_AgentRef,IMessageBase &m) = 0;
		/// Destructor
		virtual ~IProxy()
		{
		}

	};

	class CProxyAgentMail: public IBasicAgent
	{
	public:
		static IProxy *AgentProxy;

	private:
		IWordNumRef *_AgentRef;
	public:
		CProxyAgentMail(IWordNumRef *agent);
		~CProxyAgentMail();

	};
}

#endif
