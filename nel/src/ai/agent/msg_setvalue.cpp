/** \file msg_action.cpp
 *
 * $Id: msg_setvalue.cpp,v 1.1 2002/03/06 17:21:39 portier Exp $
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

#include "nel/ai/agent/msg_setvalue.h"
#include "nel/ai/agent/agent_digital.h"
#include "nel/ai/script/interpret_message_setvalue.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/codage.h"
#include "nel/ai/script/object_unknown.h"


namespace NLAIAGENT
{
	CSetValueMsg::CSetValueMsg( std::list<IObjectIA *> &l, NLAISCRIPT::CMessageClass *b):CMessageScript(l,b)
	{
		set(0, new DigitalType(0) );
	}

	CSetValueMsg::CSetValueMsg(NLAISCRIPT::CMessageClass *b):CMessageScript(b)
	{		
		CVectorGroupType *x = new CVectorGroupType(1);		
		setMessageGroup(x);
		setGroup(CMessageGroup::msgScriptingGroup);		
		set(0, new DigitalType(0) );
	}

	CSetValueMsg::CSetValueMsg(IBasicAgent *agent):
			CMessageScript((NLAISCRIPT::CMessageClass *)NLAISCRIPT::CSetValueMsgClass::IdSetValueMsgClass.getFactory()->getClass())
	{		
		CVectorGroupType *x = new CVectorGroupType(1);
		setMessageGroup(x);
		setGroup(CMessageGroup::msgScriptingGroup);
		set(0, new DigitalType(0) );
 	}

	CSetValueMsg::CSetValueMsg(const CSetValueMsg &m): CMessageScript(m)
	{
	}

	CSetValueMsg::~CSetValueMsg()
	{
		
	}

	const NLAIC::IBasicType *CSetValueMsg::clone() const
	{
		const NLAIC::IBasicType *x;
		x = new CSetValueMsg(*this);
		return x;
	}

	const NLAIC::CIdentType &CSetValueMsg::getType() const
	{
		if ( getCreatorClass() ) 
			return getCreatorClass()->getType();
		else
			return IdSetValueMsg;
	}	

	void CSetValueMsg::getDebugString(std::string &t) const
	{
		double i = ((const INombreDefine *)getFront())->getNumber();
		std::string txt;
		get()->getDebugString(txt);
		t += NLAIC::stringGetBuild("CSetValueMsg<%s>",txt.c_str());
	}


	tQueue CSetValueMsg::isMember(const IVarName *className,const IVarName *funcName,const IObjectIA &params) const
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

	NLAIAGENT::IObjectIA::CProcessResult CSetValueMsg::runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *)
	{
		return IObjectIA::CProcessResult();
	}

	IObjectIA::CProcessResult CSetValueMsg::runMethodeMember(sint32 index, IObjectIA *context)
	{
		IBaseGroupType *arg = (IBaseGroupType *) ( (NLAISCRIPT::CCodeContext *)context )->Param.back();

		switch( index - IMessageBase::getMethodIndexSize() )
		{
		case 0:
			{					
				IObjectIA *param1 = (IObjectIA *) arg->getFront();
				arg->popFront();

				IObjectIA *param2 = (IObjectIA *) arg->getFront();
				arg->popFront();

#ifdef NL_DEBUG
				std::string buffer1;
				param1->getDebugString( buffer1 );
				std::string buffer2;
				param2->getDebugString( buffer2 );

#endif
				param1->incRef();
				param2->incRef();
				set(0, param1);
				set(1, param2);
			}
			break;
		}
		return IObjectIA::CProcessResult();
	}

	sint32 CSetValueMsg::getBaseMethodCount() const
	{
		return CMessageScript::getBaseMethodCount() + 1;
	}
}
