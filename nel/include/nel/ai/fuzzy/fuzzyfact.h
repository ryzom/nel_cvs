/** \file fuzzyfact.h
 *	This class describes the output of a fuzzy rule for a particular fuzzy set
 *
 * $Id: fuzzyfact.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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


#ifndef NL_FUZZYFACT_H
#define NL_FUZZYFACT_H

#include "fuzzy/fuzzyset.h"

namespace NLIAFUZZY
{
	class CFuzzyVar;

	class CFuzzyFact : public NLIAAGENT::IObjetOp
	{	
		private:
			CFuzzyVar *_Var;
			IFuzzySet *_Set;
			double _Value;
		public:			
			static const NLIAC::CIdentType IdFuzzyFact;

			CFuzzyFact(const CFuzzyFact &);
			CFuzzyFact(IFuzzySet *set, double value);
			IFuzzySet *set();
			CFuzzyVar *var();
			double value();

			virtual void getDebugString(char *);
			virtual void load(NLMISC::IStream &is);
			virtual void save(NLMISC::IStream &os) ;

			virtual const CProcessResult &run();
			virtual bool isEqual(const NLIAAGENT::IBasicObjectIA &a) const;
			virtual bool isEqual(const NLIALOGIC::CBoolType &a) const;
			virtual std::vector<NLIALOGIC::IBaseVar *> *getVars();

			virtual const NLIAC::CIdentType &getType() const;
			const NLIAC::IBasicType *clone() const;
			const NLIAC::IBasicType *newInstance() const;
			virtual void getDebugString(char *) const;
			
			bool isTrue() const;
	};
}

#endif
