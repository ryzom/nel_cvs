/** \file file.cpp
 *	First order logic facts
 *
 * $Id: fact.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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

#include "logic/valueset.h"

namespace NLIALOGIC {
	class IBaseAssert;
	class CValueSet;
	class CVarSet;

	class CFact : public CValueSet
	{
		private:
			IBaseAssert *_Assert;
		public:
			CFact(IBaseAssert *);
			CFact(IBaseAssert *, bool);
			CFact(IBaseAssert *, CValueSet *);
			CFact(IBaseAssert *, CVarSet *);
			IBaseAssert *getAssert();
			virtual void getDebugString(char *) const;
			virtual void propagate();
			virtual bool operator == (const NLIAAGENT::IBasicObjectIA &a) const;
			CValueSet *asValueSet();
	};
}

#endif
