/** \file file.cpp
 *	Interpret class for operators
 *
 * $Id: interpret_object_operator.h,v 1.5 2001/01/25 10:09:48 portier Exp $
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

#ifndef NL_OPERATOR_CLASS
#define NL_OPERATOR_CLASS

#include "nel/ai/script/interpret_object_agent.h"
#include "nel/ai/logic/goal.h"

namespace NLAISCRIPT
{
	class COperatorClass: public CAgentClass
	{
	private:
		/// Goal the operator tris to validate
/*		CGoal							*_Goal;

		/// Preconditions asserts
		std::vector<IBaseAssert *>		_Conds;				
		std::vector< std::vector<sint32> >	_PosVarsCond;		/// Pos of a precondition pattern's vars in the operator's vars table

		/// Postconditions asserts			
		std::vector<IBaseAssert *>		_Concs;	
		std::vector< std::vector<sint32> >	_PosVarsConc;		/// Pos of a postcondition pattern's vars in the operator's vars table

		/// Description of the operator
		char							*_Comment;

  */
	public:
		static const NLAIC::CIdentType IdOperatorClass;
		
/*
			/// Sets the comment for the operator
			void setComment(char *);

			/// Sets the goal the operator tries to achieve
			virtual void setGoal(CGoal *);
			virtual const CGoal *getGoal();

			/// Asks wether the operator's preconditions are validated,
			virtual bool isValid(CFactBase *);

			/// Priority of the operator
			virtual float priority() const;

			/// Own success and failure functions
			/// These function telle other operators and goals that might be waiting for
			/// the execution of this one.
			virtual void success();
			virtual void failure();

			/// Dependencies failure and success notification
			/// These functions are called by other operators or goals who failed or succeeded
			virtual void success( IBaseOperator *);
			virtual void failure( IBaseOperator *);

			///Transforms a CFactPattern in an assert and a list of variable positions in the operator
			void compileFactPattern (CFactPattern   *, std::vector<IBaseAssert *> &, std::vector<sint32> &);

			/// Returns the pos of a vraiables in the operator's vars table, -1 if not found.
			sint32 CFirstOrderOperator::getVarPos(IBaseVar *var);
			void getPosListForward(sint32, sint32, std::vector<sint32> &);
			void getPosListBackward(sint32, sint32, std::vector<sint32> &);
			void getAssertPos(IBaseAssert *, std::vector<IBaseAssert *> &, std::vector<sint32> &);
			CValueSet *unifyBackward(std::list<CFact *> &);
			CValueSet *unifyForward(std::list<CFact *> &);
*/
	public:
		COperatorClass(const NLAIAGENT::IVarName &);
		COperatorClass(const NLAIC::CIdentType &);
		COperatorClass(const NLAIAGENT::IVarName &, const NLAIAGENT::IVarName &);
		COperatorClass(const COperatorClass &);
		COperatorClass();

		const NLAIC::IBasicType *clone() const;
		const NLAIC::IBasicType *newInstance() const;
		void getDebugString(char *t) const;

		virtual NLAIAGENT::IObjectIA *buildNewInstance() const;

		virtual ~COperatorClass();
	};
}
#endif
