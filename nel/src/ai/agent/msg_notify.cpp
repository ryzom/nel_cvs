/** \file msg_group.cpp
 *
 * $Id: msg_notify.cpp,v 1.2 2001/01/31 15:16:39 chafik Exp $
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

#include "nel/ai/agent/msg_notify.h"
#include "nel/ai/agent/agent_digital.h"

namespace NLAIAGENT
{
	CNotifyParentScript::CNotifyParentScript(NLAISCRIPT::CMessageClass *b):CMessageScript(b)
	{		
		CVectorGroupType *x = new CVectorGroupType(2);
		setMessageGroup(x);
		setGroup(CMessageGroup::msgScriptingGroup);
		set(0,new DigitalType(0.0));
	}

	CNotifyParentScript::CNotifyParentScript(IBasicAgent *agent,NLAISCRIPT::CMessageClass *b):CMessageScript(b)
	{		
		CVectorGroupType *x = new CVectorGroupType(2);
		setMessageGroup(x);
		setGroup(CMessageGroup::msgScriptingGroup);
		if(agent == NULL)
		{
			set(0,new DigitalType(0.0));
		}
		else
		{
			set(0,new DigitalType(1.0));
			set(1,new CLocalAgentMail(agent));
		}
	}
}
