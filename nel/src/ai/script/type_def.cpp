/** \file type_def.cpp
 *
 * $Id: type_def.cpp,v 1.13 2003/01/21 11:24:39 chafik Exp $
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
#include "nel/ai/script/constraint.h"
#include "nel/ai/script/type_def.h"

namespace NLAISCRIPT
{
	IOpType::~IOpType()
	{
	}	
	
	double IOpType::evalParam(IOpType *e)
	{
		double d;
		if(e->getConstraintTypeOf() != NULL)
		{
			const NLAIC::CIdentType &idG = *getConstraintTypeOf();
			const NLAIC::CIdentType &idD = *e->getConstraintTypeOf();
			if(!(idG == idD))
			{
				if(((const NLAIC::CTypeOfObject &)idD) & NLAIC::CTypeOfObject::tInterpret )
				{
					IClassInterpret *o = (IClassInterpret *)((CClassInterpretFactory *)idD.getFactory())->getClass();						
					bool type = false;						
					d = 0.0;
					while(o != NULL)
					{								
						if( o->getType() == idG)
						{
							d += 1.0;
							type = true;
							break;
						}
						o = (IClassInterpret *)o->getComputeBaseClass();						
					}
					if(!type) return -1.0;
					else return d;
				}
				else
				{
					//NLAIC::CTypeOfObject o_t(tNombre | TString | tList | tLogic);
					if(((const NLAIC::CTypeOfObject &)idD) & ((const NLAIC::CTypeOfObject &)idG))
					{
						return 0.0;
					}
					else return -1.0;
				}
			}
			else return 0.0;
		}
		else return -1.0;		
	}

	double IOpType::eval(IOpType *e)
	{
		if(getConstraintTypeOf() != NULL && e->getConstraintTypeOf() != NULL)
		{
			if(e->getTypeOfClass() == operandSimpleListOr)
			{
				COperandSimpleListOr *l = (COperandSimpleListOr *)e;
				std::list<NLAIC::CIdentType *>::const_iterator i = l->getList().begin();
				while(i != l->getList().end() )
				{
					COperandSimple *x =new COperandSimple (new NLAIC::CIdentType(*(*i++)));
					double d = evalParam(x);
					delete x;
					if(d >= 0.0) return d;					
				}

			}
			else return evalParam(e);
		}
		return -1.0;
	}

	bool COperationTypeGD::satisfied()
	{			
		if(_OpG->satisfied() && _OpD->satisfied())
		{				
			if(_Op == NLAIC::CTypeOfOperator::opAff)
			{
				if(*_OpG->getConstraintTypeOf() == *_OpD->getConstraintTypeOf()) return true;
				else
				{
					if(((const NLAIC::CTypeOfObject &)*_OpD->getConstraintTypeOf()) & NLAIC::CTypeOfObject::tInterpret)
					{
						const IClassInterpret *o = (const IClassInterpret *)((CClassInterpretFactory *)_OpD->getConstraintTypeOf()->getFactory())->getClass();
						for(sint32 i = o->sizeVTable() - 1; i >= 1; i--)
						{							
							if( o->getInheritance(i)->getType() == *(_OpG->getConstraintTypeOf())) return true;
						}
					}
					else return false;
				}
			}
			else
			{				
				const NLAIC::CIdentType *id = _OpG->getConstraintTypeOf();
				if( (( uint32)(((const NLAIC::CTypeOfOperator &)*id) & _Op)) != (uint32)_Op) return false;
				id = _OpD->getConstraintTypeOf();
				if( ((uint32)(((const NLAIC::CTypeOfOperator &)*id) & _Op)) != (uint32)_Op) return false;
			}
			return true;
		}
		return false;
	}


	COperandSimpleListOr::COperandSimpleListOr(int count, ...)
	{
		va_list marker;
		
		va_start( marker, count );
		while(count --)
		{
			NLAIC::CIdentType *o = va_arg( marker, NLAIC::CIdentType *);
			_TypeListe.push_back(o);
		}
		std::string text;
		text = NLAIC::stringGetBuild("constraint<COperandSimpleListOr> for ...");
		_TxtInfo = new char [strlen(text.c_str()) + 1];
		strcpy(_TxtInfo,text.c_str());
	}

	double COperandSimpleListOr::eval (IOpType *e)
	{
		std::list<NLAIC::CIdentType *>::const_iterator i = getList().begin();
		while(i != getList().end() )
		{
			//NLAIC::CIdentType id(*(*i++));
			COperandSimple x(*i);
			(*i)->incRef();
			double d = x.eval(e);
			i ++;
			if(d >= 0.0) return d;					
		}
		return -1.0;
	}
}
