/* A debug constraint. It keep a trace between original source code and OpCode.
 *
 * $Id: constraint_debug.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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

#ifndef NL_CONSTRAINT_DEBUG_H
#define NL_CONSTRAINT_DEBUG_H

#include "constraint.h"

namespace NLIASCRIPT
{
	class CCompilateur;
	class CCodeBrancheRun;

	/**
	 * A debug constraint. It keep a trace between original source code and OpCode.
	 * \author Gabriel ROBERT
	 * \author Nevrax France
	 * \date 2000
	 */
	class CConstraintDebug : public IConstraint
	{
	private:
		bool			_Satisfied;	/// Is the constraint satified ?
		int				_Lin;		/// OpCode line number in source texte
		int				_Col;		/// OpCode colone number in source texte
		int				_I;			/// Index in the CCodeBrancheRun
		CCodeBrancheRun	*_Code;		/// Pointer to the CCodeBrancheRun
		char			*_Txt;		/// Error message
		
	public:
		/** Constructor
		 * param lin Line number in the source code.
		 * param col Colonne number in the source code.
		 */
		CConstraintDebug(int lin, int col);

		/// Destructor
		virtual ~CConstraintDebug();

		/// \name IConstraint method.
		//@{
		void run(CCompilateur & c);
		void run(IConstraint * c);
		bool satisfied();
		const IConstraint *clone() const;
		ConstraintTypeEnum getTypeOfClass() const {return constraintDebug;}
		//@}

		/** Fix the opCode adress.
		 *	It must be call before run(CCompilateur & c).
		 *  WARNING : This function will self-destruct the constraint.
		 *	\param i Index in the CCodeBrancheRun
		 *	\param c pointer to the CCodeBrancheRun
		 */
		void addIndex(int i,CCodeBrancheRun *c);

		/// Get the error message.
		void getError(char *) const;

		/// Get the opCode line number in source texte
		int getLine() const	{return _Lin;}

		/// Get the opCode colone number in source texte
		int getColone() const {return _Col;}

		/** \name IConstraint method.
		 * Not used.
		 */
		//@{
		bool operator == (const IConstraint &) const;
		const NLIAC::CIdentType *getConstraintTypeOf();
		const char *getInfo();
		bool dependOn(const IConstraint *) const;
		//@}
	};	
}
#endif
/* End of constraint_debug.h */

