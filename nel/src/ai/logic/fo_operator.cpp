/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX <MODULE_NAME>.
 * NEVRAX <MODULE_NAME> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX <MODULE_NAME> is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX <MODULE_NAME>; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "nel/ai/logic/fo_operator.h"

#include <list>
#include <vector>
#include "nel/ai/logic/ai_assert.h"
#include "nel/ai/logic/varset.h"
#include "nel/ai/logic/valueset.h"
#include "nel/ai/logic/fact.h"
#include "nel/ai/logic/factbase.h"
#include "nel/ai/logic/fo_assert.h"

namespace NLAILOGIC
{
	using namespace NLAIAGENT;

	CFirstOrderOperator::CFirstOrderOperator()
	{
		_Comment = NULL;
	}
	
	CFirstOrderOperator::CFirstOrderOperator(const CFirstOrderOperator &c)
	{
		if ( c._Comment )
		{
			_Comment = new char[ strlen( c._Comment ) ];
			strcpy( _Comment, c._Comment );
		}
		else
			_Comment = NULL;

		std::vector<IBaseAssert *>::const_iterator it_c = c._Concs.begin();
		while ( it_c != c._Concs.end() )
		{
			_Concs.push_back( *it_c );
			it_c++;
		}

		it_c = c._Conds.begin();
		while ( it_c != c._Conds.end() )
		{
			_Conds.push_back( *it_c );
			it_c++;
		}

		std::vector< std::vector<sint32> >::const_iterator it_li = c._PosVarsCond.begin();
		while ( it_li != c._PosVarsCond.end() )
		{
			std::vector<sint32>::const_iterator it_i = (*it_li).begin();
			_PosVarsCond.push_back( std::vector<sint32>() );

			while ( it_i != (*it_li).end() )
			{
				_PosVarsCond.back().push_back( *it_i ) ;
				it_i++;
			}
			it_li++;
		}

		it_li = c._PosVarsConc.begin();
		while ( it_li != c._PosVarsConc.end() )
		{
			std::vector<sint32>::const_iterator it_i = (*it_li).begin();
			_PosVarsConc.push_back( std::vector<sint32>() );

			while ( it_i != (*it_li).end() )
			{
				_PosVarsConc.back().push_back( *it_i ) ;
				it_i++;
			}
			it_li++;
		}
	}

	/// Looks for a variable in the operato's variables vector and returns its position
	sint32 CFirstOrderOperator::getVarPos(IBaseVar *var)
	{
		if ( _Vars.size() )
		{
			for (sint32 i = 0; i < (sint32)_Vars.size() ; i++ ) 
			{
				if ( var->getName() == _Vars[ i ]->getName() )
				{
					return i;
				}
			}
		}
		return -1;
	}

	CFirstOrderOperator::~CFirstOrderOperator()
	{
		for ( sint32 i = 0; i < (sint32) _Vars.size() ; i++ )
			_Vars[ i ]->release();
	}

	void CFirstOrderOperator::compileFactPattern(CFactPattern *fp, std::vector<IBaseAssert *>&patterns, std::vector<sint32> &pos_Vars)
	{
		// Recherche si variables à ajouter
		std::vector<IBaseVar *> *vars_pattern = fp->getVars();
		if ( vars_pattern )
		{
			std::vector<IBaseVar *>::iterator it_cond = vars_pattern->begin();
			while ( it_cond != vars_pattern->end() )
			{
				sint32 id_var = getVarPos( *it_cond );
				if ( id_var != -1 )
				{
					pos_Vars.push_back( id_var );
				}
				else
				{
					_Vars.push_back( (IBaseVar *)(*it_cond)->clone() );
					pos_Vars.push_back( _Vars.size() - 1);
				}
				it_cond++;
			}
		}

		for ( sint32 i = 0; i < (sint32) vars_pattern->size(); i++ )
		{
			(*vars_pattern)[i]->release();
		}
		delete vars_pattern;
	}

	void CFirstOrderOperator::addPrecondition(CFactPattern *pattern)
	{
		if ( pattern->getAssert() )
		{
			std::vector<sint32> pos_Vars;
			compileFactPattern( pattern, _Conds, pos_Vars);

//			pattern->getAssert()->addClause( this, pos_Vars );
			_Conds.push_back( pattern->getAssert() );
			_PosVarsCond.push_back( pos_Vars );

		}
	}

	void CFirstOrderOperator::addPostcondition(CFactPattern *pattern)
	{
		if ( pattern->getAssert() )
		{
			std::vector<sint32> pos_Vars;
			compileFactPattern( pattern, _Conds, pos_Vars);

			pattern->getAssert()->addInput( this );
			_Concs.push_back( pattern->getAssert() );
			_PosVarsConc.push_back( pos_Vars );
		}
	}


