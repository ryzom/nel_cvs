/** \file test_method.cpp
 *
 * $Id: test_method.cpp,v 1.16 2003/01/21 11:24:39 chafik Exp $
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
#include <time.h>
#include "nel/ai/script/compilateur.h"
#include "nel/ai/script/interpret_object_message.h"
#include "nel/ai/script/interpret_object_manager.h"
#include "nel/ai/script/interpret_actor.h"
#include "nel/ai/script/test_method.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/script/object_unknown.h"
#include "nel/ai/agent/agent_digital.h"


namespace NLAISCRIPT
{
	CLibTest test;
	const NLAIC::CIdentType CLibTest::IdLibTest = NLAIC::CIdentType("External",
																	NLAIC::CSelfClassFactory(test),
																	NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

	//const char *IsNULL = ;
	CLibTest::CMethodCall **CLibTest::StaticMethod = NULL;
	/*{
		

		CLibTest::CMethodCall(	_CONSTRUCTOR_,
								CLibTest::TConst, NULL,
								CLibTest::CheckCount,
								0,
								new CObjectUnknown(new NLAISCRIPT::COperandVoid)),

		CLibTest::CMethodCall(	"IsNotNull", 
								CLibTest::TIsNULL, NULL,
								CLibTest::CheckCount,
								1,
								new CObjectUnknown(new COperandSimple(new NLAIC::CIdentType(NLAIAGENT::DigitalType::IdDigitalType)))),

		CLibTest::CMethodCall(	"Rand", 
								CLibTest::TRand1, 
								new NLAISCRIPT::CParam(1,new NLAISCRIPT::COperandSimpleListOr(2,
															new NLAIC::CIdentType(NLAIAGENT::DDigitalType::IdDDigitalType),
															new NLAIC::CIdentType(NLAIAGENT::DigitalType::IdDigitalType))),
								CLibTest::CheckAll,
								1,
								new CObjectUnknown(new COperandSimple(new NLAIC::CIdentType(NLAIAGENT::DDigitalType::IdDDigitalType)))),

		CLibTest::CMethodCall(	"Rand", 
								CLibTest::TRand2, 
								new NLAISCRIPT::CParam(2,new NLAISCRIPT::COperandSimpleListOr(2,
															new NLAIC::CIdentType(NLAIAGENT::DDigitalType::IdDDigitalType),
															new NLAIC::CIdentType(NLAIAGENT::DigitalType::IdDigitalType)),
															new NLAISCRIPT::COperandSimpleListOr(2,
															new NLAIC::CIdentType(NLAIAGENT::DDigitalType::IdDDigitalType),
															new NLAIC::CIdentType(NLAIAGENT::DigitalType::IdDigitalType))),
								CLibTest::CheckAll,
								2,
								new CObjectUnknown(new COperandSimple(new NLAIC::CIdentType(NLAIAGENT::DDigitalType::IdDDigitalType))))
	};*/
	void CLibTest::initClass()
	{
		CLibTest::StaticMethod = new CLibTest::CMethodCall *[CLibTest::TLastM];

		CLibTest::StaticMethod[CLibTest::TConst] = new CLibTest::CMethodCall(	_CONSTRUCTOR_,
											CLibTest::TConst, NULL,
											CLibTest::CheckCount,
											0,
											new CObjectUnknown(new NLAISCRIPT::COperandVoid));

		CLibTest::StaticMethod[CLibTest::TIsNULL] = new CLibTest::CMethodCall(	
												"IsNotNull", 
												CLibTest::TIsNULL, NULL,
												CLibTest::CheckCount,
												1,
												new CObjectUnknown(new COperandSimple(new NLAIC::CIdentType(NLAIAGENT::DigitalType::IdDigitalType))));

		CLibTest::StaticMethod[CLibTest::TRand1] = new CLibTest::CMethodCall(
											"Rand", 
											CLibTest::TRand1, 
											new NLAISCRIPT::CParam(1,new NLAISCRIPT::COperandSimpleListOr(2,
																		new NLAIC::CIdentType(NLAIAGENT::DDigitalType::IdDDigitalType),
																		new NLAIC::CIdentType(NLAIAGENT::DigitalType::IdDigitalType))),
											CLibTest::CheckAll,
											1,
											new CObjectUnknown(new COperandSimple(new NLAIC::CIdentType(NLAIAGENT::DDigitalType::IdDDigitalType))));

		CLibTest::StaticMethod[CLibTest::TRand2] = new CLibTest::CMethodCall(
											"Rand", 
											CLibTest::TRand2, 
											new NLAISCRIPT::CParam(2,new NLAISCRIPT::COperandSimpleListOr(2,
																		new NLAIC::CIdentType(NLAIAGENT::DDigitalType::IdDDigitalType),
																		new NLAIC::CIdentType(NLAIAGENT::DigitalType::IdDigitalType)),
																		new NLAISCRIPT::COperandSimpleListOr(2,
																		new NLAIC::CIdentType(NLAIAGENT::DDigitalType::IdDDigitalType),
																		new NLAIC::CIdentType(NLAIAGENT::DigitalType::IdDigitalType))),
											CLibTest::CheckAll,
											2,
											new CObjectUnknown(new COperandSimple(new NLAIC::CIdentType(NLAIAGENT::DDigitalType::IdDDigitalType))));

		CLibTest::StaticMethod[CLibTest::TDiscretRand] = new CLibTest::CMethodCall(
											"DRand", 
											CLibTest::TDiscretRand, 
											new NLAISCRIPT::CParam(2,new NLAISCRIPT::COperandSimpleListOr(2,
																		new NLAIC::CIdentType(NLAIAGENT::DDigitalType::IdDDigitalType),
																		new NLAIC::CIdentType(NLAIAGENT::DigitalType::IdDigitalType)),
																		new NLAISCRIPT::COperandSimpleListOr(2,
																		new NLAIC::CIdentType(NLAIAGENT::DDigitalType::IdDDigitalType),
																		new NLAIC::CIdentType(NLAIAGENT::DigitalType::IdDigitalType))),
											CLibTest::CheckAll,
											2,
											new CObjectUnknown(new COperandSimple(new NLAIC::CIdentType(NLAIAGENT::DDigitalType::IdDDigitalType))));

	}

	void CLibTest::releaseClass()
	{
		int i;
		for(i = 0; i < CLibTest::TLastM; i++)
		{
			delete CLibTest::StaticMethod[i];
		}
		delete CLibTest::StaticMethod;
	}


	CLibTest::CLibTest()
	{	

		//srand(clock());

	}
	sint32 CLibTest::isClassInheritedFrom(const NLAIAGENT::IVarName &className) const
	{
		NLAIAGENT::CStringVarName check((const char *)IdLibTest);

		if(className == check)
		{
			return 0;
		}
		else return -1;		
	}

	NLAIAGENT::TQueue CLibTest::isMember(const NLAIAGENT::IVarName *className,const NLAIAGENT::IVarName *methodName,const NLAIAGENT::IObjectIA &param) const
	{
		NLAIAGENT::TQueue r;
		NLAIAGENT::CStringVarName check((const char *)IdLibTest);

		if(className == NULL || *className == check)
		{
			sint i;
			CLibTest::TMethodNumDef index;
			for(i = 0; i < CLibTest::TLastM; i ++)
			{
				if(CLibTest::StaticMethod[i]->MethodName == *methodName)
				{
					index = (CLibTest::TMethodNumDef)CLibTest::StaticMethod[i]->Index;
					switch(CLibTest::StaticMethod[i]->CheckArgType)
					{
						case CLibTest::CheckAll:
						{
							double d = ((NLAISCRIPT::CParam &)*CLibTest::StaticMethod[i]->ArgType).eval((NLAISCRIPT::CParam &)param);
							if(d >= 0.0)
							{																
								CLibTest::StaticMethod[i]->ReturnValue->incRef();
								r.push(NLAIAGENT::CIdMethod((CLibTest::getMethodIndexSize() + CLibTest::StaticMethod[i]->Index),
															0.0,
															NULL,
															CLibTest::StaticMethod[i]->ReturnValue));
								return r;
							}
						}
						break;
						case CLibTest::CheckCount:
						{
							if(((NLAISCRIPT::CParam &)param).size() == CLibTest::StaticMethod[i]->ArgCount)
							{																
								CLibTest::StaticMethod[i]->ReturnValue->incRef();
								r.push(NLAIAGENT::CIdMethod((CLibTest::getMethodIndexSize() + CLibTest::StaticMethod[i]->Index),
															0.0,
															NULL,
															CLibTest::StaticMethod[i]->ReturnValue ));
								return r;
							}
						}
						break;

					case CLibTest::DoNotCheck:
						{														
							CLibTest::StaticMethod[i]->ReturnValue->incRef();
							r.push(NLAIAGENT::CIdMethod((CLibTest::getMethodIndexSize() + CLibTest::StaticMethod[i]->Index),
														0.0,
														NULL,
														CLibTest::StaticMethod[i]->ReturnValue));
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
		NLAIAGENT::IObjectIA::CProcessResult r;

		switch(index)
		{
		case CLibTest::TIsNULL:
			{
				NLAIAGENT::IObjectIA *param = (NLAIAGENT::IObjectIA *)((NLAIAGENT::IBaseGroupType *)p)->get();
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
			break;
		case CLibTest::TRand1:
			{
				NLAIAGENT::DDigitalType *param = (NLAIAGENT::DDigitalType *)((NLAIAGENT::IBaseGroupType *)p)->get();
				r.Result = new NLAIAGENT::DDigitalType(rand(0.0,param->getNumber()));
				return r;
			}			

		case CLibTest::TRand2:
			{
				NLAIAGENT::CIteratorContener iter = ((NLAIAGENT::IBaseGroupType *)p)->getIterator();
				NLAIAGENT::DDigitalType *p1 = (NLAIAGENT::DDigitalType *)(iter ++);
				NLAIAGENT::DDigitalType *p2 = (NLAIAGENT::DDigitalType *)(iter ++);
				r.Result = new NLAIAGENT::DDigitalType(rand(p1->getNumber(),p2->getNumber()));
				return r;
			}

		case CLibTest::TDiscretRand:
			{
				NLAIAGENT::CIteratorContener iter = ((NLAIAGENT::IBaseGroupType *)p)->getIterator();
				NLAIAGENT::DDigitalType *p1 = (NLAIAGENT::DDigitalType *)(iter ++);
				NLAIAGENT::DDigitalType *p2 = (NLAIAGENT::DDigitalType *)(iter ++);
				r.Result = new NLAIAGENT::DDigitalType((double)dRand((sint)p1->getNumber(), (sint)p2->getNumber()));
				return r;
			}
/*
		case TIdle:
			{
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = new NLAIAGENT::DigitalType( CEntityState::Idle );
			}
			break;

		case TWalk:
			{
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = new NLAIAGENT::DigitalType( CEntityState::Idle );
			}
			break;

		case TYes:
			{
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = new NLAIAGENT::DigitalType( CEntityState::Idle );
			}
			break;

		case TNo:
			{
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = new NLAIAGENT::DigitalType( CEntityState::Idle );
			}
			break;

		case TWave:
			{
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = new NLAIAGENT::DigitalType( CEntityState::Idle );
			}
			break;

		case TBow:
			{
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = new NLAIAGENT::DigitalType( CEntityState::Idle );
			}
			break;
			*/
		}
		return r;
	}

	double CLibTest::rand(double d1, double d2) const
	{
		double r = ((double)::rand() - (((double)RAND_MAX)/2));

		r /= ((double)RAND_MAX);

		return d1 + r*(d2 - d1);

	}

	sint CLibTest::dRand(sint d1, sint d2) const
	{
		//srand((sint16)clock() );
		sint ra = ::rand();
		sint d = (d2 - d1);
		if(d < 0) d = -d;
		if(!d) d = 1;
		sint r = d1 + (ra%d);

		return r;

	}

}

