/** \file digital.cpp
 *
 * $Id: digital.cpp,v 1.6 2001/01/17 10:14:52 saffray Exp $
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
#include "nel/ai/agent/agent_digital.h"

namespace NLAIAGENT
{

	// DigitalType
	DigitalType DigitalType::NullOperator = DigitalType(0.0);
	
	const NLAIC::CIdentType &DigitalType::getType() const
	{		
		return IdDigitalType;
	}
	
	const IObjectIA::CProcessResult &DigitalType::run()
	{
		return IObjectIA::ProcessRun;
	}
	
	// BorneDigitalType
	const IObjectIA::CProcessResult &BorneDigitalType::run()
	{
		return IObjectIA::ProcessRun;
	}

	const NLAIC::CIdentType &BorneDigitalType::getType() const
	{		
		return IdBorneDigitalType;
	}	

	// DDigitalType
	const NLAIC::CIdentType &DDigitalType::getType() const
	{		
		return IdDDigitalType;
	}
	
	const IObjectIA::CProcessResult &DDigitalType::run()
	{
		return IObjectIA::ProcessRun;
	}
	
	// BornedDDigitalType
	const IObjectIA::CProcessResult &BorneDDigitalType::run()
	{
		return IObjectIA::ProcessRun;
	}

	const NLAIC::CIdentType &BorneDDigitalType::getType() const
	{		
		return IdBorneDDigitalType;
	}	

	// IntegerType
	const IObjectIA::CProcessResult &IntegerType::run()
	{
		return IObjectIA::ProcessRun;
	}

	const NLAIC::CIdentType &IntegerType::getType() const
	{		
		return IdIntegerType;
	}	

	// ShortIntegerType
	const IObjectIA::CProcessResult &ShortIntegerType::run()
	{
		return IObjectIA::ProcessRun;
	}

	const NLAIC::CIdentType &ShortIntegerType::getType() const
	{		
		return IdShortIntegerType;
	}	

	// CharType
	const IObjectIA::CProcessResult &CharType::run()
	{
		return IObjectIA::ProcessRun;
	}

	const NLAIC::CIdentType &CharType::getType() const
	{		
		return IdCharType;
	}	
}
/*#######################################################################################################*/
/*################################################## VectorType #########################################*/
/*#######################################################################################################*/

#include "nel/ai/script/interpret_methodes.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/agent/object_type.h"
//non de methodes pre-definie
#include "nel/ai/agent/agent_method_def.h"

namespace NLAIAGENT
{

	sint32 IVector::getMethodIndexSize() const
	{
		return getMethodIndexSize() + 10;
	}

