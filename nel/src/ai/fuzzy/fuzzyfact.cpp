/** \file fuzzyfact.cpp
 * Fuzzy facts
 *
 * $Id: fuzzyfact.cpp,v 1.6 2001/05/22 16:08:15 chafik Exp $
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
#include "nel/ai/fuzzy/fuzzyfact.h"

namespace NLAIFUZZY
{
	
	CFuzzyFact::CFuzzyFact(IFuzzySet *set, double value)
	{
		_Set = set;
		_Value = value;
	}

	CFuzzyFact::CFuzzyFact(const CFuzzyFact &cp)
	{
		_Set = cp._Set;
		_Value = cp._Value;
	}

	IFuzzySet *CFuzzyFact::set()
	{
		return _Set;
	}

	double CFuzzyFact::value()
	{
		return _Value;
	}

	CFuzzyVar *CFuzzyFact::var()
	{
		return _Var;
	}	

	void CFuzzyFact::load(NLMISC::IStream &)
	{
	}

	void CFuzzyFact::save(NLMISC::IStream &)
	{
	}

	const NLAIAGENT::IObjectIA::CProcessResult &CFuzzyFact::run()
	{
		return NLAIAGENT::IObjectIA::ProcessRun;
	}

	bool CFuzzyFact::isEqual(const NLAIAGENT::IBasicObjectIA &) const
	{
		return false;
	}

	bool CFuzzyFact::isEqual(const NLAILOGIC::CBoolType &) const
	{
		return false;
	}

	const NLAIC::CIdentType &CFuzzyFact::getType() const
	{
		return IdFuzzyFact;
	}

	const NLAIC::IBasicType *CFuzzyFact::clone() const
	{
		NLAIC::IBasicInterface *m = new CFuzzyFact( *this );
		return m;
	}

	const NLAIC::IBasicType *CFuzzyFact::newInstance() const
	{
		return clone();
	}

	void CFuzzyFact::getDebugString(std::string &txt) const
	{
		std::string buf;
		_Set->getDebugString( buf );
		txt += NLAIC::stringGetBuild("CFuzzyFact (%s IS %d)", buf.c_str(), _Value);
	}

	bool CFuzzyFact::isTrue() const
	{
		return false;
	}

	std::vector<NLAILOGIC::IBaseVar *> *CFuzzyFact::getVars()
	{
		return NULL;
	}

}

