/** \file file.cpp
 *	Bool asserts for rules and operators
 *
 * $Id: bool_assert.h,v 1.4 2001/01/10 11:27:14 chafik Exp $
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

#ifndef NL_BOOL_ASSERT_H
#define NL_BOOL_ASSERT_H

#include "nel/ai/logic/ia_assert.h"

namespace NLAILOGIC {
	class CBoolAssert : public NLAILOGIC::IBaseAssert 
	{
		private:
			bool _Value;
		public:
			CBoolAssert( const NLAIAGENT::IVarName &, bool truth = false); 
			CBoolAssert( const CBoolAssert &);
			virtual ~CBoolAssert();
			virtual void addFact(NLAILOGIC::CFact *);
			virtual void addFact(CVarSet *);
			virtual void addFact(CValueSet *);
			virtual void removeFact(NLAILOGIC::CFact *);

			static const NLAIC::CIdentType IdBoolAssert;
			virtual const NLAIC::CIdentType &getType() const;

			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual void getDebugString(char *text) const;
			bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const;
			virtual const NLAIAGENT::IObjectIA::CProcessResult &run();

			virtual bool isTrue() const;

			virtual sint32 nbVars() const;
			virtual std::list<NLAILOGIC::CFact *> *getFacts() const;
	};
}
#endif

