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

#include "nel/ai/logic/clause.h"
#include "nel/ai/logic/boolval.h"
#include "nel/ai/logic/ai_assert.h"
#include "nel/ai/logic/valueset.h"
#include "nel/ai/logic/fo_assert.h"

namespace NLAILOGIC
{
	using namespace NLAIAGENT;

	CClause::CClause()
	{
		_Liaisons.push_back( new CValueSet( _Vars.size() ) );
		_NbValid = 0;
	}

	CClause::CClause(std::list<CFactPattern *> &conds) : CVarSet()
	{
		_NbValid = 0;
		std::list<CFactPattern *>::iterator it_c = conds.begin();
		while ( it_c != conds.end() )
		{
			addCond( (CFactPattern *) (*it_c)->clone() );
			it_c++;
		}
		_Liaisons.push_back( new CValueSet( _Vars.size() ) );
	}

	CClause::CClause(const CClause &cl) : CVarSet(cl)

	{
		_NbValid = cl._NbValid;
		std::vector<IBaseBoolType *>::const_iterator it_c = cl._Conds.begin();
		while ( it_c != cl._Conds.end() )
		{
			addCond( (CFactPattern *) *it_c );
			it_c++;
		}

		std::vector< std::vector<sint32> >::const_iterator it_li = cl._PosVarsCond.begin();
		while ( it_li != cl._PosVarsCond.end() )
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
	}

	CClause::~CClause()
	{
		for ( sint32 i = 0; i < (sint32) _Conds.size(); i++ )
			_Conds[i]->release();
		
		while ( ! _Conflits.empty() )
		{
			delete _Conflits.front();
			_Conflits.pop_front();
		}

		while ( ! _Liaisons.empty() )
		{
			delete _Liaisons.front();
			_Liaisons.pop_front();
		}

		while ( ! _BufLiaisons.empty() )
		{
			delete _BufLiaisons.front();
			_BufLiaisons.pop_front();
		}
	}

	const NLAIC::IBasicType *CClause::clone() const
	{
		CClause *clone = new CClause( *this );
		return clone;
	}	
	
	const NLAIC::IBasicType *CClause::newInstance() const
	{
		CClause *instance = new CClause();
		return instance;
	}

	sint32 CClause::findAssert(IBaseAssert *a)
	{
		if ( a )
			for ( sint32 i = 0; i < (sint32)_Asserts.size(); i++ )
			{
				if ( _Asserts[i]->getName() == a->getName() )
				{
					return i;
				}
			}

		return -1;
	}

