/** \file agent_object.cpp
 *
 * $Id: agent_object.cpp,v 1.10 2002/11/15 09:21:07 chafik Exp $
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
	
}