	tQueue IVector::isMember(const IVarName *baseClass,const IVarName *methodName,const IObjectIA &p) const
	{
		//NLAISCRIPT::CParam est le type reel de p voir nel/ai/script/interpret_methodes.h pour la declaration de la class.
		const NLAISCRIPT::CParam &param = (const NLAISCRIPT::CParam &)p;
		//Type IVarName voir agent_string.h.
		const CStringVarName methodX("x");
		const CStringVarName methodY("y");
		const CStringVarName methodZ("z");
		const CStringVarName methodSquareLength("SquareLength");
		const CStringVarName methodLength("Length");
		const CStringVarName methodUnit("Unit");
		const CStringVarName selfName((const char *)getType());
		const CStringVarName constructor(_CONSTRUCTOR_);
			
		if(baseClass == NULL || (*baseClass) == selfName)
		{
			if((*methodName) == methodX)
			{
				if(!param.size())
				{
					tQueue a;
					//Type du retour de la method x(), c'est un Float dont de type DigitalType c'est à dire DigitalType::IdDigitalType;
					//CObjectType est un IObjectIA qui permet d'encapsuler un NLAIC::CIdentType c'est à dire un type.
					NLAIAGENT::CObjectType *t = new NLAIAGENT::CObjectType(new NLAIC::CIdentType(DigitalType::IdDigitalType));
					t->incRef();
					//CIdMethod est dans basicai.h elle va être documenter.
					a.push(CIdMethod(IObjetOp::getMethodIndexSize() + 1,0.0,NULL,t));
					return a;
				}
				else
				{
					NLAISCRIPT::CParam xParam;
					xParam.push(NLAISCRIPT::COperandSimple(new NLAIC::CIdentType(DigitalType::IdDigitalType)));
					if(param == xParam)
					{		
						tQueue a;
						//Type du retour de la method x(), c'est un void dont de type VoidType c'est à dire NLAIC::CIdentType::VoidType;
						//CObjectType est un IObjectIA qui permet d'encapsuler un NLAIC::CIdentType c'est à dire un type.
						//la variable NLAIC::CIdentType::VoidType est un static dans la class NLAIC::CIdentType.
						NLAIAGENT::CObjectType *t = new NLAIAGENT::CObjectType(new NLAIC::CIdentType(NLAIC::CIdentType::VoidType));
						t->incRef();
						//CIdMethod est dans basicai.h elle va être documenter.
						a.push(CIdMethod(IObjetOp::getMethodIndexSize() + 2,0.0,NULL,t));
						return a;
					}
				}
			}
			else if((*methodName) == methodY)
			{
				if(!param.size())
				{
					tQueue a;
					//Type du retour de la method y(), c'est un Float dont de type DigitalType c'est à dire DigitalType::IdDigitalType;
					//CObjectType est un IObjectIA qui permet d'encapsuler un NLAIC::CIdentType c'est à dire un type.
					NLAIAGENT::CObjectType *t = new NLAIAGENT::CObjectType(new NLAIC::CIdentType(DigitalType::IdDigitalType));
					t->incRef();
					//CIdMethod est dans basicai.h elle va être documenter.
					a.push(CIdMethod(IObjetOp::getMethodIndexSize() + 3,0.0,NULL,t));
					return a;
				}
				else
				{
					NLAISCRIPT::CParam xParam;
					xParam.push(NLAISCRIPT::COperandSimple(new NLAIC::CIdentType(DigitalType::IdDigitalType)));
					if(param == xParam)
					{		
						tQueue a;
						//Type du retour de la method y(), c'est un void dont de type VoidType c'est à dire CIdentType::VoidType;
						//CObjectType est un IObjectIA qui permet d'encapsuler un NLAIC::CIdentType c'est à dire un type.
						//la variable NLAIC::CIdentType::VoidType est un static dans la class NLAIC::CIdentType.
						NLAIAGENT::CObjectType *t = new NLAIAGENT::CObjectType(new NLAIC::CIdentType(NLAIC::CIdentType::VoidType));
						t->incRef();
						//CIdMethod est dans basicai.h elle va être documenter.
						a.push(CIdMethod(IObjetOp::getMethodIndexSize() + 4,0.0,NULL,t));
						return a;
					}
				}
			}
			if((*methodName) == methodZ)
			{
				if(!param.size())
				{
					tQueue a;
					//Type du retour de la method x(), c'est un Float dont de type DigitalType c'est à dire DigitalType::IdDigitalType;
					//CObjectType est un IObjectIA qui permet d'encapsuler un NLAIC::CIdentType c'est à dire un type.
					NLAIAGENT::CObjectType *t = new NLAIAGENT::CObjectType(new NLAIC::CIdentType(DigitalType::IdDigitalType));
					t->incRef();
					//CIdMethod est dans basicai.h elle va être documenter.
					a.push(CIdMethod(IObjetOp::getMethodIndexSize() + 5,0.0,NULL,t));
					return a;
				}
				else
				{
					NLAISCRIPT::CParam xParam;
					xParam.push(NLAISCRIPT::COperandSimple(new NLAIC::CIdentType(DigitalType::IdDigitalType)));
					if(param == xParam)
					{		
						tQueue a;
						//Type du retour de la method x(), c'est un void dont de type VoidType c'est à dire NLAIC::CIdentType::VoidType;
						//CObjectType est un IObjectIA qui permet d'encapsuler un NLAIC::CIdentType c'est à dire un type.
						//la variable NLAIC::CIdentType::VoidType est un static dans la class NLAIC::CIdentType.
						NLAIAGENT::CObjectType *t = new NLAIAGENT::CObjectType(new NLAIC::CIdentType(NLAIC::CIdentType::VoidType));
						t->incRef();
						//CIdMethod est dans basicai.h elle va être documenter.
						a.push(CIdMethod(IObjetOp::getMethodIndexSize() + 6,0.0,NULL,t));
						return a;
					}
				}
			}
			else if((*methodName) == methodSquareLength)
			{
				if(!param.size())
				{
					tQueue a;
					//Return a void type (no value return)
					NLAIAGENT::CObjectType *t = new NLAIAGENT::CObjectType(new NLAIC::CIdentType(DigitalType::IdDigitalType));
					t->incRef();
					a.push(CIdMethod(IObjetOp::getMethodIndexSize() + 7,0.0,NULL,t));
					return a;
				}
			}
			else if((*methodName) == methodLength)
			{
				if(!param.size())
				{
					tQueue a;
					//Return a digital type
					NLAIAGENT::CObjectType *t = new NLAIAGENT::CObjectType(new NLAIC::CIdentType(DigitalType::IdDigitalType));
					t->incRef();
					a.push(CIdMethod(IObjetOp::getMethodIndexSize() + 8,0.0,NULL,t));
					return a;
				}
			}
			else if((*methodName) == methodUnit)
			{
				if(!param.size())
				{
					tQueue a;
					//Return a void type
					NLAIAGENT::CObjectType *t = new NLAIAGENT::CObjectType(new NLAIC::CIdentType(NLAIC::CIdentType::VoidType));
					t->incRef();
					a.push(CIdMethod(IObjetOp::getMethodIndexSize() + 9,0.0,NULL,t));
					return a;
				}
			}
			else if((*methodName) == constructor)
			{
				NLAISCRIPT::CParam constructParam;
				constructParam.push(NLAISCRIPT::COperandSimple(new NLAIC::CIdentType(DigitalType::IdDigitalType)));
				constructParam.push(NLAISCRIPT::COperandSimple(new NLAIC::CIdentType(DigitalType::IdDigitalType)));
				constructParam.push(NLAISCRIPT::COperandSimple(new NLAIC::CIdentType(DigitalType::IdDigitalType)));
				if(param == constructParam)
				{		
					tQueue a;
					//Type du retour du constructeur , c'est un void dont de type VoidType c'est à dire CIdentType::VoidType;
					//CObjectType est un IObjectIA qui permet d'encapsuler un NLAIC::CIdentType c'est à dire un type.
					//la variable NLAIC::CIdentType::VoidType est un static dans la class NLAIC::CIdentType.
					NLAIAGENT::CObjectType *t = new NLAIAGENT::CObjectType(new NLAIC::CIdentType(NLAIC::CIdentType::VoidType));
					t->incRef();
					//CIdMethod est dans basicai.h elle va être documentee.
					a.push(CIdMethod(IObjetOp::getMethodIndexSize() + 0,0.0,NULL,t));
					return a;
				}
			}
		}
		return IObjetOp::isMember(baseClass,methodName,p);
	}

