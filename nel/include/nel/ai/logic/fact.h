/** \file fact.h
 *	First order logic facts
 *
 * $Id: fact.h,v 1.11 2003/01/21 11:24:25 chafik Exp $
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

#ifndef NL_FACT_H
#define NL_FACT_H

#include "nel/ai/logic/valueset.h"

namespace NLAILOGIC {
	class IBaseAssert;
	class CValueSet;
	class CVarSet;

	class CFact : public CValueSet
	{
		private:
			IBaseAssert *_Assert;
			NLAIAGENT::IVarName *_AssertName;
		public:

			CFact();			
			CFact(IBaseAssert *);
			CFact(IBaseAssert *, bool);
			CFact(IBaseAssert *, CValueSet *);
			CFact(IBaseAssert *, CVarSet *);
			virtual ~CFact();

			CFact(sint32,NLAIAGENT::IVarName *);
			CFact(NLAIAGENT::IVarName &);
			CFact(NLAIAGENT::IVarName &, bool);
			CFact(NLAIAGENT::IVarName &, CValueSet *);
			CFact(NLAIAGENT::IVarName &, CVarSet *);

			CFact(const CFact &);

			const NLAIC::IBasicType *clone() const;
			const NLAIC::IBasicType *newInstance() const;

			static const NLAIC::CIdentType IdFact;

			IBaseAssert *getAssert();
			NLAIAGENT::IVarName &getAssertName();
			virtual void getDebugString(std::string &) const;
			virtual void propagate();
			virtual bool operator == (const NLAIAGENT::IBasicObjectIA &a) const;
			CValueSet *asValueSet();

			virtual NLAIAGENT::TQueue isMember(const NLAIAGENT::IVarName *,const NLAIAGENT::IVarName *,const NLAIAGENT::IObjectIA &) const;
			virtual	NLAIAGENT::IObjectIA::CProcessResult runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *);
			virtual	NLAIAGENT::IObjectIA::CProcessResult runMethodeMember(sint32 index, NLAIAGENT::IObjectIA *p);
			sint32 getMethodIndexSize() const;

			virtual const NLAIC::CIdentType &getType() const;
	};
}

#endif
