/** \file file.cpp
 *	Factbase, a container for 0 or first order logic asserts and facts of an agent
 *
 * $Id: factbase.h,v 1.8 2001/07/19 13:19:54 portier Exp $
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

#ifndef NL_FACTBASE_H
#define NL_FACTBASE_H

#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/agent_string.h"

namespace NLAILOGIC
{
	class IBaseAssert;
	class CVarSet;
	class CFactPattern;
	class CValueSet;
	class CFact;
	
	class CFactBase : public NLAIAGENT::IObjetOp {
		private:
			std::list<IBaseAssert *> _Asserts;

		private:
			IBaseAssert *findAssert(IBaseAssert *);
			IBaseAssert *findAssert(NLAIAGENT::IVarName &, sint32);

		public:
			static const NLAIC::CIdentType IdFactBase;			
			
		public:
			CFactBase();
			virtual ~CFactBase();
			void addFact(NLAIAGENT::IVarName &, CValueSet *);		
			void addFact(CFact *);

			void removeFact(CFact *);
			IBaseAssert *addAssert(NLAIAGENT::IVarName &, sint32);
			void addAssert(IBaseAssert *);

			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual const NLAIC::CIdentType &getType() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual void getDebugString(std::string &) const;

			virtual bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const;
			virtual const CProcessResult &run();
			virtual bool isTrue() const;

			virtual std::list<CFact *> *getAssertFacts(IBaseAssert *);

			virtual void addGoal(NLAIAGENT::IVarName &, CValueSet *);
	};
}
#endif
