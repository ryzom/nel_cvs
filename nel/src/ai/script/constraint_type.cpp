/** \file constraint_type.cpp
 *
 * $Id: constraint_type.cpp,v 1.5 2001/01/17 10:32:10 chafik Exp $
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

namespace NLAISCRIPT
{
	CConstraintChkMethodeType::CConstraintChkMethodeType(IConstraint *constraint,int lin,int col):
					_Constraint(constraint),_Satisfied(false),_Lin(lin),_Col(col)
	{
		_Ident = NULL;
		((CConstraintMethode *)constraint)->addConstraint(this);
		char txt[1028*8];		
		sprintf(txt,"constraint<CConstraintChkMethodeType> for %s",((IConstraint *)_Constraint)->getInfo());
		_TxtInfo = new char [strlen(txt) + 1];
		strcpy(_TxtInfo,txt);
		_RunIsDone = false;
	}

	const IConstraint *CConstraintChkMethodeType::clone() const
	{
		IConstraint *x = new CConstraintChkMethodeType(_Constraint,_Lin,_Col);
		return x;
	}

	void CConstraintChkMethodeType::run(CCompilateur &c)
	{
		if(_Ident == NULL)
		{
			_Constraint->run(c);
			_Ident = (NLAIC::CIdentType *)((CConstraintMethode *)_Constraint)->getConstraintTypeOf();
		}
		runConnexcion();
	}

	void CConstraintChkMethodeType::run(IConstraint *c)
	{
		if(c == _Constraint)
		{
			//_Constraint = NULL;
			_RunIsDone = true;
			_Ident = (NLAIC::CIdentType *)((CConstraintMethode *)c)->getConstraintTypeOf();
		}				
	}

	bool CConstraintChkMethodeType::operator == (const IConstraint &c) const
	{
		if(getTypeOfClass() == c.getTypeOfClass() && *((const CConstraintChkMethodeType &)c)._Constraint == *_Constraint)
		{
			return true;
		}
		return false;
	}

	void CConstraintChkMethodeType::addIndex(int i,CCodeBrancheRun *c)
	{
	}

	void CConstraintChkMethodeType::getError(char *txt) const
	{			
		strcpy(txt,_TxtInfo);
	}

	CConstraintChkMethodeType::~CConstraintChkMethodeType()
	{							
		if(!_RunIsDone) ((CConstraintMethode *)_Constraint)->popConstraint(this);		
		_Constraint->release();
		delete _TxtInfo;
	}	
}
