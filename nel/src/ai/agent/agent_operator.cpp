/** \file agent_operator.cpp
 *
 * $Id: agent_operator.cpp,v 1.9 2003/01/21 11:24:39 chafik Exp $
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

	TQueue IObjetOp::isMember(const IVarName *className, const IVarName *name, const IObjectIA &params) const
	{
		TQueue result;

		result = IObjectIA::isMember( className, name, params );

		if ( result.size() )
			return result;

		if(className != NULL) 
		{
			if ( *name == CStringVarName(_OPPLUS_) )
			{
				result.push( NLAIAGENT::CIdMethod(op_add,0.0,NULL,NULL) );
			}
			if ( *name == CStringVarName(_OPMOINS_) )
			{
				result.push( NLAIAGENT::CIdMethod(op_sub,0.0,NULL,NULL) );
			}
			if ( *name == CStringVarName(_OPMUL_) )
			{
				result.push( NLAIAGENT::CIdMethod(op_mul,0.0,NULL,NULL) );
			}
			if ( *name == CStringVarName(_OPEQ_) )
			{
				result.push( CIdMethod(NLAIC::CTypeOfOperator::opEq,0.0,NULL,NULL) );
			}
			if ( *name == CStringVarName(_OPDIV_) )
			{
				result.push( NLAIAGENT::CIdMethod(op_div,0.0,NULL,NULL) );
			}

			if ( *name == CStringVarName(_OPLESS_) )
			{
				result.push( NLAIAGENT::CIdMethod(op_neg,0.0,NULL,NULL) );
			}

			if ( *name == CStringVarName(_OPEQ_) )
			{
				result.push( NLAIAGENT::CIdMethod(op_eq,0.0,NULL,NULL) );
			}

			if ( *name == CStringVarName(_OPINFEQ_) )
			{
				result.push( NLAIAGENT::CIdMethod(op_inf_eq,0.0,NULL,NULL) );
			}

			if ( *name == CStringVarName(_OPSUPEQ_) )
			{
				result.push( NLAIAGENT::CIdMethod(op_sup_eq,0.0,NULL,NULL) );
			}

			if ( *name == CStringVarName(_OPDIFF_) )
			{
				result.push( NLAIAGENT::CIdMethod(op_diff,0.0,NULL,NULL) );
			}
		}
		
		return result;
	}

	sint32 IObjetOp::getMethodIndexSize() const
	{
		return IObjectIA::getMethodIndexSize() + op_last;
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
			case op_add:
				r.Result = (IObjectIA *) &( (*this) += ( * (IObjetOp *) x) );
				break;

			case op_sub:
				r.Result = (IObjectIA *) &( (*this) -= ( * (IObjetOp *) x) );
				break;

			case op_mul:
				r.Result = (IObjectIA *) &( (*this) *= ( * (IObjetOp *) x) );
				break;

			case op_div:
				r.Result = (IObjectIA *) &( (*this) /= ( * (IObjetOp *) x) );
				break;

			case op_not:
				r.Result = (IObjectIA *) ( !(*this)  );
				break;

			case op_eq:
				r.Result = (IObjectIA *) ( (*this) == ( * (IObjetOp *) x ) );
				break;

			case NLAIC::CTypeOfOperator::opInfEq	:
				r.Result = (IObjectIA *) ( (*this) <= ( * (IObjetOp *) x ) );
				break;

			case NLAIC::CTypeOfOperator::opSupEq	:
				r.Result = (IObjectIA *) ( (*this) >= ( * (IObjetOp *) x ) );
				break;

			case NLAIC::CTypeOfOperator::opDiff	:
				r.Result = (IObjectIA *) ( (*this) != ( * (IObjetOp *) x ) );
				break;
		}
		return r;
	}
}
