/** \file libcode.cpp
 *
 * $Id: libcode.cpp,v 1.3 2001/01/08 11:15:29 chafik Exp $
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
#include "script/compilateur.h"
#include "script/interpret_object_message.h"
#include "script/interpret_object_manager.h"
#include "script/libcode.h"
#include "script/type_def.h"
#include "script/object_unknown.h"

namespace NLAISCRIPT
{
	const NLAIC::CIdentType CCallPrint::IdCallPrint = NLAIC::CIdentType(	"Print",
																	NLAIC::CSelfClassCFactory(CCallPrint()),																
																	NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

	NLAIC::IIO *CCallPrint::inputOutput = NULL;

	NLAIAGENT::tQueue CCallPrint::isMember(const NLAIAGENT::IVarName *className,const NLAIAGENT::IVarName *mathodName,const NLAIAGENT::IObjectIA &p) const	
	{
		NLAIAGENT::CStringVarName print((const char *)IdCallPrint);
		NLAIAGENT::CStringVarName format("Format");
		NLAIAGENT::CStringVarName constructor(_CONSTRUCTOR_);
		NLAIAGENT::tQueue m;
		//const CParam &param = (const CParam &)p;
		CParam param;
		

		if(className != NULL)
		{
			if(*className == print)
			{
				if(*mathodName == format)
				{
					NLAISCRIPT::COperandSimple typeR(new NLAIC::CIdentType(NLAIAGENT::CStringType::IdStringType));
					NLAISCRIPT::CObjectUnknown *t = new NLAISCRIPT::CObjectUnknown((NLAISCRIPT::IOpType *)typeR.clone());
					t->incRef();					
					m.push(NLAIAGENT::CIdMethod(2,0.0,NULL,t));
				}
				else
				if(*mathodName == print)
				{
					NLAISCRIPT::COperandVoid typeR;
					NLAISCRIPT::CObjectUnknown *t = new NLAISCRIPT::CObjectUnknown((NLAISCRIPT::IOpType *)typeR.clone());
					t->incRef();
					m.push(NLAIAGENT::CIdMethod(0,0.0,NULL,t));
				}				
				else
				if(*mathodName == constructor && p == param)
				{
					NLAISCRIPT::COperandVoid typeR;
					NLAISCRIPT::CObjectUnknown *t = new NLAISCRIPT::CObjectUnknown((NLAISCRIPT::IOpType *)typeR.clone());
					t->incRef();
					m.push(NLAIAGENT::CIdMethod(1,0.0,NULL,t));
				}
			}
		}		
		else 
		{
			if(*mathodName == print)
			{
				NLAISCRIPT::COperandVoid typeR;
				NLAISCRIPT::CObjectUnknown *t = new NLAISCRIPT::CObjectUnknown((NLAISCRIPT::IOpType *)typeR.clone());
				t->incRef();
				m.push(NLAIAGENT::CIdMethod(0,0.0,NULL,t));
			}
			else
			if(*mathodName == constructor && p == param)
			{
				NLAISCRIPT::COperandVoid typeR;
				NLAISCRIPT::CObjectUnknown *t = new NLAISCRIPT::CObjectUnknown((NLAISCRIPT::IOpType *)typeR.clone());
				t->incRef();
				m.push(NLAIAGENT::CIdMethod(1,0.0,NULL,t));
			}
		}
		return m;
	}	

	NLAIAGENT::IObjectIA::CProcessResult CCallPrint::format(const NLAIAGENT::IBaseGroupType *g)
	{
		NLAIAGENT::CStringVarName str("");
		NLAIAGENT::CIteratorContener i = g->getIterator();
		while(!i.isInEnd())
		{
			char temp[1024*8];
			(i++)->getDebugString(temp);				
			str += NLAIAGENT::CStringVarName(temp);
			if(!i.isInEnd()) str += NLAIAGENT::CStringVarName(" ");
		}

		NLAIAGENT::CStringType s(str);		
		NLAIAGENT::IObjectIA::CProcessResult r;
		r.ResultState =  NLAIAGENT::processIdle;
		r.Result =  (NLAIAGENT::IObjectIA *)s.clone();
		return r;
	}

	NLAIAGENT::IObjectIA::CProcessResult CCallPrint::printList(const NLAIAGENT::IBaseGroupType *g)
	{
		char str[1024*32];		
		NLAIAGENT::CIteratorContener i = g->getIterator();
		strcpy(str,"");
		while(!i.isInEnd())
		{
			char temp[1024*24];
			(i++)->getDebugString(temp);				
			strcat(str,temp);
			if(!i.isInEnd()) strcat(str," ");
		}

		NLAIAGENT::IObjectIA::CProcessResult r;
		inputOutput->Echo("%s\n",str);		
		r.ResultState =  NLAIAGENT::processIdle;
		r.Result =  NULL;
		return r;
	}

	NLAIAGENT::IObjectIA::CProcessResult CCallPrint::runMethodeMember(sint32 heritance, sint32 index, NLAIAGENT::IObjectIA *param)
	{
		switch(heritance)
		{
		case 1:
			return runMethodeMember(index,param);
			break;
		}
		NLAIAGENT::IObjectIA::CProcessResult r;
		r.ResultState =  NLAIAGENT::processIdle;
		r.Result =  NULL;
		return r;
	}

	NLAIAGENT::IObjectIA::CProcessResult CCallPrint::runMethodeMember(sint32 index,NLAIAGENT::IObjectIA *param)
	{
		NLAIAGENT::IObjectIA::CProcessResult r;
		r.ResultState =  NLAIAGENT::processIdle;
		r.Result =  NULL;
		switch(index)
		{
		case 0:
			return printList((const NLAIAGENT::IBaseGroupType *)param);
		case 1:			
			return r;
		case 2:			
			return format((const NLAIAGENT::IBaseGroupType *)param);
		}
		return r;
	}
	

	void initExternalLib()
	{		
		CCallPrint p;

		NLAIC::CSelfClassCFactory &f = (NLAIC::CSelfClassCFactory&)*((CAgentClass::IdAgentClass).getFactory());
		f = NLAISCRIPT::CAgentClass(CAgentClass::IdAgentClass);
		((CAgentClass *)f.getClass())->setClassName(NLAIAGENT::CStringVarName((const char *)CAgentClass::IdAgentClass));

		NLAIC::CSelfClassCFactory &fi = (NLAIC::CSelfClassCFactory&)*((CMessageClass::IdMessageClass).getFactory());
		fi = CMessageClass(CMessageClass::IdMessageClass);
		((CMessageClass *)fi.getClass())->setClassName(NLAIAGENT::CStringVarName((const char *)CMessageClass::IdMessageClass));

		NLAIC::CSelfClassCFactory &fa = (NLAIC::CSelfClassCFactory&)*((CManagerClass::IdManagerClass).getFactory());
		fa = CManagerClass(CManagerClass::IdManagerClass);
		((CManagerClass *)fa.getClass())->setClassName(NLAIAGENT::CStringVarName((const char *)CManagerClass::IdManagerClass));
		NLAIAGENT::DigitalType::NullOperator.incRef();
	}
}