	// Retourne les assertions avec les positions des variables d'une conclusion dans les conditions
	void CFirstOrderOperator::getPosListBackward(sint32 no_conc, sint32 no_cond, std::vector<sint32> &cond_pos)
	{
		std::vector<sint32>::iterator it_conc = _PosVarsConc[ no_conc ].begin();
		while ( it_conc != _PosVarsConc[ no_conc ].end() )
		{
			std::vector<sint32>::iterator it_cond = _PosVarsCond[ no_cond ].begin();
			while ( it_cond != _PosVarsCond[ no_cond ].end() )
			{
				if ( (*it_conc) == (*it_cond) )
				{
					cond_pos.push_back( *it_cond );
				}
				it_cond++;
			}
			it_conc++;
		}
	}

	// Retourne les assertions avec les positions des variables d'une conclusion dans les conditions
	void CFirstOrderOperator::getPosListForward(sint32 no_cond, sint32 no_conc, std::vector<sint32> &conc_pos)
	{
		std::vector<sint32>::iterator it_cond = _PosVarsCond[ no_cond ].begin();
		while ( it_cond != _PosVarsCond[ no_cond ].end() )
		{
			std::vector<sint32>::iterator it_conc = _PosVarsConc[ no_conc ].begin();
			while ( it_conc != _PosVarsConc[ no_conc ].end() )
			{
				if ( (*it_cond) == (*it_conc) )
				{
					conc_pos.push_back( *it_conc );
				}
				it_conc++;
			}
			it_cond++;
		}
	} 

	void CFirstOrderOperator::getAssertPos(IBaseAssert *a, std::vector<IBaseAssert *> &l, std::vector<sint32> &pos)
	{
		for (sint32 i = 0; i < (sint32) l.size() ; i++ )
		{
			if ( (*(l[i])) == a )
				pos.push_back(i);
		}
	}

	std::list<CFactPattern *> *CFirstOrderOperator::forward(CFactPattern *fact)
	{
		return NULL;		
	}

	CValueSet *CFirstOrderOperator::unifyBackward(std::list<CFact *> &facts)
	{
		CValueSet *unified = new CValueSet( _Vars.size() );
		std::list<CFact *>::iterator it_f = facts.begin();
		while ( it_f != facts.end() )
		{
			std::vector<sint32> pos_assert;
			getAssertPos( (*it_f)->getAssert(), _Concs, pos_assert );
			for (sint32 pos = 0; pos < (sint32) pos_assert.size(); pos++)
			{
				for ( sint32 ivar = 0; ivar < (sint32) _PosVarsConc[ pos_assert[pos] ].size(); ivar++ )
				{
					sint32 l_pos = _PosVarsConc[ pos_assert[pos] ][ivar];

					IObjectIA *l_val = (*unified)[ l_pos ]; 
					IObjectIA *r_val = (**it_f)[ ivar ];

					if ( !l_val )
					{
						if ( r_val )
						{
							unified->setValue( l_pos, r_val );
						}
					}
					else
					{
						if ( r_val && ( l_val != r_val ) )
						{
							unified->release();
							return NULL;
						}
					}
				}
			}
			it_f++;
		}
		return unified;
	}

	CValueSet *CFirstOrderOperator::unifyForward(std::list<CFact *> &facts)
	{
		CValueSet *unified = new CValueSet( _Vars.size() );
		std::list<CFact *>::iterator it_f = facts.begin();
		while ( it_f != facts.end() )
		{
			std::vector<sint32> pos_assert;
			getAssertPos( (*it_f)->getAssert(), _Conds, pos_assert );
			for (sint32 pos = 0; pos < (sint32) pos_assert.size(); pos++)
			{
				for ( sint32 ivar = 0; ivar < (sint32) _PosVarsCond[ pos_assert[pos] ].size(); ivar++ )
				{
					sint32 l_pos = _PosVarsCond[ pos_assert[pos] ][ivar];

					IObjectIA *l_val = (*unified)[ l_pos ]; 
					IObjectIA *r_val = (**it_f)[ ivar ];

					if ( !l_val )
					{
						if ( r_val )
						{
							unified->setValue( l_pos, r_val );
						}
					}
					else
					{
						if ( r_val && ( l_val != r_val ) )
						{
							unified->release();
							return NULL;
						}
					}
				}
			}
			it_f++;
		}
		return unified;
	}

