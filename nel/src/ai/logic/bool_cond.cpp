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

#include "logic/bool_cond.h"

namespace NLIALOGIC
{
	using namespace NLIAAGENT;
	
	IBaseCond::IBaseCond()
	{
	}

	IBaseCond::IBaseCond(std::list<IBaseBoolType *> &conds)
	{
		std::list<IBaseBoolType *>::iterator it_c = conds.begin();
		while ( it_c != conds.end() )
		{
			_Conds.push_back( (IBaseBoolType *) (*it_c)->clone() );
			it_c++;
		}
	}
	
	IBaseCond::~IBaseCond()
	{
		std::list<IBaseBoolType *>::iterator it_c = _Conds.begin();
		while ( it_c != _Conds.end() )
		{
			( *it_c )->release();
			it_c++;
		}
	}
	
	void IBaseCond::addCond(IBaseBoolType *cond)
	{
		_Conds.push_back( (IBaseBoolType *) cond->clone() );
	}

	CondAnd::CondAnd()
	{
	}
	
	CondAnd::CondAnd(std::list<IBaseBoolType *> &conds) : IBaseCond( conds )
	{
	}

	float CondAnd::truthValue() const
	{
		float truth = 1.0;
		std::list<IBaseBoolType *>::const_iterator it_c = _Conds.begin();
		while ( it_c != _Conds.end() )
		{
			if ( ( *it_c )->truthValue() < truth )
				truth = ( *it_c )->truthValue();
		}
		return truth;
	};

	bool CondAnd::isTrue()
	{
		std::list<IBaseBoolType *>::iterator it_c = _Conds.begin();
		while ( it_c != _Conds.end() )
		{
			if ( ! ( *it_c )->isTrue() )
				return false;
		}
		return true;
	}
		
	CondOr::CondOr()
	{
	}

	CondOr::CondOr(std::list<IBaseBoolType *> &conds) : IBaseCond( conds )
	{
	}
	
	float CondOr::truthValue() const
	{
		float truth = 0.0;
		std::list<IBaseBoolType *>::const_iterator it_c = _Conds.begin();
		while ( it_c != _Conds.end() )
		{
			if ( ( *it_c )->truthValue() > truth )
				truth = ( *it_c )->truthValue();
		}
		return truth;
	};

	bool CondOr::isTrue()
	{
		std::list<IBaseBoolType *>::iterator it_c = _Conds.begin();
		while ( it_c != _Conds.end() )
		{
			if ( ! ( *it_c )->isTrue() )
				return true;
		}
		return false;
	}	
}
