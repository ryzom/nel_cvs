/** \file file.cpp
 *	Base class for assertions
 *
 * $Id: ia_assert.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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

#ifndef NL_IA_ASSERT_H
#define NL_IA_ASSERT_H

#include "agent/agent.h"
#include "logic/var.h"

namespace NLIALOGIC
{
	class CClause;
	class CVarSet;
	class CValueSet;
	class CFact;
	class IBaseOperator;

	class IBaseAssert : public NLIAAGENT::IObjetOp
	{
		protected:
			NLIAAGENT::IVarName *_Name;
			std::vector<IBaseOperator *>		_Outputs;		// Operators referecing the assertion in their preconditions
			std::vector<IBaseOperator *>		_Inputs;		// Operators referecing the assertion in their postconditions
	
		public:
			IBaseAssert(const NLIAAGENT::IVarName &n );
			virtual ~IBaseAssert();

			//virtual void connectClause(CClause *) = 0;
			virtual void addFact(CValueSet *) = 0;
			virtual void addFact(CVarSet *) = 0;
			virtual void removeFact(CFact *) = 0;

			virtual NLIAAGENT::IVarName &getName() const
			{
				return *_Name;
			}

			virtual bool operator==(IBaseAssert *cp) 
			{
				return ( _Name == cp->_Name );
			}

			virtual void addInput(IBaseOperator *);
			virtual const std::vector<IBaseOperator *> &getInputs();
			virtual void addOutput(IBaseOperator *);
			virtual const std::vector<IBaseOperator *> &getOutputs();

			virtual const NLIAC::CIdentType &getType() const = 0;
			virtual sint32 nbVars() const = 0;
			virtual std::list<CFact *> *getFacts() const = 0;
	};
}

#endif
