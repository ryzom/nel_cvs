/** \file msg_notify.cpp
 *
 * $Id: msg_notify.cpp,v 1.10 2002/08/13 15:33:21 chafik Exp $
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
#include "nel/ai/script/interpret_object_message.h"

namespace NLAIAGENT
{
	CNotifyParentScript::CNotifyParentScript( std::list<IObjectIA *> &l, NLAISCRIPT::CMessageClass *b):CMessageScript(l,b)
	{
		/*set(0,new DigitalType(0.0));
		set(1,new CLocalAgentMail(NULL));*/
	}

	CNotifyParentScript::CNotifyParentScript(NLAISCRIPT::CMessageClass *b):CMessageScript(b)
	{		
		/*CVectorGroupType *x = new CVectorGroupType(2);		
		setMessageGroup(x);
		setGroup(CMessageGroup::msgScriptingGroup);		
		set(0,new DigitalType(0.0));
		set(1,new CLocalAgentMail(NULL));*/
	}

	CNotifyParentScript::CNotifyParentScript(IBasicAgent *agent):
			CMessageScript((NLAISCRIPT::CMessageClass *)NLAISCRIPT::CMsgNotifyParentClass::IdMsgNotifyParentClass.getFactory()->getClass())
	{		
		/*CVectorGroupType *x = new CVectorGroupType(2);
		setMessageGroup(x);
		setGroup(CMessageGroup::msgScriptingGroup);
		if(agent == NULL)
		{
			set(0,new DigitalType(0.0));
			set(1,new CLocalAgentMail(NULL));
		}
		else
		{
			set(0,new DigitalType(1.0));
			set(1,new CLocalAgentMail(agent));
		}*/
 	}

	CNotifyParentScript::CNotifyParentScript(const CNotifyParentScript &m): CMessageScript(m)
	{
	}

	const NLAIC::IBasicType *CNotifyParentScript::clone() const
	{
		/*if(((const INombreDefine *)getFront())->getNumber() != 0.0)
		{
			//CLocalAgentMail *g = (CLocalAgentMail *)get();
			return new CNotifyParentScript(*this);
		}
		else
		{
			return new CNotifyParentScript();
		}*/
		return new CNotifyParentScript(*this);
	}

	const NLAIC::CIdentType &CNotifyParentScript::getType() const
	{
		if ( getCreatorClass() ) 
			return getCreatorClass()->getType();
		else
			return IdNotifyParentScript;
	}	

	void CNotifyParentScript::getDebugString(std::string &t) const
	{
		double i = ((const INombreDefine *)getFront())->getNumber();
		if(i != 0.0)
		{
			std::string txt;
			get()->getDebugString(txt);
			t += NLAIC::stringGetBuild("CNotifyParentScript<true,%s>",txt.c_str());
		}
		else
		{
			t += "CNotifyParentScript<false,NULL>";
		}
	}
}
