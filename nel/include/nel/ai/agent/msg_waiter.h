/** \file msg_waiter.h
 * Sevral class for the definition of agent.
 *
 * $Id: msg_waiter.h,v 1.1 2001/12/05 10:02:43 portier Exp $
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

#ifndef NL_MSGWAITER_H
#define NL_MSGWAITER_H

#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/msg.h"

namespace NLAIAGENT 
{

#define DEFAULT_TIMEOUT	5000

	class CMsgWaiter : public IBasicAgent
	{
		private:
			IMessageBase::TPerformatif	_Perf;
			IBasicAgent					*_Sender;
			sint32						_TimeOut;
			IObjectIA					*_Answer;
		public:
			CMsgWaiter(const IWordNumRef *, IMessageBase::TPerformatif  perf = IMessageBase::PTell, IBasicAgent *sender = NULL, sint32 timeout = DEFAULT_TIMEOUT);

			virtual void processMessages();	
			virtual const CProcessResult &run();
			IObjectIA *getAnswer();
			virtual IObjectIA *run(const IMessageBase &m);		
	};
}	// NLAIAGENT

#endif // NL_MSGWAITER_H
