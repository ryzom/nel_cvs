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


#include "nel/ai/logic/fo_assert.h"
#include "nel/ai/logic/fo_operator.h"
#include "nel/ai/logic/valueset.h"
#include "nel/ai/logic/varset.h"
#include "nel/ai/logic/fact.h"
#include "nel/ai/logic/clause.h"
#include "nel/ai/agent/agent_digital.h"

namespace NLAILOGIC
{
	using namespace NLAIAGENT;

	CFirstOrderAssert::CFirstOrderAssert(const IVarName &n, sint32 nb_vars) : IBaseAssert( n )
	{
		_NbVars = nb_vars;
/*		std::vector<IBaseVar *> *lv = new std::vector<IBaseVar *>;
		for (sint32 i = 0; i < _NbVars; i++)
			lv->pushBack(NULL);*/
	}


	CFirstOrderAssert::CFirstOrderAssert(const CFirstOrderAssert &cp) : IBaseAssert( *cp._Name )
	{

		CConstIteratorContener r = _Facts.getConstIterator();
		while( !r.isInEnd() )
		{
			_Facts.pushBack( 50, (CValueSet *) (*r).clone() );
		}
	}

	CFirstOrderAssert::~CFirstOrderAssert()
	{
		while ( _Facts.size() )
		{
			_Facts.Front()->release();
			_Facts.popFront();
		}
	}

	void CFirstOrderAssert::connectClause(CClause *, std::vector<sint32> &)
	{
	}

	void CFirstOrderAssert::addFact(CVarSet *f)
	{
		// Lorsqu'on ajoute un fait, le transmet à toutes les conditions qui en dépendent avec la position des variables dans la condition
		for ( sint32 i = 0; i < (sint32)_Outputs.size(); i++ )
		{
			// Créé la liste des valeurs
			std::list<IObjetOp *> *values = f->getValues();	

//			TODO: unifier pour bool et first order operator
//			(CFirstOrderOperator *) _Outputs[i]->propagate(values, _PosVars[i] );
			delete values;
		}
		
		_Facts.pushBack( 50, f->asCValueSet() );
	}

	void CFirstOrderAssert::addFact(CValueSet *f)
	{
		// Lorsqu'on ajoute un fait, le transmet à toutes les conditions qui en dépendent avec la position des variables dans la condition
		for ( sint32 i = 0; i < (sint32)_Outputs.size(); i++ )
		{
			// Créé la liste des valeurs
//			std::list<IObjectIA *> *values = f->getValues();
//			TODO: unifier pour bool et first order operator			
//			_Outputs[i]->propagate(values, _PosVars[i] );
		}

		_Facts.pushBack( 50, (CValueSet *) f->clone() );
	}

	void CFirstOrderAssert::removeFact(CFact *f)
	{
		std::vector<IBaseOperator *>::iterator it_cl = _Outputs.begin();
		while ( it_cl != _Outputs.end() )
		{
			// doit passer par un envoi de message!!!
			it_cl++;
		}

		CValueSet *tmp = f->asValueSet();

		CConstIteratorContener it_f = _Facts.getConstIterator();
		while( !it_f.isInEnd() )
		{
			if ( *f == *(CValueSet *)((const IObjetOp*)it_f) )
			{
				_Facts.erase( it_f );
				tmp->release();
				return;
			}
			it_f++;
		}
		tmp->release();
	}

	void CFirstOrderAssert::addClause(CClause *clause, std::vector<sint32> &posvars) 
	{
		_Clauses.push_back( clause );
		_PosVars.push_back(std::vector<sint32>());
		
		std::vector<sint32>::iterator it_pos = posvars.begin();
		while ( it_pos != posvars.end() )
		{
			_PosVars[ _Clauses.size() -1 ].push_back( *it_pos );
			it_pos++;
		}
	}

	const NLAIC::IBasicType *CFirstOrderAssert::clone() const
	{
		CFirstOrderAssert *clone = new CFirstOrderAssert( *this );		
		return (NLAIC::IBasicInterface *) clone;
	}

	const NLAIC::IBasicType *CFirstOrderAssert::newInstance() const
	{
		return clone();
	}
		
	void CFirstOrderAssert::save(NLMISC::IStream &os)
	{
	}

	void CFirstOrderAssert::load(NLMISC::IStream &is)
	{
	}

