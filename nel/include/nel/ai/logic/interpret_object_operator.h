/** \file interpret_object_operator.h
 *	Interpret class for operators
 *
 * $Id: interpret_object_operator.h,v 1.21 2002/09/19 10:09:34 robert Exp $
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
#include "nel/ai/fuzzy/fuzzyset.h"

namespace NLAISCRIPT
{
	class COperatorClass: public CAgentClass
	{
	private:
		NLAILOGIC::CGoal							*_Goal;				/// Goal the operator tris to validate

		const NLAIAGENT::IVarName					*_GoalAssert;		/// Name of the goal's assert
		std::list<const NLAIAGENT::IVarName *>		_GoalVars;			/// Name of the goal's vars
		std::vector<sint32>							_GoalPosVar;		/// Pos of the goal's var in the operato's static components

		std::vector<NLAILOGIC::IBaseVar *>			_Vars;				/// Variables of the operator

		std::vector< NLAILOGIC::IBaseAssert *>		_Conds;				/// Preconditions asserts
		std::vector< std::vector<sint32> >			_PosVarsCond;		/// Pos of a precondition pattern's vars in the operator's vars table

		std::vector<NLAILOGIC::IBaseAssert *>		_Concs;				/// Postconditions asserts			
		std::vector< std::vector<sint32> >			_PosVarsConc;		/// Pos of a postcondition pattern's vars in the operator's vars table

		std::vector<NLAIAGENT::IMessageBase::TPerformatif>	_TrigMsgPerf;
		std::vector<NLAIC::CIdentType>				_TrigMsgClass;
		std::vector<std::string>					_TrigMsgVarname;
		std::vector<sint32>							_TrigMsgPos;

		char										*_Comment;			/// Description of the operator

		std::vector<NLAILOGIC::CGoal>				_Steps;				// Successive goals to be achieved
		std::vector<sint32>							_StepsMode;			// Infos about synchro for the steps of the operator

		sint32										_UpdateCycles;		// Number of cycles before checking preconditions

		float										_Priority;

	public:
		static const NLAIC::CIdentType IdOperatorClass;
		
		/// Sets the comment for the operator
		void setComment(char *);

		/// Sets the goal the operator tries to achieve
//		virtual void setGoal(NLAILOGIC::CGoal *);
		virtual void setGoal(const NLAIAGENT::CStringVarName &);
		virtual void setGoal(const NLAIAGENT::IVarName *, std::list<const NLAIAGENT::IVarName *> &);
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
		/// These functions tell other operators and goals that might be waiting for
		/// the completion of this one
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
		virtual ~COperatorClass();

		const NLAIC::IBasicType *clone() const;
		const NLAIC::IBasicType *newInstance() const;
		virtual void getDebugString(std::string &) const;

		virtual NLAIAGENT::IObjectIA *buildNewInstance() const;


		void addPrecondition(NLAILOGIC::CFactPattern *);
		void addPostcondition(NLAILOGIC::CFactPattern *);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// Initialisation from the grammar

		// Logique
		std::list<const NLAIAGENT::IVarName *> _BooleanConds;
		std::list<const NLAIAGENT::IVarName *> _BooleanConcs;
		std::vector<const NLAIAGENT::IVarName *> _CondAsserts;
		std::vector<const NLAIAGENT::IVarName *> _ConcAsserts;
		std::vector< std::list<const NLAIAGENT::IVarName *> *> _ClassCondVars;
		std::vector< std::list<const NLAIAGENT::IVarName *> *> _ClassConcVars;
		std::vector< IOpCode *> _CondCode;
		std::vector< IOpCode *> _ConcCode;

		NLAILOGIC::CFactBase *_FactBase;

		std::vector<NLAIAGENT::IVarName *> _FuzzyVars;
		std::vector<NLAIAGENT::IVarName *> _FuzzySets;
	//	NLAIAGENT::CStringVarName *_GoalName;
		///////////////////////////////////////////////////

/*
		/// Adds a first order logic precondition to the operator
		void addPrecondition(NLAILOGIC::CFactPattern   *);

		/// Adds a first order logic postcondition to the operator
		void addPostcondition(NLAILOGIC::CFactPattern   *);
*/
//		virtual ~COperatorClass();

		/// Add first order patterns as preconditions or postconditions
		void addFirstOrderCond(const NLAIAGENT::IVarName *, std::list<const NLAIAGENT::IVarName *> &);
		void addFirstOrderConc(const NLAIAGENT::IVarName *, std::list<const NLAIAGENT::IVarName *> &);

		/// Add first order patterns as preconditions or postconditions
		void addBoolCond(const NLAIAGENT::IVarName *);
		void addBoolConc(const NLAIAGENT::IVarName *);

		/// Add first order patterns as preconditions or postconditions
		/// PreConditions code must be any piece of code that returns an object that is true or false using the isTrue() function.
		void addCodeCond(IOpCode *);
		/// PostConditions code is code that will be executed upon completion of the execution of the operator
		void addCodeConc(IOpCode *);

		// Adds a fuzzy cond of the form <attrib> is <filter>
		void addFuzzyCond(NLAIAGENT::IVarName *, NLAIAGENT::IVarName *);

		/// Initialises the logic conditions compiled tables
		void buildLogicTables();

		bool isValidFonc(NLAIAGENT::IObjectIA *);
		void activatePostConditions(NLAIAGENT::IObjectIA *);

		void initialiseFactBase(NLAILOGIC::CFactBase *);

		std::vector<NLAIAGENT::IVarName *> &getFuzzyVars()
		{
			return _FuzzyVars;
		}

		std::vector<NLAIAGENT::IVarName *> &getFuzzySets()
		{
			return _FuzzySets;
		}

		std::vector<NLAILOGIC::IBaseVar *>	&getVars()
		{
			return _Vars;
		}

		std::vector< NLAILOGIC::IBaseAssert *>	&getConds()
		{
			return _Conds;
		}

		std::vector< NLAILOGIC::IBaseAssert *>	&getConcs()
		{
			return _Concs;
		}

		std::vector< std::vector<sint32> >	&getPosVarsConds()
		{
			return _PosVarsCond;
		}

		std::vector< std::vector<sint32> >	&getPosVarsConcs()
		{
			return _PosVarsConc;
		}

		std::vector<sint32> &getGoalVarPos()
		{
			return _GoalPosVar;
		}

		virtual void setUpdateEvery(sint32 cycles)
		{
			_UpdateCycles = cycles;
		}

		sint32 getUpdateEvery()
		{
			return _UpdateCycles;
		}

		void setPriority(float p)
		{
			_Priority = p;
		}

		float getPriority() const
		{
			return _Priority;
		}

		void RegisterMessage(NLAIAGENT::IMessageBase::TPerformatif , const std::string &, const std::string & );
		virtual sint32 checkTriggerMsg(const NLAIAGENT::IMessageBase *);
		inline sint32 NbMsgTrigger()
		{
			return _TrigMsgPerf.size();
		}
	};
}
#endif
