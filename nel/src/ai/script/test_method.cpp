/** \file libcode.cpp
 *
 * $Id: test_method.cpp,v 1.1 2001/01/29 11:11:42 chafik Exp $
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
#include "nel/ai/script/test_method.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/script/object_unknown.h"


namespace NLAISCRIPT
{
	const NLAIC::CIdentType CLibTest::IdLibTest = NLAIC::CIdentType("Check",
																	NLAIC::CSelfClassCFactory(CLibTest()),
																	NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

	const char *IsNULL = "IsNull";
	CLibTest::CMethodCall CLibTest::StaticMethod[] = 
	{
		CLibTest::CMethodCall(	IsNULL, 
								CLibTest::TIsNULL, NULL,
								CLibTest::CheckCount,
								1,
								new CObjectUnknown(new COperandSimple(new NLAIC::CIdentType(NLAIAGENT::DigitalType::IdDigitalType))))
	};

	sint32 CLibTest::isClassInheritedFrom(const NLAIAGENT::IVarName &className) const
	{
		NLAIAGENT::CStringVarName check((const char *)IdLibTest);

		if(className == check)
		{
			return 0;
		}
		else return -1;		
	}

	NLAIAGENT::tQueue CLibTest::isMember(const NLAIAGENT::IVarName *className,const NLAIAGENT::IVarName *methodName,const NLAIAGENT::IObjectIA &param) const
	{
		NLAIAGENT::tQueue r;
		NLAIAGENT::CStringVarName check((const char *)IdLibTest);

		if(className == NULL || *className == check)
		{
			sint i;
			CLibTest::TMethodNumDef index;
			for(i = 0; i < CLibTest::TLastM; i ++)
			{
				if(CLibTest::StaticMethod[i].MethodName == *methodName)
				{
					index = (CLibTest::TMethodNumDef)CLibTest::StaticMethod[i].Index;
					switch(CLibTest::StaticMethod[i].CheckArgType)
					{
						case CLibTest::CheckAll:
						{
							double d = ((NLAISCRIPT::CParam &)*CLibTest::StaticMethod[i].ArgType).eval((NLAISCRIPT::CParam &)param);
							if(d >= 0.0)
							{																
								CLibTest::StaticMethod[i].ReturnValue->incRef();
								r.push(NLAIAGENT::CIdMethod((CLibTest::getMethodIndexSize() + CLibTest::StaticMethod[i].Index),
															0.0,
															NULL,
															CLibTest::StaticMethod[i].ReturnValue));
								return r;
							}
						}
						break;
						case CLibTest::CheckCount:
						{
							if(((NLAISCRIPT::CParam &)param).size() == CLibTest::StaticMethod[i].ArgCount)
							{																
								CLibTest::StaticMethod[i].ReturnValue->incRef();
								r.push(NLAIAGENT::CIdMethod((CLibTest::getMethodIndexSize() + CLibTest::StaticMethod[i].Index),
															0.0,
															NULL,
															CLibTest::StaticMethod[i].ReturnValue ));
								return r;
							}
						}
						break;

					case CLibTest::DoNotCheck:
						{														
							CLibTest::StaticMethod[i].ReturnValue->incRef();
							r.push(NLAIAGENT::CIdMethod((CLibTest::getMethodIndexSize() + CLibTest::StaticMethod[i].Index),
														0.0,
														NULL,
														CLibTest::StaticMethod[i].ReturnValue));
							return r;						
						}
						break;					
					}
				}

			}			
		}
		return r;
	}		

	NLAIAGENT::IObjectIA::CProcessResult CLibTest::runMethodeMember(sint32 heritance, sint32 index, NLAIAGENT::IObjectIA *)
	{
		return NLAIAGENT::IObjectIA::ProcessRun;
	}

	NLAIAGENT::IObjectIA::CProcessResult CLibTest::runMethodeMember(sint32 index,NLAIAGENT::IObjectIA *p)
	{
		NLAIAGENT::IObjectIA *param = (NLAIAGENT::IObjectIA *)((NLAIAGENT::IBaseGroupType *)p)->get();
		NLAIAGENT::IObjectIA::CProcessResult r;

		if( ((const NLAIC::CTypeOfObject &)param->getType()) & NLAIC::CTypeOfObject::tNombre)
		{			
			param->incRef();
			r.Result = param;
			return r;
		}
		else
		{
			r.Result = new NLAIAGENT::DigitalType(1.0);
			return r;
		}
	}

}