/** \file libcode.cpp
 *
 * $Id: libcode.cpp,v 1.1 2001/01/05 10:53:49 chafik Exp $
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

namespace NLIASCRIPT
{
	const NLIAC::CIdentType CCallPrint::IdCallPrint = NLIAC::CIdentType(	"Print",
																	NLIAC::CSelfClassCFactory(CCallPrint()),																
																	NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

	NLIAC::IIO *CCallPrint::inputOutput = NULL;

	NLIAAGENT::tQueue CCallPrint::isMember(const NLIAAGENT::IVarName *className,const NLIAAGENT::IVarName *mathodName,const NLIAAGENT::IObjectIA &p) const	
	{
		NLIAAGENT::CStringVarName print((const char *)IdCallPrint);
		NLIAAGENT::CStringVarName format("Format");
		NLIAAGENT::CStringVarName constructor(_CONSTRUCTOR_);
		NLIAAGENT::tQueue m;
		//const CParam &param = (const CParam &)p;
		CParam param;
		

		if(className != NULL)
		{
			if(*className == print)
			{
				if(*mathodName == format)
				{
					NLIASCRIPT::COperandSimple typeR(new NLIAC::CIdentType(NLIAAGENT::CStringType::IdStringType));
					NLIASCRIPT::CObjectUnknown *t = new NLIASCRIPT::CObjectUnknown((NLIASCRIPT::IOpType *)typeR.clone());
					t->incRef();					
					m.push(NLIAAGENT::CIdMethod(2,0.0,NULL,t));
				}
				else
				if(*mathodName == print)
				{
					NLIASCRIPT::COperandVoid typeR;
					NLIASCRIPT::CObjectUnknown *t = new NLIASCRIPT::CObjectUnknown((NLIASCRIPT::IOpType *)typeR.clone());
					t->incRef();
					m.push(NLIAAGENT::CIdMethod(0,0.0,NULL,t));
				}				
				else
				if(*mathodName == constructor && p == param)
				{
					NLIASCRIPT::COperandVoid typeR;
					NLIASCRIPT::CObjectUnknown *t = new NLIASCRIPT::CObjectUnknown((NLIASCRIPT::IOpType *)typeR.clone());
					t->incRef();
					m.push(NLIAAGENT::CIdMethod(1,0.0,NULL,t));
				}
			}
		}		
		else 
		{
			if(*mathodName == print)
			{
				NLIASCRIPT::COperandVoid typeR;
				NLIASCRIPT::CObjectUnknown *t = new NLIASCRIPT::CObjectUnknown((NLIASCRIPT::IOpType *)typeR.clone());
				t->incRef();
				m.push(NLIAAGENT::CIdMethod(0,0.0,NULL,t));
			}
			else
			if(*mathodName == constructor && p == param)
			{
				NLIASCRIPT::COperandVoid typeR;
				NLIASCRIPT::CObjectUnknown *t = new NLIASCRIPT::CObjectUnknown((NLIASCRIPT::IOpType *)typeR.clone());
				t->incRef();
				m.push(NLIAAGENT::CIdMethod(1,0.0,NULL,t));
			}
		}
		return m;
	}	

	NLIAAGENT::IObjectIA::CProcessResult CCallPrint::format(const NLIAAGENT::IBaseGroupType *g)
	{
		NLIAAGENT::CStringVarName str("");
		NLIAAGENT::CIteratorContener i = g->getIterator();
		while(!i.isInEnd())
		{
			char temp[1024*8];
			(i++)->getDebugString(temp);				
			str += NLIAAGENT::CStringVarName(temp);
			if(!i.isInEnd()) str += NLIAAGENT::CStringVarName(" ");
		}

		NLIAAGENT::CStringType s(str);		
		NLIAAGENT::IObjectIA::CProcessResult r;
		r.ResultState =  NLIAAGENT::processIdle;
		r.Result =  (NLIAAGENT::IObjectIA *)s.clone();
		return r;
	}

	NLIAAGENT::IObjectIA::CProcessResult CCallPrint::printList(const NLIAAGENT::IBaseGroupType *g)
	{
		char str[1024*32];		
		NLIAAGENT::CIteratorContener i = g->getIterator();
		strcpy(str,"");
		while(!i.isInEnd())
		{
			char temp[1024*24];
			(i++)->getDebugString(temp);				
			strcat(str,temp);
			if(!i.isInEnd()) strcat(str," ");
		}

		NLIAAGENT::IObjectIA::CProcessResult r;
		inputOutput->Echo("%s\n",str);		
		r.ResultState =  NLIAAGENT::processIdle;
		r.Result =  NULL;
		return r;
	}

	NLIAAGENT::IObjectIA::CProcessResult CCallPrint::runMethodeMember(sint32 heritance, sint32 index, NLIAAGENT::IObjectIA *param)
	{
		switch(heritance)
		{
		case 1:
			return runMethodeMember(index,param);
			break;
		}
		NLIAAGENT::IObjectIA::CProcessResult r;
		r.ResultState =  NLIAAGENT::processIdle;
		r.Result =  NULL;
		return r;
	}

	NLIAAGENT::IObjectIA::CProcessResult CCallPrint::runMethodeMember(sint32 index,NLIAAGENT::IObjectIA *param)
	{
		NLIAAGENT::IObjectIA::CProcessResult r;
		r.ResultState =  NLIAAGENT::processIdle;
		r.Result =  NULL;
		switch(index)
		{
		case 0:
			return printList((const NLIAAGENT::IBaseGroupType *)param);
		case 1:			
			return r;
		case 2:			
			return format((const NLIAAGENT::IBaseGroupType *)param);
		}
		return r;
	}
	

	void initExternalLib()
	{		
		CCallPrint p;

		NLIAC::CSelfClassCFactory &f = (NLIAC::CSelfClassCFactory&)*((CAgentClass::IdAgentClass).getFactory());
		f = NLIASCRIPT::CAgentClass(CAgentClass::IdAgentClass);
		((CAgentClass *)f.getClass())->setClassName(NLIAAGENT::CStringVarName((const char *)CAgentClass::IdAgentClass));

		NLIAC::CSelfClassCFactory &fi = (NLIAC::CSelfClassCFactory&)*((CMessageClass::IdMessageClass).getFactory());
		fi = CMessageClass(CMessageClass::IdMessageClass);
		((CMessageClass *)fi.getClass())->setClassName(NLIAAGENT::CStringVarName((const char *)CMessageClass::IdMessageClass));

		NLIAC::CSelfClassCFactory &fa = (NLIAC::CSelfClassCFactory&)*((CManagerClass::IdManagerClass).getFactory());
		fa = CManagerClass(CManagerClass::IdManagerClass);
		((CManagerClass *)fa.getClass())->setClassName(NLIAAGENT::CStringVarName((const char *)CManagerClass::IdManagerClass));
		NLIAAGENT::DigitalType::NullOperator.incRef();
	}
}