	CFact *CFirstOrderOperator::buildFromVars(IBaseAssert *assert, std::vector<sint32> &pl, CValueSet *vars)
	{
		CFact *result = new CFact( assert);	// TODO:: pas besoin du nombre dans ce constructeur puisqu'on a l'assert
		for (sint32 i = 0; i < (sint32) pl.size() ; i++ )
		{
			result->setValue( i, (*vars)[ pl[i] ] );
		}
		return result;
	}

	std::list<CFact *> *CFirstOrderOperator::backward(std::list<CFact *> &facts)
	{
		CValueSet *unified = unifyBackward( facts );
		std::list<CFact *> *result = new std::list<CFact *>;
		for (sint32 i = 0; i < (sint32) _Conds.size(); i++ )
		{
			CFact *tmp = buildFromVars( _Conds[i], _PosVarsCond[i], unified );
			result->push_back( tmp );
#ifdef NL_DEBUG
			std::string buffer;
			tmp->getDebugString(buffer);
#endif
		}
		unified->release();
		return result;
	}

	std::list<CFact *> *CFirstOrderOperator::forward(std::list<CFact *> &facts)
	{
		CValueSet *unified = unifyForward( facts );

#ifdef NL_DEBUG
		std::string buf;
		unified->getDebugString( buf );
#endif

		std::list<CFact *> *result = new std::list<CFact *>;
		for (sint32 i = 0; i < (sint32) _Concs.size(); i++ )
		{
			CFact *tmp = buildFromVars( _Concs[i], _PosVarsConc[i], unified );
			result->push_back( tmp );

#ifdef NL_DEBUG
			std::string buffer;
			tmp->getDebugString(buffer);
#endif
		}
		unified->release();
		return result;
	}


	std::list<CFact *> *CFirstOrderOperator::propagate(std::list<CFact *> &facts)
	{
		std::list<CFact *> *conflicts = new std::list<CFact *>;
		std::list< CValueSet *>	liaisons;
		CValueSet *empty = new CValueSet( _Vars.size() );
		liaisons.push_back( empty );

		std::list<CFact *>::iterator it_f = facts.begin();
		while ( it_f != facts.end() )
		{
			std::vector<sint32> pos_asserts;
			getAssertPos( (*it_f)->getAssert() , _Conds, pos_asserts);
			for (sint32 i = 0; i < (sint32) pos_asserts.size(); i++ )
			{
				std::list<CValueSet *> *links = propagate( liaisons, *it_f, _PosVarsCond[ pos_asserts[i] ] );
				if ( links )
				{
					while ( links->size() )
					{
						for (sint32 i = 0; i < (sint32) _Concs.size(); i++ )
						{
							CFact *r = buildFromVars( _Concs[i], _PosVarsConc[i], links->front() );
#ifdef NL_DEBUG
							std::string buf;
							r->getDebugString( buf );
#endif
							// Tests if the fact is already in the conflicts list
							bool found = false;
							std::list<CFact *>::iterator it_c = conflicts->begin();
							while ( ! found && it_c != conflicts->end() )
							{
								found = (**it_c) == *r;
								it_c++;
							}
							if ( !found )
							{
#ifdef NL_DEBUG
								std::string buf;
								r->getDebugString( buf );
#endif
								conflicts->push_back( r );
							}
						}
						links->front()->release();
						links->pop_front();
					}
					delete links;
				}
			}
			it_f++;
		}

		while ( liaisons.size() )
		{
			liaisons.front()->release();
			liaisons.pop_front();
		}

		return conflicts;
	}

	CValueSet *CFirstOrderOperator::unifyLiaison( const CValueSet *fp, CValueSet *vals, std::vector<sint32> &pos_vals)
	{
		CValueSet *result;

		if ( (result = fp->unify( vals, pos_vals )) )
			return result;
		else
		{
			delete result;
			return NULL;
		}
	}

	std::list<CValueSet *> *CFirstOrderOperator::propagate(std::list<CValueSet *> &liaisons, CValueSet *fact, std::vector<sint32> &pos_vals) 
	{
		std::list<CValueSet *> *conflits = new std::list<CValueSet *>;
		std::list<CValueSet *> buf_liaisons;
		// Pour chaque liaison...
		std::list< CValueSet *>::iterator it_l = liaisons.begin();
		
		while ( it_l != liaisons.end() )
		{

			CValueSet *l = *it_l;
#ifdef NL_DEBUG
			std::string buf;
			l->getDebugString( buf );
#endif

			CValueSet *result = unifyLiaison( l, fact, pos_vals );
			if ( result )
			{
#ifdef NL_DEBUG
				std::string buf;
				result->getDebugString( buf );
#endif

				if ( result->undefined() == 0 )
				{
					conflits->push_back( result );
				}
				else 
					buf_liaisons.push_back( result );
			}
			it_l++;
		}

		while ( buf_liaisons.size() )
		{
			liaisons.push_back( buf_liaisons.front() );
			buf_liaisons.pop_front();
		}

		return conflits;
	}	

