/** \file fuzzycond.h
 * Fuzzy condition, ie 
 *
 * $Id: fuzzycond.h,v 1.4 2001/05/22 16:08:01 chafik Exp $
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


#ifndef NL_FUZZYCOND_H
#define NL_FUZZYCOND_H

#include "nel/ai/agent/agent.h"

#include "nel/ai/fuzzy/fuzzytype.h"

namespace NLAIFUZZY
{
 
	class CFuzzyVar;
	class IFuzzySet;

	// CCondition floue simple.

	class CSimpleFuzzyCond  : public FuzzyType {
		private:
			CFuzzyVar *_Var;
			IFuzzySet *_Set;
		public:
			CSimpleFuzzyCond(CFuzzyVar *,IFuzzySet *);
			virtual void getDebugString(std::string &) const;
			virtual bool isTrue();
			virtual float truthValue();
			virtual void save(NLMISC::IStream &os) ;
			virtual void load(NLMISC::IStream &is);
			virtual void init(NLAIAGENT::IObjectIA *);
	};
}
#endif
