/** \file libcode.cpp
 *
 * $Id: libcode.cpp,v 1.22 2003/01/21 11:24:39 chafik Exp $
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
#include "nel/ai/script/compilateur.h"
#include "nel/ai/script/interpret_object_message.h"
#include "nel/ai/script/interpret_object_manager.h"
#include "nel/ai/script/interpret_actor.h"
#include "nel/ai/logic/interpret_object_operator.h"
#include "nel/ai/script/libcode.h"
#include "nel/ai/script/test_method.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/script/object_unknown.h"


namespace NLAISCRIPT
{
	const NLAIC::CIdentType CCallPrint::IdCallPrint = NLAIC::CIdentType(	"Print",
																	NLAIC::CSelfClassFactory(CCallPrint()),																
																	NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

	NLAIC::IIO *CCallPrint::inputOutput = NULL;

	NLAIAGENT::TQueue CCallPrint::isMember(const NLAIAGENT::IVarName *className,const NLAIAGENT::IVarName *mathodName,const NLAIAGENT::IObjectIA &p) const	
	{
		NLAIAGENT::CStringVarName print((const char *)IdCallPrint);
		NLAIAGENT::CStringVarName format("Format");
		NLAIAGENT::CStringVarName constructor(_CONSTRUCTOR_);
		NLAIAGENT::TQueue m;
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
					m.push(NLAIAGENT::CIdMethod(2,0.0,NULL,t));
				}
				else
				if(*mathodName == print)
				{
					NLAISCRIPT::COperandVoid typeR;
					NLAISCRIPT::CObjectUnknown *t = new NLAISCRIPT::CObjectUnknown((NLAISCRIPT::IOpType *)typeR.clone());
					m.push(NLAIAGENT::CIdMethod(0,0.0,NULL,t));
				}				
				else
				if(*mathodName == constructor && p == param)
				{
					NLAISCRIPT::COperandVoid typeR;
					NLAISCRIPT::CObjectUnknown *t = new NLAISCRIPT::CObjectUnknown((NLAISCRIPT::IOpType *)typeR.clone());
					m.push(NLAIAGENT::CIdMethod(1,0.0,NULL,t));
				}
			}
		}		
		else 
		{
			if(*mathodName == format)
			{
				NLAISCRIPT::COperandSimple typeR(new NLAIC::CIdentType(NLAIAGENT::CStringType::IdStringType));
				NLAISCRIPT::CObjectUnknown *t = new NLAISCRIPT::CObjectUnknown((NLAISCRIPT::IOpType *)typeR.clone());
				m.push(NLAIAGENT::CIdMethod(2,0.0,NULL,t));
			}
			else
			if(*mathodName == print)
			{
				NLAISCRIPT::COperandVoid typeR;
				NLAISCRIPT::CObjectUnknown *t = new NLAISCRIPT::CObjectUnknown((NLAISCRIPT::IOpType *)typeR.clone());
				m.push(NLAIAGENT::CIdMethod(0,0.0,NULL,t));
			}
			else
			if(*mathodName == constructor && p == param)
			{
				NLAISCRIPT::COperandVoid typeR;
				NLAISCRIPT::CObjectUnknown *t = new NLAISCRIPT::CObjectUnknown((NLAISCRIPT::IOpType *)typeR.clone());
				m.push(NLAIAGENT::CIdMethod(1,0.0,NULL,t));
			}
		}
		return m;
	}	

	NLAIAGENT::IObjectIA::CProcessResult CCallPrint::format(const NLAIAGENT::IBaseGroupType *g)
	{
		NLAIAGENT::CStringVarName str("");
		NLAIAGENT::CConstIteratorContener i = g->getConstIterator();
		while(!i.isInEnd())
		{
			std::string temp;
			(i++)->getDebugString(temp);				
			str += NLAIAGENT::CStringVarName(temp.c_str());
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
		std::string str;		
		NLAIAGENT::CConstIteratorContener i = g->getConstIterator();
		
		while(!i.isInEnd())
		{
			std::string temp;
			(i++)->getDebugString(temp);
			str += temp.c_str();
			if(!i.isInEnd()) str += " ";
		}

		NLAIAGENT::IObjectIA::CProcessResult r;
		NLAIC::Out("%s\n",str.c_str());		

		//nlinfo("%s",str.c_str());

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
			break;
		}
		return r;
	}
		
}
