/** \file constraint_stack_component.cpp
 *
 * $Id: constraint_stack_component.cpp,v 1.2 2001/01/08 10:48:01 chafik Exp $
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
#include "script/type_def.h"
#include "script/constraint_stack_component.h"

namespace NLIASCRIPT
{

	CConstraintStackComp::CConstraintStackComp(OpCodeType opCodeType,sint32 stackPos,const std::list<NLIASCRIPT::CStringType > &memberName,IOpType *typeStack,sint32 lign,sint32 col):
	_MemberName(memberName),_TypeStack(typeStack),_Lin(lign),_Col(col)
	{
		char txt[1028*24];
		char m[1028*8];
		std::list<NLIASCRIPT::CStringType >::const_iterator i = _MemberName.begin();
		m[0] = 0;
		while(i != _MemberName.end())
		{
			const NLIASCRIPT::CStringType &s = *i;
			strcat(m,s.data());
			i ++;
			if(i != _MemberName.end()) strcat(m,".");
		}
		sprintf(txt,"resolve constraint for the component '%s' at line %d",m,_Lin);
		_TxtInfo = new char [strlen(txt) + 1];
		strcpy(_TxtInfo,txt);		
		_Satisfied = false;
		_Id = NULL;
		_OpCodeType = opCodeType;
		_StackPos = stackPos;
	}

	CConstraintStackComp::~CConstraintStackComp()
	{
		_TypeStack->release();
		if(_Id) delete _Id;
		delete _TxtInfo;
	}

	void CConstraintStackComp::run(CCompilateur &comp)
	{
		if(_Satisfied) return;
		_TypeStack->run(comp);
		if(_TypeStack->satisfied())
		{
			if(_MemberName.size() == 1)
			{
				const NLAIAGENT::IObjectIA *c = (NLAIAGENT::IObjectIA *)((NLAIC::IClassCFactory *)_TypeStack->getConstraintTypeOf()->getFactory())->getClass();
				sint32 i = c->getStaticMemberIndex(NLAIAGENT::CStringVarName(_MemberName.front().data()));
				if(i >= 0)
				{					
					/*if((const NLAIC::CTypeOfObject &)c->getType() & NLAIC::CTypeOfObject::tAgentInterpret) 
					{
						i += comp.getCompementShift((IClassInterpret *)c);
					}*/					

					std::list<sint32> b;
					b.push_back(i);
					IOpCode *x;
					switch(_OpCodeType)
					{
					case stackCall:
						x = new CLdbStackMemberiOpCode(b);
						break;
					case heapCall:
						x = new CLdbHeapMemberiOpCode(b,_StackPos);
						break;

					case heapAffectation:
						x = new CAffHeapMemberiOpCode(b,_StackPos);
						break;
						
					}
					x->incRef();

					std::pair<sint32, CCodeBrancheRun *> &p = _Code.back();
					IOpCode *op = (*p.second)[p.first];//
					op->release();
					(*p.second)[p.first] = x;
					_Code.pop_back();	
					_Satisfied = true;
				}
				else
				{					
					return;
				}								
			}
			else
			{
				std::list<sint32> b;
				IOpType *c;
				NLAIAGENT::IObjectIA *cl = (NLAIAGENT::IObjectIA *)((NLAIC::IClassCFactory *)_TypeStack->getConstraintTypeOf()->getFactory())->getClass();
				if(!comp.isValidateVarName(cl,b,_MemberName,c) && c->getConstraintTypeOf() != NULL)
				{
					c->release();
					return;
				}
				_Id = new NLAIC::CIdentType (*c->getConstraintTypeOf());
				c->release();
				IOpCode *x;

				switch(_OpCodeType)
				{
				case stackCall:
					x = new CLdbStackMemberiOpCode(b);
					break;
				case heapCall:
					x = new CLdbHeapMemberiOpCode(b,_StackPos);
					break;

				case heapAffectation:
					x = new CAffHeapMemberiOpCode(b,_StackPos);
					break;
				}
				x->incRef();

				std::pair<sint32, CCodeBrancheRun *> &p = _Code.back();
				IOpCode *op = (*p.second)[p.first];//
				op->release();
				(*p.second)[p.first] = x;
				_Code.pop_back();	
				_Satisfied = true;
				
			}
		}
	}

	void CConstraintStackComp::run(IConstraint *)
	{		
	}
	
	bool CConstraintStackComp::dependOn(const IConstraint *c) const
	{
		return _TypeStack == c;
	}

	bool CConstraintStackComp::operator == (const IConstraint &c) const
	{
		if((getTypeOfClass() == c.getTypeOfClass()) && *((const CConstraintStackComp &)c)._TypeStack == *_TypeStack)
		{
			return	((const CConstraintStackComp &)c)._MemberName == _MemberName && 
					((const CConstraintStackComp &)c)._OpCodeType == _OpCodeType && 
					((const CConstraintStackComp &)c)._StackPos == _StackPos;
		}
		return false;
	}

	void CConstraintStackComp::getError(char *txt) const
	{
		sprintf(txt,"can't %s",_TxtInfo);
	}

	const IConstraint *CConstraintStackComp::clone() const		
	{
		_TypeStack->incRef();
		IConstraint *x = new CConstraintStackComp(_OpCodeType,0,_MemberName,_TypeStack,_Lin,_Col);
		x->incRef();
		return x;
	}

	const NLAIC::CIdentType *CConstraintStackComp::getConstraintTypeOf()
	{
		return _Id;
	}
}