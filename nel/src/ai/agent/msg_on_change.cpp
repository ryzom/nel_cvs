/** \file msg_on_change.cpp
 *
 * $Id: msg_on_change.cpp,v 1.4 2002/05/27 09:23:04 chafik Exp $
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

#include "nel/ai/agent/msg_on_change.h"
#include "nel/ai/agent/agent_digital.h"
#include "nel/ai/script/interpret_object_message.h"
//#include "nel/ai/logic/goal.h"
#include "nel/ai/agent/object_type.h"

namespace NLAIAGENT
{
	COnChangeMsg::COnChangeMsg( std::list<IObjectIA *> &l, NLAISCRIPT::CMessageClass *b):CMessageScript(l,b)
	{
	}

	COnChangeMsg::COnChangeMsg(NLAISCRIPT::CMessageClass *b):CMessageScript(b)
	{		
		/*CVectorGroupType *x = new CVectorGroupType();		
		setMessageGroup(x);*/
		setGroup(CMessageGroup::msgScriptingGroup);		
	}

	COnChangeMsg::COnChangeMsg(IBasicAgent *agent):
			CMessageScript((NLAISCRIPT::CMessageClass *)NLAISCRIPT::COnChangeMsgClass::IdOnChangeMsgClass.getFactory()->getClass())
	{		
		/*CVectorGroupType *x = new CVectorGroupType();		
		setMessageGroup(x);*/
		setGroup(CMessageGroup::msgScriptingGroup);
 	}

	COnChangeMsg::COnChangeMsg(const COnChangeMsg &m): CMessageScript(m)
	{
	}

	const NLAIC::IBasicType *COnChangeMsg::clone() const
	{
		const NLAIC::IBasicType *x;
		if(((const INombreDefine *)getFront())->getNumber() != 0.0)
		{
			//CLocalAgentMail *g = (CLocalAgentMail *)get();
			x = new COnChangeMsg(*this);
		}
		else
		{
			x = new COnChangeMsg();
		}

		return x;
	}

	const NLAIC::CIdentType &COnChangeMsg::getType() const
	{
		if ( getCreatorClass() ) 
			return getCreatorClass()->getType();
		else
			return IdOnChangeMsg;
	}	

	void COnChangeMsg::getDebugString(std::string &t) const
	{
		double i = ((const INombreDefine *)getFront())->getNumber();
		if(i != 0.0)
		{
			std::string txt;
			get()->getDebugString(txt);
			t += NLAIC::stringGetBuild("COnChangeMsg<true,%s>",txt.c_str());
		}
		else
		{
			t = "COnChangeMsg<false,NULL>";
		}
	}


	tQueue COnChangeMsg::isMember(const IVarName *className,const IVarName *funcName,const IObjectIA &params) const
	{
		tQueue r;
		if(className == NULL)
		{
			if( (*funcName) == CStringVarName( "Constructor" ) )
			{					
				CObjectType *c = new CObjectType( new NLAIC::CIdentType( COnChangeMsg::IdOnChangeMsg) );
				r.push( CIdMethod( IMessageBase::getMethodIndexSize(), 0.0, NULL, c) );			
			}
		}
		return r;
	}

	NLAIAGENT::IObjectIA::CProcessResult COnChangeMsg::runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *)
	{
		return IObjectIA::CProcessResult();
	}

	IObjectIA::CProcessResult COnChangeMsg::runMethodeMember(sint32 index, IObjectIA *p)
	{
		IBaseGroupType *param = (IBaseGroupType *)p;
		return IObjectIA::CProcessResult();
	}

	sint32 COnChangeMsg::getBaseMethodCount() const
	{
		return CMessageScript::getBaseMethodCount() + 1;
	}
}
