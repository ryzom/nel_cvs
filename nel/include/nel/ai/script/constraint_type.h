/** \file constraint_type.h
 * class for the compiler fonctionality.
 *
 * $Id: constraint_type.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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
#ifndef NL_CONSTRAINT_TYPE_H
#define NL_CONSTRAINT_TYPE_H

namespace NLIASCRIPT
{	

	/**
	* Class CConstraintChkMethodeType is use when we have an undefined method return type, but when the type can be compute after the parsing.
	* This happened for exemple if we write 'return Something();'. Where Something() is a method memeber not already defined.
	*
	* \author Chafik sameh
	* \author Nevrax France
	* \date 2000
	*/
	class CConstraintChkMethodeType: public IConstraint
	{
	private:
		///Type of expression after the constraint is resolve.
		NLIAC::CIdentType	*_Ident;
		///Type of unresolving expression.
		IConstraint *_Constraint;
		/// \name Attribut need for IConstraint method.
		//@{
		bool			_Satisfied;
		int				_Lin;
		int				_Col;
		char			*_TxtInfo;
		//@}
		bool			_RunIsDone;

	public:
		CConstraintChkMethodeType(IConstraint *constraint, int lin, int col);
	
		/// \name IConstraint method.
		//@{
		bool satisfied()
		{			
			return _Satisfied;
		}

		const char *getInfo() 
		{
			return _TxtInfo;
		}

		void run(CCompilateur &);
		void run(IConstraint *);
		void addIndex(int i,CCodeBrancheRun *c);
		
		int getLine() const
		{
			return _Lin;
		}
		int getColone() const
		{
			return _Col;
		}

		bool dependOn(const IConstraint *c) const
		{
			return c == _Constraint || _Constraint->dependOn(c);
		}

		void getError(char *) const;

		const IConstraint *clone() const;		

		const NLIAC::CIdentType *getConstraintTypeOf()
		{
			return _Ident;
		}

		bool operator == (const IConstraint &) const;

		ConstraintTypeEnum getTypeOfClass() const
		{
			return constraintChkMethodeType;
		}
		//@}
		
		~CConstraintChkMethodeType();
	};
}

#endif
