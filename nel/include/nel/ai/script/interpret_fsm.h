/** \file interpret_fsm.h
 *	Interpret class for operators
 *
 * $Id: interpret_fsm.h,v 1.3 2001/05/22 16:08:01 chafik Exp $
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

#ifndef NL_FSM_CLASS
#define NL_FSM_CLASS

#include "nel/ai/script/interpret_object_agent.h"
#include "nel/ai/logic/goal.h"
#include "nel/ai/logic/fact.h"
#include "nel/ai/logic/var.h"
#include "nel/ai/logic/operator_script.h"

namespace NLAISCRIPT
{
	class CFsmClass: public CAgentClass
	{
	private:
		char										*_Comment;			/// Description of the operator

		// Conditions de classe: l'automate se termine quelque quelque soit l'état sur certains évenements extérieurs.

	public:
		static const NLAIC::CIdentType IdFsmClass;
		
		/// Sets the comment for the operator
		void setComment(char *);

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
		CFsmClass(const NLAIAGENT::IVarName &);
		CFsmClass(const NLAIC::CIdentType &);
		CFsmClass(const NLAIAGENT::IVarName &, const NLAIAGENT::IVarName &);
		CFsmClass(const CFsmClass &);
		CFsmClass();
		virtual ~CFsmClass();

		const NLAIC::IBasicType *clone() const;
		const NLAIC::IBasicType *newInstance() const;
		virtual void getDebugString(std::string &) const;

		virtual NLAIAGENT::IObjectIA *buildNewInstance() const;
	};



	class CSeqFsmClass : public CFsmClass
	{
	private:
		std::vector<NLAIAGENT::IVarName *> _Steps;

	public:
		static const NLAIC::CIdentType IdSeqFsmClass;
		
		/// Sets the comment for the operator
		void setComment(char *);

		/// Own success and failure functions
		/// These function telle other operators and goals that might be waiting for
		/// the execution of this one.
		virtual void success();
		virtual void failure();

		/// Dependencies failure and success notification
		/// These functions are called by other operators or goals who failed or succeeded
		virtual void success( NLAILOGIC::IBaseOperator *);
		virtual void failure( NLAILOGIC::IBaseOperator *);

		const NLAIAGENT::IVarName *getStep(sint32);

public:
		CSeqFsmClass(const NLAIAGENT::IVarName &);
		CSeqFsmClass(const NLAIC::CIdentType &);
		CSeqFsmClass(const NLAIAGENT::IVarName &, const NLAIAGENT::IVarName &);
		CSeqFsmClass(const CFsmClass &);
		CSeqFsmClass();
		~CSeqFsmClass();

		const NLAIC::IBasicType *clone() const;
		const NLAIC::IBasicType *newInstance() const;
		void getDebugString(char *t) const;

		virtual NLAIAGENT::IObjectIA *buildNewInstance() const;

		virtual void addStep(const NLAIAGENT::IVarName &);
	};
}
#endif
