/** \file agent_operator.cpp
 *
 * $Id: agent_operator.cpp,v 1.1 2001/01/05 10:53:49 chafik Exp $
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

#include "agent/agentexport.h"
#include "agent/agent_operator.h"

namespace NLIAAGENT
{

	tQueue IObjetOp::isMember(const IVarName *className, const IVarName *name, const IObjectIA &params) const
	{
		tQueue result;

		result = IObjectIA::isMember( className, name, params );

		if ( result.size() )
			return result;

		if(className != NULL) 
		{
			if ( *name == CStringVarName("operator+") )
			{
				result.push( CIdMethod(NLIAC::CTypeOfOperator::opAdd,0.0,NULL,NULL) );
			}
			if ( *name == CStringVarName("operator-") )
			{
				result.push( CIdMethod(NLIAC::CTypeOfOperator::opSub,0.0,NULL,NULL) );
			}
			if ( *name == CStringVarName("operator*") )
			{
				result.push( CIdMethod(NLIAC::CTypeOfOperator::opMul,0.0,NULL,NULL) );
			}
			if ( *name == CStringVarName("operator==") )
			{
				result.push( CIdMethod(NLIAC::CTypeOfOperator::opDiv,0.0,NULL,NULL) );
			}
			if ( *name == CStringVarName("operator!") )
			{
				result.push( CIdMethod(NLIAC::CTypeOfOperator::opNeg,0.0,NULL,NULL) );
			}
			if ( *name == CStringVarName("operator==") )
			{
				result.push( CIdMethod(NLIAC::CTypeOfOperator::opEq,0.0,NULL,NULL) );
			}
			if ( *name == CStringVarName("operator<=") )
			{
				result.push( CIdMethod(NLIAC::CTypeOfOperator::opInfEq,0.0,NULL,NULL) );
			}
			if ( *name == CStringVarName("operator>=") )
			{
				result.push( CIdMethod(NLIAC::CTypeOfOperator::opSupEq,0.0,NULL,NULL) );
			}
			if ( *name == CStringVarName("operator!=") )
			{
				result.push( CIdMethod(NLIAC::CTypeOfOperator::opDiff,0.0,NULL,NULL) );
			}
		}
		
		return result;
	}

	sint32 IObjetOp::getMethodIndexSize() const
	{
		return IObjectIA::getMethodIndexSize() + NLIAC::CTypeOfOperator::opAff;
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
			case NLIAC::CTypeOfOperator::opAdd:
				r.Result = (IObjectIA *) &( (*this) += ( * (IObjetOp *) x) );
				break;

			case NLIAC::CTypeOfOperator::opSub:
				r.Result = (IObjectIA *) &( (*this) -= ( * (IObjetOp *) x) );
				break;

			case NLIAC::CTypeOfOperator::opMul:
				r.Result = (IObjectIA *) &( (*this) *= ( * (IObjetOp *) x) );
				break;

			case NLIAC::CTypeOfOperator::opDiv:
				r.Result = (IObjectIA *) &( (*this) /= ( * (IObjetOp *) x) );
				break;

			case NLIAC::CTypeOfOperator::opNot:
				r.Result = (IObjectIA *) ( !(*this)  );
				break;

			case NLIAC::CTypeOfOperator::opEq:
				r.Result = (IObjectIA *) ( (*this) == ( * (IObjetOp *) x ) );
				break;
		}
		return r;
	}
}