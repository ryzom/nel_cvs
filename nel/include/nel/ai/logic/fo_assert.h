/** \file file.cpp
 *	Frst order logic assertions
 *
 * $Id: fo_assert.h,v 1.5 2001/05/22 16:08:01 chafik Exp $
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

#ifndef NL_FO_ASSERT_H
#define NL_FO_ASSERT_H

#include "nel/ai/logic/ai_assert.h"

namespace NLAILOGIC
{
	class CFirstOrderAssert : public IBaseAssert 
	{
		private:
			std::vector< std::vector<sint32> >	_PosVars;		// Position des variables de l'assertion dans ces clauses
			std::list<CValueSet *> _Facts;			// Faits pour l'assertion
			sint32 _NbVars;

			std::vector<CClause *>			_Clauses;

			sint32 findAssert(IBaseAssert *);

			std::vector< std::vector<sint32> > _PosVarsInputs;

		public:
			CFirstOrderAssert(const NLAIAGENT::IVarName &n, sint32 nb_vars = 0);
			CFirstOrderAssert(const CFirstOrderAssert &);
			virtual ~CFirstOrderAssert();
			void connectClause(CClause *, std::vector<sint32> &);
			void addFact(CVarSet *f);
			void addFact(CValueSet *);
			void removeFact(CFact *f);
			void addClause(CClause *CClause, std::vector<sint32> &posvars);
			const std::vector<CClause *> &getClauses();
			void addInput(CClause *, std::vector<sint32> &posvars );

			static const NLAIC::CIdentType IdFirstOrderAssert;

			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual void getDebugString(std::string &) const;
			bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const;
			virtual bool isEqual(const CFirstOrderAssert &) const;
			virtual const IObjectIA::CProcessResult &run();
			virtual bool isTrue() const;
			virtual const NLAIC::CIdentType &getType() const;

			virtual void init(NLAIAGENT::IObjectIA *);
			virtual sint32 nbVars() const;

			void backward(CValueSet *, std::list<CValueSet *> &);

			std::list<CFact *> *backward(CFact *);

			std::list<CFact *> *getFacts() const;
	};
}
#endif
