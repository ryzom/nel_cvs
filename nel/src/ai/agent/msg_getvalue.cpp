/** \file msg_getvalue.cpp
 *
 * $Id: msg_getvalue.cpp,v 1.3 2003/01/21 11:24:39 chafik Exp $
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


#ifndef NL_MSGGETVALUE_H
#define NL_MSGGETVALUE_H

#include "nel/ai/agent/msg_getvalue.h"
#include "nel/ai/agent/agent_digital.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/codage.h"
#include "nel/ai/script/object_unknown.h"
#include "nel/ai/script/interpret_message_getvalue.h"

namespace NLAIAGENT
{
	CGetValueMsg::CGetValueMsg( std::list<IObjectIA *> &l, NLAISCRIPT::CMessageClass *b):CMessageScript(l,b)
	{
		set(0, new DigitalType(0) );
	}

	CGetValueMsg::CGetValueMsg(NLAISCRIPT::CMessageClass *b) : CMessageScript(b)
	{		
		CVectorGroupType *x = new CVectorGroupType(1);		
		setMessageGroup(x);		
		set(0, new DigitalType(0) );
	}

	CGetValueMsg::CGetValueMsg(IBasicAgent *agent):
			CMessageScript((NLAISCRIPT::CMessageClass *)NLAISCRIPT::CGetValueMsgClass::IdGetValueMsgClass.getFactory()->getClass())
	{		
		CVectorGroupType *x = new CVectorGroupType(1);
		setMessageGroup(x);		
		set(0, new DigitalType(0) );
 	}

	CGetValueMsg::CGetValueMsg(const CGetValueMsg &m): CMessageScript(m)
	{
	}

	CGetValueMsg::~CGetValueMsg()
	{
		
	}

	const NLAIC::IBasicType *CGetValueMsg::clone() const
	{
		const NLAIC::IBasicType *x;
		x = new CGetValueMsg(*this);
		return x;
	}

	const NLAIC::CIdentType &CGetValueMsg::getType() const
	{
		if ( getCreatorClass() ) 
			return getCreatorClass()->getType();
		else
			return NLAISCRIPT::CGetValueMsgClass::IdGetValueMsgClass;
	}	

	void CGetValueMsg::getDebugString(std::string &t) const
	{
		const CStringType *c = (const CStringType *)getFront();
		std::string txt;
		c->getDebugString(txt);
		t += NLAIC::stringGetBuild("CGetValueMsg<%s>",txt.c_str());
	}


	TQueue CGetValueMsg::isMember(const IVarName *className,const IVarName *funcName,const IObjectIA &params) const
	{

		TQueue r;

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

	NLAIAGENT::IObjectIA::CProcessResult CGetValueMsg::runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *)
	{
		return IObjectIA::CProcessResult();
	}

	IObjectIA::CProcessResult CGetValueMsg::runMethodeMember(sint32 index, IObjectIA *context)
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

	sint32 CGetValueMsg::getBaseMethodCount() const
	{
		return CMessageScript::getBaseMethodCount() + 1;
	}


}	// NLAIAGENT

#endif // NL_MSGGETVALUE_H
