/** \file agent_object.cpp
 *
 * $Id: agent_object.cpp,v 1.8 2002/05/06 12:55:23 robert Exp $
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

#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/agent_object.h"
#include "nel/ai/logic/boolval.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/interpret_methodes.h"
#include "nel/ai/agent/agent_method_def.h"

namespace NLAIAGENT
{		
	const NLAIC::CIdentType &CStringType::getType() const
	{		
		return IdStringType;
	}

	IObjetOp &CStringType::operator += (const IObjetOp &a)
	{
		const IVarName &b = (const IVarName &)a;
		*_Str += b;		
		return *this;
	}

	IObjetOp &CStringType::operator -= (const IObjetOp &a)
	{
		const IVarName &b = (const IVarName &)a;
		*_Str -= b;
		return *this;
	}
	

	IObjetOp *CStringType::operator < (IObjetOp &a) const
	{		
		const IVarName &b = (const IVarName &)a;
		NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(*_Str < b);
		return x;
	}

	IObjetOp *CStringType::operator > (IObjetOp &a) const
	{		
		const IVarName &b = (const IVarName &)a;		
		NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(*_Str > b);
		return x;
	}

	IObjetOp *CStringType::operator <= (IObjetOp &a) const
	{
		const IVarName &b = (const IVarName &)a;		
		NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(!(*_Str > b));
		return x;
	}

	IObjetOp *CStringType::operator >= (IObjetOp &a) const
	{		
		const IVarName &b = (const IVarName &)a;		
		NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(!(*_Str < b));
		return x;
	}
	
	IObjetOp *CStringType::operator != (IObjetOp &a) const
	{
		const CStringType &b = (const CStringType &)a;		
		NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(!(*_Str == b.getStr()));
		return x;
	}

	IObjetOp *CStringType::operator == (IObjetOp &a) const
	{
		const CStringType &b = (const CStringType &)a;		
		NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType((*_Str == b.getStr()));
		return x;
	}

	bool CStringType::operator < (const CStringType &a) const
	{
		return *_Str < *a._Str;
	}

	const IObjectIA::CProcessResult &CStringType::run()
	{
		return IObjectIA::ProcessRun;
	}


	const static sint32 _Const = 0;
	const static sint32 _First = 1;
	const static sint32 _Second = 2;	
	const static sint32 _LastM = 3;

	CPaireType::CMethodCall CPaireType::_Method[] = 
	{
		CPaireType::CMethodCall(_CONSTRUCTOR_,NLAIAGENT::_Const),
		CPaireType::CMethodCall(_FIRST_, NLAIAGENT::_First),
		CPaireType::CMethodCall(_SECOND_, NLAIAGENT::_Second)		
	};

	sint32 CPaireType::getMethodIndexSize() const
	{
		return IObjetOp::getMethodIndexSize() + _LastM;
	}

	tQueue CPaireType::isMember(const IVarName *className,const IVarName *methodName,const IObjectIA &p) const
	{
		tQueue a;
		NLAISCRIPT::CParam methodParam;
		NLAISCRIPT::CParam &param = (NLAISCRIPT::CParam &)p;
		
		if(className == NULL)
		{
			for(int i = 0; i < _LastM; i++)
			{
				if(*methodName == IBaseGroupType::_Method[i].MethodName)
				{										
					switch(_Method[i].Index)
					{
					case NLAIAGENT::_Const:					
					case NLAIAGENT::_First:
					case NLAIAGENT::_Second:						
					default:						
						return a;
					}
				}
			}
		}
		return IObjetOp::isMember(className,methodName,p);
	}

	IObjectIA::CProcessResult CPaireType::runMethodeMember(sint32, sint32, IObjectIA *)
	{
		return IObjectIA::CProcessResult();
	}
	IObjectIA::CProcessResult CPaireType::runMethodeMember(sint32 index,IObjectIA *p)
	{
		IBaseGroupType *param = (IBaseGroupType *)p;

		/*switch(index - IObjetOp::getMethodIndexSize())
		{
		case _Const:
			return IObjectIA::CProcessResult();
		case _Push:	
			{
				CIteratorContener i = param->getIterator();
				while(!i.isInEnd())
				{
					IObjectIA *a = (IObjectIA *)i++;
					a->incRef();
					push(a);
				}	
			}
			return IObjectIA::CProcessResult();
		case _PushFront:
			{
				CIteratorContener i = param->getIterator();
				while(!i.isInEnd())
				{
					IObjectIA *a = (IObjectIA *)i++;
					a->incRef();
					pushFront(a);
				}	
			}
			return IObjectIA::CProcessResult();
			
		
		case _Pop:
			{
				IObjectIA::CProcessResult c;
				IObjectIA *a = (IObjectIA *)pop();				
				c.Result = a;
				c.ResultState = IObjectIA::ProcessIdle;
				return c;
			}

		case _PopFront:	
			{
				IObjectIA::CProcessResult c;
				IObjectIA *a = (IObjectIA *)popFront();				
				c.Result = a;
				c.ResultState = IObjectIA::ProcessIdle;
				return c;
			}

		case _Back:
			{
				IObjectIA::CProcessResult c;
				IObjectIA *a = (IObjectIA *)get();
				a->incRef();
				c.Result = a;
				c.ResultState = IObjectIA::ProcessIdle;
				return c;
			}

		case _Front:	
			{
				IObjectIA::CProcessResult c;
				IObjectIA *a = (IObjectIA *)getFront();
				a->incRef();
				c.Result = a;
				c.ResultState = IObjectIA::ProcessIdle;
				return c;
			}

		case _Get:
			{
				IObjectIA::CProcessResult c;
				const INombreDefine *f = (const INombreDefine *)param->get();
				IObjectIA *a = (IObjectIA *)(*this)[(sint32)f->getNumber()];
				a->incRef();
				c.Result = a;
				c.ResultState = IObjectIA::ProcessIdle;
				return c;
			}

		case _Set:
			{
				IObjectIA::CProcessResult c;
				CIteratorContener i = param->getIterator();
				const DigitalType *f = (const DigitalType *)i ++;
				IObjectIA *n = (IObjectIA *)i++;				
				

				set((sint32)f->getValue(),n);
				n->incRef();
				return IObjectIA::CProcessResult();
			}
		case _Size:
			{
				DigitalType *f = new DigitalType((float)size());
				IObjectIA::CProcessResult c;
				c.Result = f;
				c.ResultState = IObjectIA::ProcessIdle;
				return c;
			}
		}*/

		return IObjectIA::runMethodeMember(index,p);
	}
}