	sint32 IVector::isClassInheritedFrom(const IVarName &baseClass) const
	{

/////////////Temporaire parce que la methode getInheritedSize n'est pas defini.
		//Cette class n'a aucun heritage.
		const CStringVarName selfName((const char *)getType());
		//ça c'est si jamais quelqu'un ecrit un truc du type Vector.x() où Vector est le non de cette class dans le script.
		if((baseClass) == selfName) return 0;		
		else return -1;
/////////////Normalement il aurait falut ecrire:
/*
		//Cette class n'a aucun heritage.
		const CStringVarName selfName((const char *)getType());
		//ça c'est si jamais quelqu'un ecrit un truc du type Vector.x() où Vector est le non de cette class dans le script.
		if((baseClass) == selfName) return 0 + IObjetOp::getInheritedSize();
		else return IObjetOp::isClassInheritedFrom(baseClass);
*/
	}

	IObjectIA::CProcessResult IVector::runMethodeMember(sint32 baseClassTag, sint32 methodTag, IObjectIA *p)
	{
/////////////Temporaire parce que la methode getInheritedSize n'est pas defini.
		if(baseClassTag == 0)
		{
			return runMethodeMember(methodTag,p);
		}
/////////////Normalement il aurait falut ecrire:
/*		if(baseClassTag == (0+IObjetOp::getInheritedSize()) )
		{
			return runMethodeMember(methodTag,p);
		}
*/
		return IObjectIA::runMethodeMember(baseClassTag,methodTag,p);

	}