	std::list<CFact *> *CFirstOrderOperator::test(std::list<CFact *> &facts)
	{
		std::list<CFact *> *preconds = backward( facts );

		std::list<CFact *>::iterator it_f = preconds->begin();
		while ( it_f != preconds->end() )
		{
			( (CFirstOrderAssert *)(*it_f)->getAssert() )->backward( *it_f );
			it_f++;
		}
		return NULL;
	}


	const NLAIC::IBasicType *CFirstOrderOperator::clone() const
	{
		CFirstOrderOperator *clone = new CFirstOrderOperator( *this );
		return clone;
	}

	const NLAIC::IBasicType *CFirstOrderOperator::newInstance() const
	{
		CFirstOrderOperator *instance = new CFirstOrderOperator;
		return instance;
	}

	void CFirstOrderOperator::save(NLMISC::IStream &os)
	{
	}

	void CFirstOrderOperator::load(NLMISC::IStream &is)
	{
	}

	void CFirstOrderOperator::getDebugString(std::string &txt) const
	{
		txt += "Operator:\n   -Preconditions:\n";
		if ( _Comment )
		{
			txt += _Comment;
			txt += "\n";
		}

		std::vector<IBaseAssert *>::const_iterator it_a = _Conds.begin();
		std::vector<std::vector<sint32> >::const_iterator it_p = _PosVarsCond.begin();
		while ( it_a != _Conds.end() )
		{
			std::string buf;
			(*it_a)->getDebugString(buf);
			txt += "    ( ";
			txt += buf;
			for (sint32 i = 0; i < (sint32) (*it_p).size(); i++ )
			{
				txt += _Vars[ (*it_p)[i] ]->getName().getString();
				txt += " ";
			}
			txt += ")\n";

			it_a++;
			it_p++;
		}

		txt += "  -Postconditions:\n";
		it_a = _Concs.begin();
		it_p = _PosVarsConc.begin();
		while ( it_a != _Concs.end() )
		{
			std::string buf;
			(*it_a)->getDebugString(buf);
			txt += "    ( ";
			txt += buf;
			for (sint32 i = 0; i < (sint32) (*it_p).size(); i++ )
			{
				txt += _Vars[ (*it_p)[i] ]->getName().getString();
				txt += " ";
			}
			txt += ")\n";

			it_a++;
			it_p++;
		}
	}

	bool CFirstOrderOperator::isEqual(const CFirstOrderOperator &a) const
	{
		return false;
	}

	const IObjectIA::CProcessResult &CFirstOrderOperator::run()
	{		
		return IObjectIA::ProcessRun;
	}

	bool CFirstOrderOperator::isEqual(const IBasicObjectIA &a) const
	{
		return false;		
	}

	bool CFirstOrderOperator::isTrue() const
	{
		return false;
	}

	const NLAIC::CIdentType &CFirstOrderOperator::getType() const
	{
		return IdCFirstOrderOperator;
	}

	bool CFirstOrderOperator::operator==(CFirstOrderOperator *)
	{
		return false;
	}

	float CFirstOrderOperator::truthValue() const
	{
		return 0.0;
	}
	
	// Vérifie si l'opérateur est activable avec les informations d'une base de faits
	bool CFirstOrderOperator::isValid(CFactBase *fb)
	{		
		std::list<CFact *> *facts = new std::list<CFact *>;
		for (sint32 i = 0; i < (sint32) _Conds.size(); i++ )
		{
			std::list<CFact *> *fa = fb->getAssertFacts( _Conds[i] );
			while ( fa->size() )
			{
				facts->push_back( fa->front() );
				fa->pop_front();
			}
			delete fa;
		}
		std::list<CFact *> *res = propagate( *facts );
		bool is_valid = !res->empty();
		while ( res->size() )
		{
#ifdef NL_DEBUG
			std::string buffer;
			res->front()->getDebugString( buffer );
#endif
			res->front()->release();
			res->pop_front();
		}
		delete res;

		while ( facts->size() )
		{
			facts->front()->release();
			facts->pop_front();	
		}
		delete facts;

		return is_valid;
	}

	float CFirstOrderOperator::priority() const
	{
		return 0.0;
	}

	void CFirstOrderOperator::success()
	{
	}

	void CFirstOrderOperator::failure()
	{
	}

	void CFirstOrderOperator::success(IBaseOperator *)
	{
	}

	void CFirstOrderOperator::failure(IBaseOperator *)
	{
	}

}
