/** \file constraint_debug.cpp
 *
 * $Id: constraint_debug.cpp,v 1.4 2001/01/08 14:42:11 valignat Exp $
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
#include "nel/ai/script/constraint_debug.h"

namespace NLAISCRIPT
{
	CConstraintDebug::CConstraintDebug(int lin, int col):
		_Satisfied(false),_Lin(lin),_Col(col)
	{			
		_Code = NULL;
		_Txt = NULL;
	}

	CConstraintDebug::~CConstraintDebug()
	{
		if(_Txt) delete _Txt;
	}	

	bool CConstraintDebug::satisfied()
	{
		return _Satisfied;
	}

	void CConstraintDebug::addIndex(int i,CCodeBrancheRun *c)
	{
		_I = i;
		_Code = c;
		if( _Code->getType() == CCodeBrancheRunDebug::IdCodeBrancheRunDebug)
		{
			CCodeBrancheRunDebug* cbrd = (CCodeBrancheRunDebug*) _Code;
			cbrd->setLineCode(_Lin, _I);
		}
		this->release();
	}


	void CConstraintDebug::run(CCompilateur &comp)
	{;
		_Satisfied = true;
	}

	void CConstraintDebug::run(IConstraint *comp)
	{
		_Satisfied = true;
	}

	void CConstraintDebug::getError(char *txt) const
	{			
		strcpy(txt,_Txt);
	}

	/// Not used.
	bool CConstraintDebug::operator == (const IConstraint &) const
	{
		return false;
	}

	/// Not used.
	const NLAIC::CIdentType *CConstraintDebug::getConstraintTypeOf()
	{
		return NULL;
	}

	const IConstraint *CConstraintDebug::clone() const
	{
		IConstraint *x = new CConstraintDebug(_Lin,_Col);
		x->incRef();
		return x;
	}

	/// Not used.
	const char *CConstraintDebug::getInfo()
	{
		return "Y ?";
	}

	/// Not used.
	bool CConstraintDebug::dependOn(const IConstraint *) const
	{
		return false;
	}
}
/* End of constraint_debug.h */
