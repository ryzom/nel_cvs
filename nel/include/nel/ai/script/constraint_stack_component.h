/** \file constraint_stack_component.h
 * Class for the compiler fonctionality.
 *
 * $Id: constraint_stack_component.h,v 1.2 2001/01/08 10:47:05 chafik Exp $
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
#ifndef NL_CONSTRAINT_STACK_COMPONENT_H
#define NL_CONSTRAINT_STACK_COMPONENT_H

namespace NLIASCRIPT
{

	/**
	* Class CConstraintStackComp.
	* 
	* This class is allow to load component from the stack or the heap.	
	*
	* \author Chafik sameh
	* \author Nevrax France
	* \date 2000
	*/
	class CConstraintStackComp: public IOpType		
	{
	public:
		///This enum give information of the way to load the compoment.
		enum OpCodeType {
			///Load the compoment from a class stored in the heap.
			heapCall,
			///Load the compoment from a class stored in the stack.
			stackCall,
			///set a compoment stored in the self class.
			heapAffectation
		};
	private:
		///Set the type of component, it can be an unresolved constraint.
		IOpType	*_TypeStack;
		///Set herarchy name (a hierarchy name is some thing like classBaseName.className.component).
		std::list<NLIASCRIPT::CStringType > _MemberName;

		/// \name Attribut need for IConstraint method.
		//@{
		sint32 _Lin;
		sint32 _Col;
		bool _Satisfied;
		char *_TxtInfo;
		std::list<std::pair<sint32, CCodeBrancheRun *> > _Code;
		//@}

		///The type of compoment when it resolve.
		NLAIC::CIdentType *_Id;
		///Type of load.
		OpCodeType _OpCodeType;
		///It used when the type is a heapCall and define the heap position.
		sint32	_StackPos;

	public:
		CConstraintStackComp(OpCodeType opCodeType,sint32 stackPos,const std::list<NLIASCRIPT::CStringType > &memberName,IOpType *typeStack,sint32 lign,sint32 col);
		/// \name IConstraint method.
		//@{
		const char *getInfo() 
		{
			return _TxtInfo;
		}
		void run(CCompilateur &);
		void run(IConstraint *);
		bool satisfied()
		{
			return _Satisfied;
		}
		bool dependOn(const IConstraint *) const;
		bool operator == (const IConstraint &c) const;
		void addIndex(sint32 i,CCodeBrancheRun *c)
		{
			_Code.push_back(std::pair<sint32, CCodeBrancheRun *>(i,c));
		}
		void getError(char *) const;				
		const IConstraint *clone() const;
		virtual ConstraintTypeEnum getTypeOfClass() const
		{
			return constraintStackComp;
		}

		const NLAIC::CIdentType *getConstraintTypeOf();
		int getLine() const
		{
			return _Lin;
		}
		int getColone() const
		{
			return _Col;
		}
		//@}

		~CConstraintStackComp();

	};
}
#endif
