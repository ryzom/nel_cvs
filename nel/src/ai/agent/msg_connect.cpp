/** \file msg_action.cpp
 *
 * $Id: msg_connect.cpp,v 1.1 2002/03/06 17:21:39 portier Exp $
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

#include "nel/ai/agent/msg_connect.h"
#include "nel/ai/agent/agent_digital.h"
#include "nel/ai/script/interpret_message_connect.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/codage.h"
#include "nel/ai/script/object_unknown.h"


namespace NLAIAGENT
{
	CConnectObjectValueMsg::CConnectObjectValueMsg( std::list<IObjectIA *> &l, NLAISCRIPT::CMessageClass *b):CMessageScript(l,b)
	{
//		set(0, new DigitalType(0) );
	}

	CConnectObjectValueMsg::CConnectObjectValueMsg(NLAISCRIPT::CMessageClass *b):CMessageScript(b)
	{		
/*		CVectorGroupType *x = new CVectorGroupType(1);		
		setMessageGroup(x);
		setGroup(CMessageGroup::msgScriptingGroup);		
		set(0, new DigitalType(0) );
		*/
	}

	CConnectObjectValueMsg::CConnectObjectValueMsg(IBasicAgent *agent):
			CMessageScript((NLAISCRIPT::CMessageClass *)NLAISCRIPT::CConnectObjectValueMsgClass::IdConnectObjectValueMsgClass.getFactory()->getClass())
	{		
		CVectorGroupType *x = new CVectorGroupType(1);
		setMessageGroup(x);
		setGroup(CMessageGroup::msgScriptingGroup);
		set(0, new DigitalType(0) );
 	}

	CConnectObjectValueMsg::CConnectObjectValueMsg(const CConnectObjectValueMsg &m): CMessageScript(m)
	{
	}

	CConnectObjectValueMsg::~CConnectObjectValueMsg()
	{
		
	}

	const NLAIC::IBasicType *CConnectObjectValueMsg::clone() const
	{
		const NLAIC::IBasicType *x;
		x = new CConnectObjectValueMsg(*this);
		return x;
	}

	const NLAIC::CIdentType &CConnectObjectValueMsg::getType() const
	{
		if ( getCreatorClass() ) 
			return getCreatorClass()->getType();
		else
			return IdConnectObjectValueMsg;
	}	

	void CConnectObjectValueMsg::getDebugString(std::string &t) const
	{
		double i = ((const INombreDefine *)getFront())->getNumber();
		std::string txt;
		get()->getDebugString(txt);
		t += NLAIC::stringGetBuild("CConnectObjectValueMsg<%s>",txt.c_str());
	}


	tQueue CConnectObjectValueMsg::isMember(const IVarName *className,const IVarName *funcName,const IObjectIA &params) const
	{

		tQueue r;

		if(className == NULL)
		{
			if( (*funcName) == CStringVarName( "Constructor" ) )
			{					
				r.push( CIdMethod( IMessageBase::getMethodIndexSize(), 0.0, NULL, new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandVoid) ) );			
			}
		}

		if ( r.empty() )
			return CMessageScript::isMember(className, funcName, params);
		else
			return r;
	}

	NLAIAGENT::IObjectIA::CProcessResult CConnectObjectValueMsg::runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *)
	{
		return IObjectIA::CProcessResult();
	}

	IObjectIA::CProcessResult CConnectObjectValueMsg::runMethodeMember(sint32 index, IObjectIA *context)
	{
		IBaseGroupType *arg = (IBaseGroupType *) ( (NLAISCRIPT::CCodeContext *)context )->Param.back();

		switch( index - IMessageBase::getMethodIndexSize() )
		{
		case 0:
			{					
				IObjectIA *param = (IObjectIA *) arg->get();
				arg->popFront();
#ifdef NL_DEBUG
				std::string buffer;
				param->getDebugString( buffer );
#endif
				set(0, param);
			}
			break;
		}
		return IObjectIA::CProcessResult();
	}

	sint32 CConnectObjectValueMsg::getBaseMethodCount() const
	{
		return CMessageScript::getBaseMethodCount() + 1;
	}
}
