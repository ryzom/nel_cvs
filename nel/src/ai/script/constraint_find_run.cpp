/** \file constraint_find_run.cpp
 *
 * $Id: constraint_find_run.cpp,v 1.3 2001/01/08 11:15:29 chafik Exp $
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
#include "script/constraint.h"
#include "script/constraint_find_run.h"
#include "script/object_load.h"

namespace NLAISCRIPT
{	

	CConstraintFindRun::CConstraintFindRun(	CConstraintMethode::TCallTypeOpCode callType,
											int posHeap,IConstraint *baseClass,	NLAIAGENT::IBaseGroupType *methodName,CParam *param,int lin,int col):
											CConstraintMethode(searchCall,posHeap,baseClass,methodName,param,lin,col),_TypeOfCallType(callType)						
	{		
	}

	const IConstraint *CConstraintFindRun::clone() const
	{
		if(_BaseClass) _BaseClass->incRef();
		_MethodName->incRef();
		_Param->incRef();
		IConstraint *x = new CConstraintFindRun(_TypeOfCallType,_PosHeap,_BaseClass,_MethodName,_Param,_Lin,_Col);
		x->incRef();
		return x;
	}

	void CConstraintFindRun::run(CCompilateur &c)
	{		
		if(_BaseClass)
		{
			if(!_BaseClass->satisfied()) _BaseClass->run(c);
			if(_BaseClass->satisfied())
			{
				const NLAIAGENT::IObjectIA *cl = (const NLAIAGENT::IObjectIA *)(_BaseClass->getConstraintTypeOf()->getFactory())->getClass();
				if(!(( (const NLAIC::CTypeOfObject &)cl->getType() ) & NLAIC::CTypeOfObject::tAgentInterpret))
				{
					_Satisfied = true;
					_MethodName->incRef();
					_Param->incRef();
					_BaseClass->incRef();
					const CFunctionTag &m = getfunctionTag();
					ILoadObject *o;
					switch(_TypeOfCallType)
					{
					case normalCall:
						o = new CLoadStackObject(m.Member);
						o->incRef();
						break;

					case stackCall:
						o = new CLoadStackObject(m.Member);
						o->incRef();
						break;

					case heapCall:
						o = new CLoadHeapObject(m.Member,_PosHeap);
						o->incRef();
						break;

					case newCall:
						o = NULL;
						break;

					case searchCall:
						o = NULL;
						break;
						
					}
					IOpCode *x = new CFindRunMsg(_MethodName,_Param,(IOpType *)_BaseClass,o);
					x->incRef();
					setOpCode(x);
					return;
				}
			}

		}		
		CConstraintMethode::run(c);
	}		

	void CConstraintFindRun::setOpCode(CCompilateur &comp,IOpCode *,IConstraint *cType,bool del)
	{
		const CFunctionTag &m = getfunctionTag();
		IOpCode *x = new CMarkMsg(m.Inheritance, m.MethodNum);
		x->incRef();
		while(_Code.size())
		{			
			std::pair<int, CCodeBrancheRun *> &p = _Code.back();
			IOpCode *op = (*p.second)[p.first];
			op->release();
			(*p.second)[p.first] = x;
			_Code.pop_back();			
			if(_Code.size()) x->incRef();
		}
		_Type = cType;
		_Type->run(comp);
		_DelCType = del;
		runConnexcion();
	}
	void CConstraintFindRun::setOpCode(IOpCode *x)
	{
		while(_Code.size())
		{			
			std::pair<int, CCodeBrancheRun *> &p = _Code.back();
			IOpCode *op = (*p.second)[p.first];
			op->release();
			(*p.second)[p.first] = x;
			_Code.pop_back();			
			if(_Code.size()) x->incRef();
		}
		_Type = _BaseClass;
		runConnexcion();
	}
	
	CConstraintFindRun::~CConstraintFindRun()
	{									
	}
}