	void CClause::addCond(CFactPattern *cond)
	{
		_Conds.push_back( (CFactPattern *) cond->clone() );

		// Recherche l'assertion
		sint32 pos_assert= findAssert( cond->getAssert() );
		if ( pos_assert  < 0 )
		{
			_Asserts.push_back( cond->getAssert() );
			pos_assert = _Asserts.size();
		}

		// Recherche si variables à ajouter
		std::vector<sint32> pos_vars;
		std::vector<IBaseVar *> *vars_cond = cond->getVars();
		if ( vars_cond )
		{
			std::vector<IBaseVar *>::iterator it_cond = vars_cond->begin();
			while ( it_cond != vars_cond->end() )
			{
				bool found;
				if ( _Vars.size() )
				{
					found = false;
					for (sint32 i = 0; i < (sint32)_Vars.size() ; i++ ) 
					{
						if ( ( *it_cond )->getName() == _Vars[ i ]->getName() )
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
					_Vars.push_back( (IBaseVar *)(*it_cond)->clone() );
					pos_vars.push_back( _Vars.size() - 1);
				}
				it_cond++;
			}
		}

		// Prévenir l'assertion
		if ( cond->getAssert() )
		{
			((CFirstOrderAssert *) cond->getAssert())->addClause( this, pos_vars );
			_PosVarsCond.push_back( pos_vars );
		}

		for ( sint32 i = 0; i < (sint32) vars_cond->size(); i++ )
		{
			(*vars_cond)[i]->release();
		}
		delete vars_cond;
	}

	void CClause::getDebugString(std::string &txt) const
	{		
		txt += "CClause ";
		for (sint32 i = 0; i < (sint32)_Vars.size(); i++ )
		{
			std::string buf;
			_Vars[ i ]->getDebugString( buf );
			txt += "  - ";
			txt += buf;
		}
	}

	void CClause::propagate(CFactPattern *fp)
	{
		// Pour chaque liaison...
/*		std::list< CValueSet *>::iterator it_l = _Liaisons.begin();
		
		while ( it_l != _Liaisons.end() )
		{
			CValueSet *result;
			if ( result = fp->unify( *it_l ) )
			{
				if ( result->undefined() == 0 )
					_Conflits.push_back( result );
				else 
					_BufLiaisons.push_back( result );
			}
			it_l++;
		}*/
	}
	
	void CClause::propagate(std::list<IObjectIA *> *vals, std::vector<sint32> &pos_vals) 
	{
		// Pour chaque liaison...
		std::list< CValueSet *>::iterator it_l = _Liaisons.begin();
		
		while ( it_l != _Liaisons.end() )
		{
//			char buf[512];
			CValueSet *l = *it_l;
/*			l->getDebugString( buf );
			TRACE( "Pour liaison: %s \n", buf );*/
			CValueSet *result = unifyLiaison( l, vals, pos_vals );
			if ( result )
			{

/*				char buf[512];
				result->getDebugString( buf );
				TRACE( "Unification: %s \n", buf );*/

				if ( result->undefined() == 0 )
				{
					_Conflits.push_back( result );
					_NbValid++;
				}

				else 
					_BufLiaisons.push_back( result );
			}
			it_l++;
		}

		addBuffer();
		addConflicts();

	}	

	// Tente d'unifier deux instanciations partielles des variables de la CClause
	CValueSet *CClause::unifyLiaison( const CValueSet *fp, std::list<IObjectIA *> *vals, std::vector<sint32> &pos_vals)
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

	IObjetOp *CClause::operator ! () const
	{

		return new CBoolType( !isTrue() );
	}

	IObjetOp *CClause::operator != (IObjetOp &a) const
	{
		return new CBoolType( isTrue() != a.isTrue() );
	}

	bool CClause::isTrue() const
	{
		return ( _NbValid > 0 );
	}

	float CClause::truthValue() const
	{
		if ( _NbValid > 0 )
			return 1.0;
		else
			return 0.0;
	}

	IObjetOp *CClause::operator == (IObjetOp &a) const
	{
		return new CBoolType( isTrue() == a.isTrue() );
	}

	void CClause::showConflicts()
	{
		std::list<CValueSet *>::iterator it_vs = _Conflits.begin();
		while ( it_vs != _Conflits.end() )
		{
			std::string buf;
			( *it_vs )->getDebugString( buf );
			it_vs++;
		}
	}

	void CClause::showBuffer()
	{
		std::list<CValueSet *>::iterator it_vs = _BufLiaisons.begin();
		while ( it_vs != _BufLiaisons.end() )
		{
			std::string buf;
			( *it_vs )->getDebugString( buf );			
			it_vs++;
		}
	}

	void CClause::showLiaisons()
	{
		std::list<CValueSet *>::iterator it_vs = _Liaisons.begin();
		while ( it_vs != _Liaisons.end() )
		{
			std::string buf;
			( *it_vs )->getDebugString( buf );			
			it_vs++;
		}
	}


	// Transfert des liaisons du buffer à la liste des liaisons
	void CClause::addBuffer()
	{
		while ( !_BufLiaisons.empty() )
		{
			_Liaisons.push_back( _BufLiaisons.front() );
			std::string buf;
			_BufLiaisons.front()->getDebugString( buf );
			//TRACE ( "Ajout de la liaison du buffer: %s ", buf );
			_BufLiaisons.pop_front();
		}
	}
	
	// Traitement des conflits
	void CClause::addConflicts()
	{
	}

	// Renvoie les assertions des conditions de la CClause
	std::vector<IBaseAssert *> &CClause::getAssert()
	{
		return _Asserts;
	}

	CVarSet *CClause::unify(CVarSet *)
	{
		// TODO:
/*
		std::list<CValueSet *>::iterator it_l = _Liaisons.begin();
		while ( it_cond != _Conds.end() )
		{
			CVarSet *tmp = ( *it_cond )->unify( fact );
			
			if ( !tmp )
				return;
			it_cond++;
		}
		*/
		return NULL;
	}

	std::list<CClause *> *CClause::getInputs()
	{
		return new std::list<CClause *>;
	}

	sint32 CClause::nbVars()
	{
		return _Vars.size();
	}

	void CClause::init(IObjectIA *params)
	{
/*		if ( params->size() != 3 )
		{
			// TODO throw Exc::....
		}*/

		// Conditions
		CIteratorContener it_fp = ((IBaseGroupType *)params)->getIterator();
		while ( !it_fp.isInEnd())
		{
			addCond( (CFactPattern *) it_fp++ );			
		}
	}

/*	const std::vector< std::list<sint32> > &CClause::getPosVar()
	{
		return _pos_vars;
	}
	*/
}
