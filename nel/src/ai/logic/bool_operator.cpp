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

#include "nel/ai/logic/bool_operator.h"
#include "nel/ai/logic/fact.h"

namespace NLAILOGIC
{
	using namespace NLAIAGENT;

	CBoolOperator::CBoolOperator()
	{
	}

	CBoolOperator::CBoolOperator(char *c) : IBaseOperator(c)
	{
	}

	CBoolOperator::CBoolOperator(CBoolOperator &c) : IBaseOperator(c)
	{
	}

	bool CBoolOperator::isTrue() const 
	{
		for (sint32 i = 0; i < (sint32) _Conds.size(); i++ )
		{
			if ( _CondsVal[i] != _Conds[i]->isTrue() )
				return false;
		}
		return true;
	}

	std::list<CFact *> *CBoolOperator::backward(std::list<IBaseAssert *> &conds, std::list<bool> &vals)
	{
		std::list<CFact *> *result = new std::list<CFact *>;
		for ( sint32 i = 0; i < (sint32) _Conds.size(); i++ )
		{
			CFact *tmp = new CFact (_Conds[i], _CondsVal[i] );
			result->push_back( tmp );
		}
		return result;
	}

	void CBoolOperator::save(NLMISC::IStream &os)
	{
		IBaseOperator::save( os );
		
		sint32 i;
		sint32 size = _Conds.size();
		os.serial( size );
		for ( i = 0; i < size; i++ )
		{
			bool ct = _CondsVal[i];
			os.serial( (bool &) ct );
		}
		
		size = _Concs.size();
		os.serial( size );
		for ( i = 0; i < size; i++ )
		{
			bool test = _ConcsVal[i];
			os.serial( test  );
		}
	}

	void CBoolOperator::load(NLMISC::IStream &is)
	{
		sint32 i;
		sint32 nb_vals;

		is.serial( nb_vals );
		for ( i = 0; i < nb_vals; i++ )
		{
			bool val;
			is.serial( val );
			_CondsVal.push_back( val );
		}

		is.serial( nb_vals );
		for ( i = 0; i < nb_vals; i++ )
		{
			bool val;
			is.serial( val );
			_ConcsVal.push_back( val );
		}

	}

	bool CBoolOperator::isValid(CFactBase *)
	{
		return isTrue();
	}

	void CBoolOperator::addPrecondition(IBaseAssert *a, bool v)
	{
		_Conds.push_back( a );
		a->incRef();
		a->addOutput( this );
		_CondsVal.push_back( v );
	}

	void CBoolOperator::addPostcondition(IBaseAssert *a, bool v)
	{
		_Concs.push_back( a );
		a->incRef();
		a->addInput( this );
		_ConcsVal.push_back( v );
	}

	const NLAIC::IBasicType *CBoolOperator::clone() const
	{
		CBoolOperator *clone = new CBoolOperator;
		sint32 i;
		for (i = 0; i < (sint32) _Conds.size(); i++ )
		{
			clone->addPrecondition( _Conds[i], _CondsVal[i] );
		}

		for (i = 0; i < (sint32) _Concs.size(); i++ )
		{
			clone->addPrecondition( _Concs[i], _ConcsVal[i] );
		}
		return clone;
	}

	const NLAIC::IBasicType *CBoolOperator::newInstance() const
	{
		CBoolOperator *instance = new CBoolOperator();
		return instance;
	}

	void CBoolOperator::getDebugString(std::string &text) const
	{
		text += "<CBoolOperator>\n  -Preconditions:\n";

		sint32 i;
		for ( i = 0; i < (sint32) _Conds.size() ; i++ )
		{
			if ( _CondsVal[i] )
				text += "    ";
			else
				text += "     !";
			text += _Conds[i]->getName().getString();

			if ( _CondsVal[i] == _Conds[i]->isTrue() )
				text += " (true)\n";
			else
				text += " (false)\n";
		}

		text += "  -Postconditions:\n";
		for ( i = 0; i < (sint32) _Concs.size() ; i++ )
		{
			if ( _ConcsVal[i] )
				text += "      ";
			else
				text += "   !";
			text += _Concs[i]->getName().getString();
			text += "\n";
		}
	}

	bool CBoolOperator::isEqual(const CBoolOperator &a) const
	{
		return a.isTrue() == isTrue();
	}

	bool CBoolOperator::isEqual(const IBasicObjectIA &a) const
	{
		return false;
	}

	const NLAIC::CIdentType &CBoolOperator::getType() const
	{
		return IdBoolOperator;
	}

	float CBoolOperator::truthValue() const
	{
		if ( isTrue() )
			return 1.0;
		else
			return 0.0;
	}

	std::list<CFact *> *CBoolOperator::backward(std::list<CFact *> &facts)
	{
		return NULL;
	}

	std::list<CFact *> *CBoolOperator::forward(std::list<CFact *> &facts) 
	{
		return propagate( facts );
	}

	std::list<CFact *> *CBoolOperator::propagate(std::list<CFact *> &facts)
	{
		std::list<CFact *> *result = new std::list<CFact *>;

		bool *test= new bool[ _Conds.size() ];
		sint32 i;
		std::list<CFact *>::iterator it_f = facts.begin();
		while ( it_f != facts.end() )
		{
			for ( i = 0; i < (sint32) _Conds.size(); i++ )
			{
				if ( (*_Conds[i]) == (*it_f)->getAssert()  )
					if ( ((CBoolType *)(*it_f)->getValue(0))->isTrue() == _CondsVal[i] ) 
						test[i] = true;
					else
						test[i] = false;
			}
			it_f++;
		}

		for ( i = 0; i < (sint32) _Conds.size(); i++ )
		{
			if ( !test[i] )
			{
				delete test;
				return result;
			}
		}
		delete test;

		for (i = 0; i < (sint32) _Concs.size(); i++ )
		{
			CFact *tmp = new CFact (_Concs[i], _ConcsVal[i] );
			result->push_back( tmp );
		}
		return result;
	}

	const IObjectIA::CProcessResult &CBoolOperator::run()
	{		
		return IObjectIA::ProcessRun;
	}

	float CBoolOperator::priority() const
	{
		return 0.0;
	}

	void CBoolOperator::success()
	{
	}

	void CBoolOperator::failure()
	{
	}

	void CBoolOperator::success(IBaseOperator *)
	{
	}

	void CBoolOperator::failure(IBaseOperator *)
	{
	}
}
