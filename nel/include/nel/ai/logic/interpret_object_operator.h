/** \file file.cpp
 *	Interpret class for operators
 *
 * $Id: interpret_object_operator.h,v 1.6 2001/01/30 10:33:20 portier Exp $
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
#include "nel/ai/logic/fact.h"
#include "nel/ai/logic/var.h"

namespace NLAISCRIPT
{
	class COperatorClass: public CAgentClass
	{
	private:
		NLAILOGIC::CGoal							*_Goal;				/// Goal the operator tris to validate

		std::vector<NLAILOGIC::IBaseVar *>			_Vars;				/// Variables of the operator

		std::vector< NLAILOGIC::IBaseAssert *>		_Conds;				/// Preconditions asserts
		std::vector< std::vector<sint32> >			_PosVarsCond;		/// Pos of a precondition pattern's vars in the operator's vars table

		std::vector<NLAILOGIC::IBaseAssert *>		_Concs;				/// Postconditions asserts			
		std::vector< std::vector<sint32> >			_PosVarsConc;		/// Pos of a postcondition pattern's vars in the operator's vars table

		char										*_Comment;			/// Description of the operator

	public:
		static const NLAIC::CIdentType IdOperatorClass;
		
		/// Sets the comment for the operator
		void setComment(char *);

		/// Sets the goal the operator tries to achieve
		virtual void setGoal(NLAILOGIC::CGoal *);
		virtual const NLAILOGIC::CGoal *getGoal();

		/// Asks wether the operator's preconditions are validated,
		virtual bool isValid(NLAILOGIC::CFactBase *);

		///Transforms a CFactPattern in an assert and a list of variable positions in the operator
		void compileFactPattern (NLAILOGIC::CFactPattern   *, std::vector<NLAILOGIC::IBaseAssert *> &, std::vector<sint32> &);

		// Buils a CFact from its assret and an instanciation for the values of the variables of the rule.
		NLAILOGIC::CFact *buildFromVars(NLAILOGIC::IBaseAssert *, std::vector<sint32> &, NLAILOGIC::CValueSet *);

		/// Returns the pos of a vraiables in the operator's vars table, -1 if not found.
		sint32 getVarPos(NLAILOGIC::IBaseVar *);

		/// Tries to unify an instaciation of the operator's variables with a new CFact
		NLAILOGIC::CValueSet *unifyLiaison( const NLAILOGIC::CValueSet *, NLAILOGIC::CValueSet *, std::vector<sint32> &);

		void getPosListForward(sint32, sint32, std::vector<sint32> &);
		void getPosListBackward(sint32, sint32, std::vector<sint32> &);
		void getAssertPos(NLAILOGIC::IBaseAssert *, std::vector<NLAILOGIC::IBaseAssert *> &, std::vector<sint32> &);
		NLAILOGIC::CValueSet *unifyBackward(std::list<NLAILOGIC::CFact *> &);
		NLAILOGIC::CValueSet *unifyForward(std::list<NLAILOGIC::CFact *> &);

		virtual std::list<NLAILOGIC::CFact *> *test(std::list<NLAILOGIC::CFact *> &) {return NULL;}
		virtual std::list<NLAILOGIC::CFact *> *backward(std::list<NLAILOGIC::CFact *> &);
		virtual std::list<NLAILOGIC::CFact *> *forward(std::list<NLAILOGIC::CFact *> &);
		virtual std::list<NLAILOGIC::CFact *> *propagate(std::list<NLAILOGIC::CFact *> &);

		/// Priority of the operator
		virtual float priority() const;

		/// Own success and failure functions
		/// These function telle other operators and goals that might be waiting for
		/// the execution of this one.
		virtual void success();
		virtual void failure();

		/// Dependencies failure and success notification
		/// These functions are called by other operators or goals who failed or succeeded
		virtual void success( NLAILOGIC::IBaseOperator *);
		virtual void failure( NLAILOGIC::IBaseOperator *);

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
