/** \file fuzzyrule.cpp
 * Fuzzy rules
 *
 * $Id: fuzzyrule.cpp,v 1.4 2001/01/10 10:10:08 chafik Exp $
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
#include "nel/ai/agent/agent.h"
#include "nel/ai/logic/boolval.h"
#include "nel/ai/fuzzy/fuzzyrule.h"
#include "nel/ai/fuzzy/fuzzyvar.h"


namespace NLAIFUZZY
{

	CFuzzyRule::CFuzzyRule(char *comment)
	{
		if ( comment )
		{
			_Comment = new char[strlen(comment) + 1];
			strcpy(_Comment, comment);
		}
		else 
			_Comment = NULL;
	}

	CFuzzyRule::CFuzzyRule(std::list<NLAILOGIC::IBaseBoolType *> &conds, char *comment, float threshold) : NLAILOGIC::CondAnd( conds )
	{
		_Threshold = threshold;

		if ( comment )
		{
			_Comment = new char[strlen(comment) + 1];
			strcpy(_Comment, comment);
		}
		else 
			_Comment = NULL;
	}

	CFuzzyRule::CFuzzyRule(const CFuzzyRule &cp) : NLAILOGIC::CondAnd(cp)
	{
		_Threshold = cp._Threshold;

		if ( cp._Comment )
		{
			_Comment = new char[strlen(cp._Comment) + 1];
			strcpy(_Comment, cp._Comment);
		}
		else
			_Comment = NULL;
	}

	CFuzzyRule::CFuzzyRule(const NLAILOGIC::CondAnd &cp) : NLAILOGIC::CondAnd(cp)
	{
	}

	CFuzzyRule::~CFuzzyRule()
	{
		if( _Comment )
			delete[] _Comment;
		// TODO: destruction des concs....
	}

	void CFuzzyRule::addCond(CFuzzyVar *var, char *set_name)
	{
		IFuzzySet *set = var->getSet(set_name);
		if ( set )
		{
			_Vars.push_back( var );
			_Sets.push_back( set );
		}
		else
		{
			// TODO: exception
		}
	}
	
	void CFuzzyRule::addConc(CFuzzyVar *var, IFuzzySet *set)
	{
		_Vars.push_back(var);
		_Sets.push_back(set);
	}

	void CFuzzyRule::addConc(CFuzzyVar *var, char *set_name)
	{
		IFuzzySet *set = var->getSet(set_name);
		if ( set )
		{
			_Vars.push_back(var);
			_Sets.push_back(set);
		}
		// TODO: exception
	}

	void CFuzzyRule::getDebugString(char *txt) const
	{
		char buf_cond[512];
		char buf_conc[512];
		buf_cond[0] = 0;
		buf_conc[0] = 0;

		//FuzzyCond::getDebugString(buf_cond);
		for ( sint32 i = 0; i < (sint32) _Vars.size(); i++ )
		{
			strcat(buf_conc, "(");
			strcat(buf_conc, _Vars[i]->getName().getString() );
			strcat(buf_conc, " is ");
			strcat(buf_conc, _Sets[i]->getName() );
			strcat(buf_conc, ")");
			
			if ( i < (sint32) (_Vars.size() - 1) )
				strcat(buf_conc, " and ");
		}

	/*	if ( _Comment )
			sprintf(txt,"// %s\nif\n %s\n then\n %s", _Comment, buf_cond, buf_conc);
		else*/
			sprintf(txt,"if\n %s\n then\n %s", buf_cond, buf_conc);
	}

	void CFuzzyRule::save(NLMISC::IStream &os)
	{
		// TODO: FuzzyCond::save(os);
		sint32 size = _Vars.size();
		for ( sint32 i = 0; i < size; i++ )
		{
			_Vars[i]->save(os);
			_Sets[i]->save(os);
		}
	}

	void CFuzzyRule::load(NLMISC::IStream &is)
	{
		// TODO:		FuzzyCond::load( is );

		sint32 nb_sets;
		is.serial( nb_sets );
		for (sint32 i = 0; i < nb_sets; i++ )
		{
			NLAIC::CIdentTypeAlloc id_var;
			is.serial( id_var );
			CFuzzyVar *tmp_var = (CFuzzyVar *) id_var.allocClass();
			tmp_var->load( is );
			_Vars.push_back( tmp_var );

			NLAIC::CIdentTypeAlloc id_set;
			is.serial( id_set );
			IFuzzySet *tmp_set = (IFuzzySet *) id_set.allocClass();
			tmp_set->load( is );
			_Sets.push_back( tmp_set );
		}
	}

	const NLAIC::IBasicType *CFuzzyRule::clone() const
	{
		CFuzzyRule *tmp = new CFuzzyRule( *this );
		tmp->incRef();
		return (NLAIC::IBasicType *) tmp;
	}

	const NLAIC::IBasicType *CFuzzyRule::newInstance() const
	{
		CFuzzyRule *tmp = new CFuzzyRule();
		tmp->incRef();
		return (NLAIC::IBasicType *) tmp;
	}

	const NLAIAGENT::IObjectIA::CProcessResult &CFuzzyRule::run()
	{
		double cond_value;
		if ( ( cond_value = truthValue() ) > 0 )
		{
			for ( sint32 i = 0; i < (sint32) _Vars.size(); i++ )
			{
				_Sets[i]->addFact( cond_value );
			}
		}
		return IObjectIA::ProcessRun;
	}

	bool CFuzzyRule::isEqual(const NLAIAGENT::IBasicObjectIA &) const
	{
		// TODO
		return false;
	}

	bool CFuzzyRule::isTrue() const
	{
		return ( truthValue() > _Threshold );
	}

	const NLAIC::CIdentType &CFuzzyRule::getType() const
	{
		return IdFuzzyRule;
	}

	NLAIAGENT::IObjetOp *CFuzzyRule::operator== (NLAIAGENT::IObjetOp &) const
	{
		NLAILOGIC::CBoolFalse *x = new NLAILOGIC::CBoolFalse;
		x->incRef();
		return x;
	}

	void CFuzzyRule::init(NLAIAGENT::IObjectIA *p)
	{		
		NLAILOGIC::CondAnd *cond = (NLAILOGIC::CondAnd *) ((NLAIAGENT::IBaseGroupType *)p)->popFront();
		cond->release();
	}
}
