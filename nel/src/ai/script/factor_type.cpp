/** \file factor_type.cpp
 *
 * $Id: factor_type.cpp,v 1.4 2001/01/10 10:10:08 chafik Exp $
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
	void CFactorType::del()
	{
		MemberName.clear();
		Member.clear();
		if(Value)
				Value->release();
		if(TypeStack)
				TypeStack->release();
	}

	void CFactorType::set(const CFactorType &v)
	{		
		del();
		Value = v.Value;
		v.Value->incRef();
		ValueVar = v.ValueVar;
		VarType = v.VarType;
		IsUsed = v.IsUsed;
		Member = v.Member;	
		MemberName = v.MemberName;
		TypeStack = v.TypeStack;
		TypeStack->incRef();
	}

	CFactorType::CFactorType(const CFactorType &v)
	{
		Value = v.Value;
		v.Value->incRef();
		ValueVar = v.ValueVar;
		VarType = v.VarType;
		IsUsed = v.IsUsed;
		Member = v.Member;
		MemberName = v.MemberName;		
		TypeStack = v.TypeStack;
		TypeStack->incRef();
	}

	CFactorType::CFactorType()
	{			
		Value = NULL;
		ValueVar = NULL;
		TypeStack = NULL;
	}
}
