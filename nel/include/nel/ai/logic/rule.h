/** \file file.cpp
 *	First order logic rules with forward and backward chaining
 *
 * $Id: rule.h,v 1.2 2001/01/08 10:47:05 chafik Exp $
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

#ifndef NL_RULE_H
#define NL_RULE_H

#include "logic/clause.h"
namespace NLAIAGENT
{
	class IObjetOp;
}

namespace NLAILOGIC 
{

	

	class CRule : public CClause 
	{
		private:
			std::vector<IBaseAssert *>		_Concs;
			std::vector< std::vector<sint32> >	_PosVarsConc;

			// Retourne la position 
			void getConcPosList(sint32, sint32, std::vector<sint32> &);

		public:
			static const NLAIC::CIdentType IdRule;

			CRule();
			CRule(std::list<CFactPattern  *> &, std::list<CFactPattern  *> &);
			CRule(const CRule &);
			virtual void init(NLAIAGENT::IObjectIA *);
			~CRule();
			void addConc(CFactPattern  *);
			void setCond(CClause &);
			void forward(CVarSet *);
			virtual void addConflicts();
			virtual const NLAIC::CIdentType &getType() const;				
			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual void getDebugString(char *text) const;
			CValueSet *unifyLiaisonBack(CValueSet *, std::list<NLAIAGENT::IObjetOp *> &, std::vector<sint32> &, bool &);

			CVarSet *backWard(CFactPattern  *);
			CVarSet *backward(CFactPattern  *, std::vector<sint32> &, std::vector<sint32> &);
			void backward( std::list<CValueSet *> * );
			std::list<CValueSet *> *unifyBack(CFactPattern  *);

			void getPosListBackward(sint32 , sint32 , std::vector<sint32> &);
			void getPosListForward(sint32 , sint32 , std::vector<sint32> &);
	};
}
#endif
