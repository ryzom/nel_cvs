/** \file fo_operator.h
 *	First order logic operators with forward and backward chaining
 *
 * $Id: fo_operator.h,v 1.6 2002/02/20 18:05:10 lecroart Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU NLIACeral Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * NLIACeral Public License for more details.

 * You should have received a copy of the GNU NLIACeral Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_FOOPERATOR_H
#define NL_FOOPERATOR_H

#include "nel/ai/agent/agent.h"
#include "nel/ai/logic/operator.h"

namespace NLAILOGIC
{
	class IBaseVar;
	class IBaseBoolType;
	class IBaseAssert;
	class CFactPattern;
	class CVarSet;
	class CFact;
	class CValueSet;

	class CFirstOrderOperator : public IBaseOperator {
		protected:
			std::vector<IBaseVar *>			_Vars;				/// Variables of the operator
			std::vector< std::vector<sint32> >	_PosVarsCond;		/// Pos of a precondition pattern's vars in the operator's vars table
			std::vector< std::vector<sint32> >	_PosVarsConc;		/// Pos of a postcondition pattern's vars in the operator's vars table

			///Transforms a CFactPattern in an assert and a list of variable positions in the operator
			void compileFactPattern (CFactPattern   *, std::vector<IBaseAssert *> &, std::vector<sint32> &);

			/// Returns the pos of a vraiables in the operator's vars table, -1 if not found.
			sint32 CFirstOrderOperator::getVarPos(IBaseVar *var);
			void getPosListForward(sint32, sint32, std::vector<sint32> &);
			void getPosListBackward(sint32, sint32, std::vector<sint32> &);
			void getAssertPos(IBaseAssert *, std::vector<IBaseAssert *> &, std::vector<sint32> &);
			CValueSet *unifyBackward(std::list<CFact *> &);
			CValueSet *unifyForward(std::list<CFact *> &);

			// Buils a CFact from its assret and an instanciation for the values of the variables of the rule.
			CFact *buildFromVars(IBaseAssert *, std::vector<sint32> &, CValueSet *);

			/// Forward chaining
			std::list<CValueSet *> *propagate(std::list<CValueSet *> &, CValueSet *, std::vector<sint32> &);

			/// Tries to unify an instaciation of the operator's variables with a new CFact
			CValueSet *unifyLiaison( const CValueSet *, CValueSet *, std::vector<sint32> &);
			
		public:
			CFirstOrderOperator();
			CFirstOrderOperator(const CFirstOrderOperator &);
			~CFirstOrderOperator();

			/// Gives the postcondition for a CFact in the preconditions
			std::list<CFactPattern   *> *forward(CFactPattern   *);

			/// Gives the precondition for a CFact in the postconditions
			std::list<CFactPattern   *> *backward(CFactPattern   *);

			/// Adds a first order logic precondition to the operator
			void addPrecondition(CFactPattern   *);

			/// Adds a first order logic postcondition to the operator
			void addPostcondition(CFactPattern   *);

			/// Returns the result of backarding a list of facts through the operator
			std::list<CFact *> *backward(std::list<CFact *> &);

			/// Returns the conclusions of the operator for given preconditions facts
			std::list<CFact *> *forward(std::list<CFact *> &);

			/// Propagates facts through the operator and network of asserts/ operators
			std::list<CFact *> *propagate(std::list<CFact *> &);

			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual void getDebugString(std::string &) const;
			virtual bool isEqual(const CFirstOrderOperator &a) const;
			virtual const NLAIAGENT::IObjectIA::CProcessResult &run();
			bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const;
			const NLAIC::CIdentType &getType() const;

			virtual bool operator==(CFirstOrderOperator *);

			static const NLAIC::CIdentType IdCFirstOrderOperator;

			std::list<CFact *> *test(std::list<CFact *> &);

			/// Returns true if the preconditions are validated
			virtual bool isTrue() const;
			virtual float truthValue() const;
			virtual bool isValid(CFactBase *);

			virtual float priority() const;
			virtual void success();
			virtual void failure();
			virtual void success(IBaseOperator *);
			virtual void failure(IBaseOperator *);
	};
}
#endif
