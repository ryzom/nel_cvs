/** \file fuzzyruleset.h
 *	A set of fuzzy rules which desribes a fuzzy controler
 *
 * $Id: fuzzyruleset.h,v 1.4 2001/05/22 16:08:01 chafik Exp $
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

#ifndef NL_FUZZYRULESET_H
#define NL_FUZZYRULESET_H

#include "nel/ai/agent/agent.h"

namespace NLAIFUZZY
{
	class CFuzzyRule;

	class CFuzzyRuleSet : public NLAIAGENT::IObjectIA 
	{
		private:
			char *_Comment;
			std::list<CFuzzyRule *> _Rules;
		public:

			static const NLAIC::CIdentType idFuzzyRuleSet;

			CFuzzyRuleSet(char *comment = NULL);
			void addRule(CFuzzyRule *);
			virtual const CProcessResult &run();
			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual void getDebugString(std::string &) const;
			const NLAIC::CIdentType &getType() const;
			virtual bool isEqual(const NLAIAGENT::IBasicObjectIA &) const;
			virtual void init(NLAIAGENT::IObjectIA *);
	};
}

#endif
