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

#include "nel/ai/logic/rule.h"
#include "nel/ai/logic/ai_assert.h"
#include "nel/ai/logic/valueset.h"
#include "nel/ai/logic/fo_assert.h"

namespace NLAILOGIC
{
	using namespace NLAIAGENT;

	CRule::CRule()
	{
	}

	CRule::CRule(std::list<CFactPattern *> &conds, std::list<CFactPattern *> &concs) : CClause( conds )
	{
		std::list<CFactPattern *>::iterator it_c = concs.begin();
		while ( it_c != concs.end() )
		{
			addConc( *it_c );
			it_c++;
		}
	}

	CRule::CRule(const CRule &c) : CClause(c)
	{
		std::vector<IBaseAssert *>::const_iterator it_c = c._Concs.begin();
		while ( it_c != c._Concs.end() )
		{
			_Concs.push_back( *it_c );
			it_c++;
		}

		std::vector< std::vector<sint32> >::const_iterator it_li = c._PosVarsConc.begin();
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

	void CRule::setCond(CClause &c)
	{
/*		std::list< std::vector<sint32> >::const_iterator it_li = c._pos_vars.begin();
		while ( it_li != c._pos_vars.end() )
		{
			std::vector<sint32>::const_iterator it_i = (*it_li).begin();
			_pos_vars.push_back( std::vector<sint32>() );

			while ( it_i != (*it_li).end() )
			{
				_pos_vars.back().push_back( *it_i ) ;
				it_i++;
			}
			it_li++;
		}*/
	}

	CRule::~CRule()
	{
	}

	const NLAIC::IBasicType *CRule::clone() const
	{
		NLAIC::IBasicInterface *m = new CRule(*this);
		return m;
	}

	const NLAIC::IBasicType *CRule::newInstance() const
	{
		CRule *instance = new CRule();
		return instance;
	}


	void CRule::addConc(CFactPattern *conc)
	{
		_Concs.push_back( conc->getAssert() );

		// Recherche l'assertion
		sint32 pos_assert= findAssert( conc->getAssert() );
		if ( pos_assert  < 0 )
		{
			_Asserts.push_back( conc->getAssert() );
			pos_assert = _Asserts.size();
		}

		// Recherche si variables à ajouter
		std::vector<sint32> pos_vars;
		std::vector<IBaseVar *> *vars_conc = conc->getVars();
		if ( vars_conc )
		{
			std::vector<IBaseVar *>::iterator it_conc = vars_conc->begin();
			while ( it_conc != vars_conc->end() )
			{
				bool found;
				if ( _Vars.size() )
				{
					found = false;
					for (sint32 i = 0; i < (sint32)_Vars.size() ; i++ ) 
					{
						if ( ( *it_conc )->getName() == _Vars[ i ]->getName() )
						{
							found = true;
							pos_vars.push_back( i );
						}
					}
				}
				else
					found = false;
				
				if ( !found ) 
				{
					_Vars.push_back( (IBaseVar *)(*it_conc)->clone() );
					pos_vars.push_back( _Vars.size() - 1);
				}
				it_conc++;
			}
		}

		// Prévenir l'assertion
		if ( conc->getAssert() )
		{
			((CFirstOrderAssert *)conc->getAssert())->addInput( this, pos_vars );
			_PosVarsConc.push_back( pos_vars );
		}

		for ( sint32 i = 0; i < (sint32) vars_conc->size(); i++ )
		{
			(*vars_conc)[i]->release();
		}
		delete vars_conc;
	}

	void CRule::forward(CVarSet *fp)
	{
		
	}

	void CRule::addConflicts()
	{
		// Execute la conclusion de la règle
		std::list<CValueSet *>::iterator it_conf = _Conflits.begin();
		while ( it_conf != _Conflits.end() )
		{
			sint32 pos = 0;
			std::vector<IBaseAssert *>::iterator it_conc = _Concs.begin();
#ifdef NL_DEBUG
			std::vector< std::vector<sint32> >::iterator it_pos = _PosVarsConc.begin();
#endif
			while ( it_conc != _Concs.end() )
			{
				// Construire le fait correspondant par rapport à la position des variables
				
				/////////////////////
#ifdef NL_DEBUG
				CValueSet *conflit = new CValueSet( *it_conf , *it_pos );
				std::string buf;
				std::string buf2;
				(*it_conc)->getDebugString( buf );
				conflit->getDebugString( buf2 );
#endif
				//TRACE("\nCONCLUSION DE LA REGLE: \n   ASSERTION: %s   VALEURS: %s\n", buf, buf2);
				/////////////////////
				it_conc++;
			}
			it_conf++;
			pos++;
		}
	}

	void CRule::init(IObjectIA *params)
	{
/*		if ( params->size() != 3 )
		{
			// TODO throw Exc::....
		}*/

		// Conditions
		CClause *cond = (CClause *) ((IBaseGroupType *)params)->popFront();
		std::vector<IBaseVar *> *vars = cond->getVars();

		int i;

		for ( i = 0; i < (sint32)vars->size() ; i++ )
		{
			_Vars.push_back( (IBaseVar *) (* vars)[ i ]->clone() );
		}
		
		for ( i = 0; i < (sint32) vars->size() ; i++ )
			(*vars)[i]->release();
		delete vars;
		cond->release();

		// Conclusions
		IBaseGroupType *concs = (IBaseGroupType *) ((IBaseGroupType *)params)->popFront();
		CIteratorContener it_fp = concs->getIterator();
		while ( !it_fp.isInEnd() )
		{
			CFactPattern *tmp = (CFactPattern *) ( it_fp ++)->clone();
			addConc( tmp);
			tmp->release();
			//it_fp++;
		}

		concs->release();
	}

	const NLAIC::CIdentType &CRule::getType() const
	{
		return IdRule;
	}

	void CRule::getDebugString(std::string &txt) const
	{		
		txt += "CRule\n - Conditions: ";
		std::vector<IBaseBoolType *>::const_iterator it_c = _Conds.begin();
		while ( it_c != _Conds.end() )
		{
			std::string buf;
			( *it_c )->getDebugString( buf );
			txt += "    - ";
			txt += buf;
			it_c++;
		}

		txt += "\n - Conclusions: ";
		
		std::vector<IBaseAssert *>::const_iterator it_cc = _Concs.begin();

		while ( it_cc != _Concs.end() )
		{
			std::string buf;
			( *it_cc )->getDebugString( buf );
			txt += "    - ";
			txt += buf;
			it_cc++;
		}

		txt += "\n - Variables ";
		for (sint32 i = 0; i < (sint32)_Vars.size(); i++ )
		{
			std::string buf;
			_Vars[ i ]->getDebugString( buf );
			txt += "    - ";
			txt += buf;
		}
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Chainage arrière


	CVarSet *CRule::backWard(CFactPattern *fp)
	{
		// Vecteur temporaire des variables
		CVarSet *tmp_vars = new CVarSet;
		for (sint32 i = 0; i < (sint32) _Vars.size() ; i++)
			tmp_vars->addVar( _Vars[i] );

		// Trouver l'assertion dans la conclusion
		std::vector<IBaseAssert *>::iterator it_a = _Concs.begin();
		std::vector< std::vector<sint32> >::iterator it_p = _PosVarsConc.begin();
		while ( it_a != _Concs.end() && (*it_a) != fp->getAssert() )
		{
			it_a++;
			it_p++;
		}
		if ( it_a == _Concs.end() )
			return NULL;

		std::vector<sint32> pos_list = *it_p;
		std::vector<sint32>::iterator it_pos = pos_list.begin();
	
		// Unification avec la valeur des variables
		sint32 index = 0;
		while ( it_pos != pos_list.end() )
		{
			if ( (*tmp_vars->getVars())[*it_pos]->getValue() == NULL)
				(*tmp_vars)[*it_pos]->setValue( (*fp)[index]->getValue() );
			else
			{
				return NULL;
			}
			it_pos++;
			index++;
		}
		return tmp_vars;
	}
 
	// Retourne une liste de faits complets
	std::list<CValueSet *> *CRule::unifyBack(CFactPattern *fp)
	{
		std::list<CValueSet *> *unified_list = new std::list<CValueSet *>;
		
		// Vecteur temporaire des variables
		CVarSet *tmp_vars = new CVarSet;
		for (sint32 i = 0; i < (sint32) _Vars.size() ; i++)
			tmp_vars->addVar( _Vars[i] );

		// Trouver l'assertion dans la conclusion
		std::vector<IBaseAssert *>::iterator it_a = _Concs.begin();
		std::vector< std::vector<sint32> >::iterator it_p = _PosVarsConc.begin();
		while ( it_a != _Concs.end() && (*it_a) != fp->getAssert() )
		{
			it_a++;
			it_p++;
		}
		if ( it_a == _Concs.end() )
			return NULL;

		// Créé la liste des valeurs 
		std::list<IObjetOp *> *vals = fp->getValues();

		// Pour chaque liaison...
		std::list< CValueSet *>::iterator it_l = _Liaisons.begin();
		
		while ( it_l != _Liaisons.end() )
		{
			CValueSet *l = *it_l;
			bool complete;
			CValueSet *result = unifyLiaisonBack( l, *vals, *it_p, complete );
			if ( result )
			{
				if ( complete )
				{
					// Ceux là on les rajoute direct dans la liste
					//char test[1024 * 2];
					//result->getDebugString( test );
					CValueSet *n = fp->asCValueSet();
					sint32 pos_r = 0;
					std::vector<sint32>::iterator it_i = (*it_p).begin();
					while ( it_i != (*it_p).end() )
					{
#ifdef NL_DEBUG
						std::string buf;
						(*result)[ *it_i ]->getDebugString(buf);
#endif

						n->setValue( pos_r, (*result)[ *it_i ] );
						it_i++;
						pos_r++;
					}
					unified_list->push_back( n );
				}
#ifdef NL_DEBUG
				else 
				{
					// Ceux là on continue à les propager en arrière!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

					std::string test;
					result->getDebugString( test );
				}
#endif
			}
			it_l++;
		}
		return unified_list;
	}

	void CRule::backward( std::list<CValueSet *> *vs )
	{
		std::vector<IBaseBoolType *>::iterator it_cond = _Conds.begin();
		while ( it_cond != _Conds.end() )
		{
			std::list<CValueSet *>::iterator it_vs = vs->begin();
			while ( it_vs != vs->end() )
			{
//				( (CFirstOrderAssert *) *it_cond )->backward( *it_vs );
				it_vs++;
			}
			it_cond++;
		}
	}

	CValueSet *CRule::unifyLiaisonBack(CValueSet *liaison, std::list<IObjetOp *> &vals, std::vector<sint32> &pos, bool &defined)
	{
		sint32 nb_undefined = pos.size();
		CValueSet *unified = new CValueSet( *liaison );
		std::vector<sint32>::iterator it_pos = pos.begin();
		std::list<IObjetOp *>::iterator it_v = vals.begin();
		while ( it_pos != pos.end() )
		{
			IObjectIA *l_val = (*unified)[*it_pos];
			IObjectIA *r_val = *it_v;
			if ( !l_val )
			{
/*				if ( r_val )
				{
					unified->setValue( p , r_val );
					nb_undefined--;
				}*/
			}
			else
			{
				if ( r_val && ( l_val != r_val ) )
				{
					unified->release();
					return NULL;
				}
				else 
					if ( r_val ) 
						nb_undefined--;
			}
			it_pos++;
			it_v++;
		}
		defined = ( nb_undefined == 0 );

#ifdef NL_DEBUG
		std::string buf;
		unified->getDebugString(buf);
#endif


		return unified;
	}

	// Retourne les assertions avec les positions des variables d'une conclusion dans les conditions
	void CRule::getPosListBackward(sint32 no_conc, sint32 no_cond, std::vector<sint32> &cond_pos)
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
	void CRule::getPosListForward(sint32 no_cond, sint32 no_conc, std::vector<sint32> &conc_pos)
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
}
