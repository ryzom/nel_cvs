/** \file fuzzycond.cpp
 * Fuzzy conditions
 *
 * $Id: fuzzycond.cpp,v 1.1 2001/01/05 10:53:49 chafik Exp $
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
#include "fuzzy/fuzzyset.h"
#include "fuzzy/fuzzyvar.h"
#include "fuzzy/fuzzycond.h"

#include "agent/agent_nombre.h"
#include "agent/agent.h"

namespace NLIAFUZZY
{

	CSimpleFuzzyCond::CSimpleFuzzyCond(CFuzzyVar *var, IFuzzySet *set)
	{
		_Var = var;
		_Set = set;
	}

	void CSimpleFuzzyCond::getDebugString(char *txt) const
	{
		sprintf(txt, "(%s is %s)", _Var->getName().getString(), _Set->getName() );
	}

	bool CSimpleFuzzyCond::isTrue()
	{
		return _Var->isIn( _Set );
	}

	float CSimpleFuzzyCond::truthValue()
	{
		return _Var->membership( _Set );
	}

	void CSimpleFuzzyCond::save(NLMISC::IStream &)
	{
		//NLIAAGENT::IObjectIA::save(os);
		// TODO: sauvegarder une référence 
	}

	void CSimpleFuzzyCond::load(NLMISC::IStream &)
	{
		// NLIAAGENT::IObjectIA::load( is );
		//TODO: recharger par rapport à la référence
	}

	void CSimpleFuzzyCond::init(NLIAAGENT::IObjectIA *params)
	{
		if ( ((NLIAAGENT::IBaseGroupType *)params)->size() < 2)
		{
			// TODO: throw Exception
		}

		NLIAAGENT::IObjectIA *arg = (NLIAAGENT::IObjectIA *) ((NLIAAGENT::IBaseGroupType *)params)->popFront();
		_Var = (CFuzzyVar *) arg->clone();
		arg->release();

		arg = (NLIAAGENT::IObjectIA *) ((NLIAAGENT::IBaseGroupType *)params)->popFront();
		_Set = (IFuzzySet *) arg->clone();
		arg->release();
	}
}