/** \file file.cpp
 *	Base class for operators
 *
 * $Id: operator.h,v 1.2 2001/01/08 10:47:05 chafik Exp $
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

#ifndef NL_OPERATOR_H
#define NL_OPERATOR_H

#include "agent/agent.h"
#include "logic/boolval.h"

namespace NLAILOGIC
{
	class IBaseVar;
	class IBaseAssert;
	class CFactPattern  ;
	class CVarSet;
	class CFact;
	class CFactBase;
	class CValueSet;
	class CClause;

	class IBaseOperator : public IBaseBoolType {
		protected:

			/// Description of the operator
			char							*_Comment;
			/// Preconditions asserts
			std::vector<IBaseAssert *>		_Conds;				
			/// Postconditions asserts			
			std::vector<IBaseAssert *>		_Concs;				

		public:

			virtual std::list<CFact *> *test(std::list<CFact *> &) {return NULL;}
			virtual std::list<CFact *> *backward(std::list<CFact *> &) = 0;
			virtual std::list<CFact *> *forward(std::list<CFact *> &) = 0;
			virtual std::list<CFact *> *propagate(std::list<CFact *> &) = 0;

			IBaseOperator();
			IBaseOperator(const char *);
			~IBaseOperator();
			IBaseOperator(const IBaseOperator &);

			/// Asks wether the operator's preconditions are validated,
			virtual bool isValid(CFactBase *) =0;

			/// Sets the comment for the operator
			void setComment(char *);

			const std::vector<IBaseAssert *> &getPrecondAsserts() const;
			const std::vector<IBaseAssert *> &getPostCondAsserts() const;

			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
	};
}
#endif