	void CFirstOrderAssert::getDebugString(std::string &text) const
	{
		std::string buf;
		getName().getDebugString(buf);
		text += NLAIC::stringGetBuild("<CFirstOrderAssert> %s", buf.c_str());
	}

	bool CFirstOrderAssert::isEqual(const CFirstOrderAssert &a) const
	{
		return ( getName() == a.getName() );
	}

	const IObjectIA::CProcessResult &CFirstOrderAssert::run()
	{
		return IObjectIA::ProcessRun;
	}

	bool CFirstOrderAssert::isEqual(const IBasicObjectIA &a) const
	{
		return false;
	}

	bool CFirstOrderAssert::isTrue() const
	{
		return false;
	}

	void CFirstOrderAssert::addInput(CClause *clause, std::vector<sint32> &posvars)
	{
		_Clauses.push_back( clause );
		_PosVarsInputs.push_back( std::vector<sint32>() );
		
		std::vector<sint32>::iterator it_pos = posvars.begin();
		while ( it_pos != posvars.end() )
		{
			_PosVarsInputs[_Inputs.size() - 1].push_back( *it_pos );
			it_pos++;
		}
	}

	const std::vector<CClause *> &CFirstOrderAssert::getClauses()
	{
		return _Clauses;
	}

	const NLAIC::CIdentType &CFirstOrderAssert::getType() const
	{
		return IdFirstOrderAssert;
	}

	void CFirstOrderAssert::init(IObjectIA *params)
	{
		((IBaseGroupType *)params)->popFront();
		
		_NbVars = (sint32) ( (DigitalType *) ((IBaseGroupType *)params)->popFront() )->getValue();
	}

	sint32 CFirstOrderAssert::nbVars() const
	{
		return _NbVars;
	}

	void CFirstOrderAssert::backward(CValueSet *vs, std::list<CValueSet *> &bindings)
	{
		/*		std::list<CValueSet *> *facts = new std::list<CValueSet *>;
	
		CFactPattern *tmp_fp = new CFactPattern(*vs);
		tmp_fp->setAssert( this );

		std::list<CClause *>::iterator it_cl = _Inputs.begin();
		while ( it_cl != _Inputs.end() )
		{
			CVarSet *l = ( (CRule *) *it_cl )->backward( tmp_fp );
			if ( l )
			{
				char buf[1024 * 2];
				l->getDebugString( buf );
				bindings.pushBack( l );
			}
			it_cl++;
		}*/
	}

	std::list<CFact *> *CFirstOrderAssert::backward(CFact *fact)
	{
		// Recherche dans l'assertion...
		std::list<CFact *> *result = new std::list<CFact *>;
		CValueSet *liaison;
/*
		CConstIteratorContener it_f = _Facts.getConstIterator();
		while( !it_f.isInEnd() )
		{
			if ( *f == *(CFact *)((const IObjetOp*)it_f) )
			{
				_Facts.erase( it_f );
				tmp->release();
				return;
			}
			it_f++;
		}
*/

		CConstIteratorContener it_l = _Facts.getConstIterator();
		while ( !it_l.isInEnd() )
		{
#ifdef NL_DEBUG
			std::string buffer;

			((CValueSet *)((const IObjetOp*)it_l))->getDebugString(buffer);
			std::string buffer2;
			fact->getDebugString(buffer2);
#endif

			liaison = ((CValueSet *)((const IObjetOp*)it_l))->unify( (CValueSet *) fact );
			if ( liaison )
			{
				if ( liaison->undefined() == 0 )
				{
					result->push_back( (CFact *) ((CValueSet *)((const IObjetOp*)it_l))->clone());
				}
				liaison->release();
			}
			it_l++;
		}


		// Et dans les règles dont la partie conclusion infèrent des faits de cette assertion
		std::list<CFact *> param;
		param.push_back( fact );
		std::vector<IBaseOperator *>::iterator it_i = _Inputs.begin();
		while ( it_i != _Inputs.end() )
		{
			((CFirstOrderOperator *)(*it_i))->backward( param );
			it_i++;
		}

		return result;
	}

	std::list<CFact *> *CFirstOrderAssert::getFacts() const
	{
		std::list<CFact *> *result = new std::list<CFact *>;
		CConstIteratorContener it_f = _Facts.getConstIterator();
		while ( !it_f.isInEnd() )
		{
			CFact *tmp = new CFact( (IBaseAssert *) this, ((CValueSet *)((const IObjetOp*)it_f)) );
			result->push_back( tmp );
			it_f++;
		}
		return result;
	}
}

