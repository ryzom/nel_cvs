/** \file fuzzyruleset.cpp
 *	A container for fuzzy rules
 *	
 * $Id: fuzzyruleset.cpp,v 1.7 2002/08/21 13:58:33 lecroart Exp $
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
#include "nel/ai/fuzzy/fuzzyruleset.h"
#include "nel/ai/fuzzy/fuzzyrule.h"

namespace NLAIFUZZY
{
	CFuzzyRuleSet::CFuzzyRuleSet(char *comment)
	{
		if ( comment )
		{
			_Comment = new char[strlen( comment ) + 1];
			strcpy(_Comment, comment);
		}
	}

	void CFuzzyRuleSet::addRule(CFuzzyRule *rule)
	{
		_Rules.push_back( rule );
	}

	const NLAIAGENT::IObjectIA ::CProcessResult &CFuzzyRuleSet::run()
	{
		std::list<CFuzzyRule *>::iterator it_r = _Rules.begin();
		while ( it_r != _Rules.end() )
		{
			( *it_r )->run();
		}
		return NLAIAGENT::IObjectIA ::ProcessRun;
	}



	void CFuzzyRuleSet::save(NLMISC::IStream &os)
	{		
/*		NLAIAGENT::IObjetOp::save( os );

		if ( _Value ) 
		{
			os.serial( (bool & ) true );
			os.serial( _Value->getType() );
			_Value->save(os);
		}
		else
			os.serial( (bool &) false );
*/			
	}

	void CFuzzyRuleSet::load(NLMISC::IStream &is)
	{
/*		if ( _Value )
			_Value->release();
//		NLAIAGENT::IObjetOp::load(is);
		Gen::CIdentTypeAlloc id;
		is.serial( id );
		_Value = (Var *)id.allocClass();
		_Value->load(is);	
*/
	}

	void CFuzzyRuleSet::getDebugString(std::string &txt) const
	{	
		if ( _Comment )
			txt += NLAIC::stringGetBuild("CFuzzyRuleSet %s\n (%d rule)", _Comment, _Rules.size());
		else
			txt += NLAIC::stringGetBuild("CFuzzyRuleSet \n (%d rule)", _Rules.size());
	}

	const NLAIC::CIdentType &CFuzzyRuleSet::getType() const
	{
		return idFuzzyRuleSet;
	}

	const NLAIC::IBasicType *CFuzzyRuleSet::clone() const
	{
		NLAIC::IBasicInterface *m = new CFuzzyRuleSet( *this );
		return m;
	}

	const NLAIC::IBasicType *CFuzzyRuleSet::newInstance() const
	{
		CFuzzyRuleSet *instance = new CFuzzyRuleSet();
		return instance;
	}

	bool CFuzzyRuleSet::isEqual(const NLAIAGENT::IBasicObjectIA &) const
	{
		return false;
	}

	void CFuzzyRuleSet::init(NLAIAGENT::IObjectIA  *p)
	{
		NLAIAGENT::IBaseGroupType *params = ((NLAIAGENT::IBaseGroupType *)p);

		while (	 params->size() )
		{
			CFuzzyRule *rule = (CFuzzyRule *) params->getFront();
			addRule( rule );
			params->popFront();
		}
	}
}
