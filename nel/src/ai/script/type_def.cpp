/** \file type_def.cpp
 *
 * $Id: type_def.cpp,v 1.2 2001/01/08 10:48:01 chafik Exp $
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

namespace NLIASCRIPT
{
	IOpType::~IOpType()
	{
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
					if(((const NLAIC::CTypeOfObject &)*_OpD->getConstraintTypeOf()) & NLAIC::CTypeOfObject::tAgentInterpret)
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
}