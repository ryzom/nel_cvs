/** \file agent_operator.cpp
 *
 * $Id: agent_operator.cpp,v 1.5 2001/01/12 11:49:58 portier Exp $
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

#include "nel/ai/agent/agentexport.h"
#include "nel/ai/agent/agent_operator.h"
#include "nel/ai/agent/agent_method_def.h"

namespace NLAIAGENT
{

	tQueue IObjetOp::isMember(const IVarName *className, const IVarName *name, const IObjectIA &params) const
	{
		tQueue result;

		result = IObjectIA::isMember( className, name, params );

		if ( result.size() )
			return result;

		if(className != NULL) 
		{
			if ( *name == CStringVarName(_OPPLUS_) )
			{
				result.push( CIdMethod(NLAIC::CTypeOfOperator::opAdd,0.0,NULL,NULL) );
			}
			if ( *name == CStringVarName(_OPMOINS_) )
			{
				result.push( CIdMethod(NLAIC::CTypeOfOperator::opSub,0.0,NULL,NULL) );
			}
			if ( *name == CStringVarName(_OPMUL_) )
			{
				result.push( CIdMethod(NLAIC::CTypeOfOperator::opMul,0.0,NULL,NULL) );
			}
			if ( *name == CStringVarName(_OPEQ_) )
			{
				result.push( CIdMethod(NLAIC::CTypeOfOperator::opDiv,0.0,NULL,NULL) );
			}
			if ( *name == CStringVarName(_OPLESS_) )
			{
				result.push( CIdMethod(NLAIC::CTypeOfOperator::opNeg,0.0,NULL,NULL) );
			}
			if ( *name == CStringVarName(_OPINFEQ_) )
			{
				result.push( CIdMethod(NLAIC::CTypeOfOperator::opInfEq,0.0,NULL,NULL) );
			}
			if ( *name == CStringVarName(_OPSUPEQ_) )
			{
				result.push( CIdMethod(NLAIC::CTypeOfOperator::opSupEq,0.0,NULL,NULL) );
			}
			if ( *name == CStringVarName(_OPDIFF_) )
			{
				result.push( CIdMethod(NLAIC::CTypeOfOperator::opDiff,0.0,NULL,NULL) );
			}
		}
		
		return result;
	}

	sint32 IObjetOp::getMethodIndexSize() const
	{
		return IObjectIA::getMethodIndexSize() + NLAIC::CTypeOfOperator::opAff;
	}

	// Executes a method from its index id and with its parameters
	IObjectIA::CProcessResult IObjetOp::runMethodeMember(sint32 index, IObjectIA *param)
	{

		if ( index <= IObjectIA::getMethodIndexSize() )
			return IObjectIA::runMethodeMember(index, param);

		IObjetOp *x = (IObjetOp *)( (IBaseGroupType *) param)->getFront();
		( (IBaseGroupType *) param)->popFront();

		IObjectIA::CProcessResult r;
		r.ResultState =  processIdle;

		switch ( index ) 
		{
			case NLAIC::CTypeOfOperator::opAdd:
				r.Result = (IObjectIA *) &( (*this) += ( * (IObjetOp *) x) );
				break;

			case NLAIC::CTypeOfOperator::opSub:
				r.Result = (IObjectIA *) &( (*this) -= ( * (IObjetOp *) x) );
				break;

			case NLAIC::CTypeOfOperator::opMul:
				r.Result = (IObjectIA *) &( (*this) *= ( * (IObjetOp *) x) );
				break;

			case NLAIC::CTypeOfOperator::opDiv:
				r.Result = (IObjectIA *) &( (*this) /= ( * (IObjetOp *) x) );
				break;

			case NLAIC::CTypeOfOperator::opNot:
				r.Result = (IObjectIA *) ( !(*this)  );
				break;

			case NLAIC::CTypeOfOperator::opEq:
				r.Result = (IObjectIA *) ( (*this) == ( * (IObjetOp *) x ) );
				break;
		}
		return r;
	}
}