	IObjectIA::CProcessResult IVector::runMethodeMember(sint32 methodTag,IObjectIA *p)
	{
		IBaseGroupType &param = (IBaseGroupType &)*p;

		switch(methodTag - IObjetOp::getMethodIndexSize())
		{
		case 0:
			{
				CIteratorContener i = param.getIterator();
				x(((DigitalType *)i++)->getValue());
				y(((DigitalType *)i++)->getValue());
				z(((DigitalType *)i++)->getValue());
				IObjectIA::CProcessResult a;
				a.ResultState = processIdle;
				a.Result = NULL;
				return a;
			}

		case 1:
			{				
				DigitalType *r = new DigitalType(x());
				r->incRef();
				IObjectIA::CProcessResult a;
				a.ResultState = processIdle;
				a.Result = r;
				return a;
			}						

		case 2:
			{
				x(((DigitalType *)param.get())->getValue());
				IObjectIA::CProcessResult a;
				a.ResultState = processIdle;
				a.Result = NULL;
				return a;
			}			

		case 3:
			{				
				DigitalType *r = new DigitalType(y());
				r->incRef();
				IObjectIA::CProcessResult a;
				a.ResultState = processIdle;
				a.Result = r;
				return a;
			}						

		case 4:
			{
				y(((DigitalType *)param.get())->getValue());
				IObjectIA::CProcessResult a;
				a.ResultState = processIdle;
				a.Result = NULL;
				return a;
			}			

		case 5:
			{				
				DigitalType *r = new DigitalType(z());
				r->incRef();
				IObjectIA::CProcessResult a;
				a.ResultState = processIdle;
				a.Result = r;
				return a;
			}						

		case 6:
			{
				z(((DigitalType *)param.get())->getValue());
				IObjectIA::CProcessResult a;
				a.ResultState = processIdle;
				a.Result = NULL;
				return a;
			}
		
		case 7:
			{
				DigitalType *r = new DigitalType(squareLength());
				r->incRef();
				IObjectIA::CProcessResult a;
				a.ResultState = processIdle;
				a.Result = r;
				return a;
			}

		case 8:
			{
				DigitalType *r = new DigitalType(length());
				r->incRef();
				IObjectIA::CProcessResult a;
				a.ResultState = processIdle;
				a.Result = r;
				return a;
			}

		case 9:
			{
				unit();
				IObjectIA::CProcessResult a;
				a.ResultState = processIdle;
				a.Result = NULL;
				return a;
			}

		default:
			return IObjetOp::runMethodeMember(methodTag,p);
			break;
		}
	}

	const IObjectIA::CProcessResult &VectorType::run()
	{
		return IObjectIA::ProcessRun;
	}

	const NLAIC::CIdentType &VectorType::getType() const
	{		
		return IdVectorType;
	}	
}
