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

#include "logic/fact.h"
#include "logic/ia_assert.h"
#include "logic/varset.h"

namespace NLIALOGIC
{
	using namespace NLIAAGENT;

	CFact::CFact(IBaseAssert *a) : CValueSet(a->nbVars() )
	{
		_Assert = a;
	}

	CFact::CFact(IBaseAssert *a, CValueSet *vals) : CValueSet( *vals )
	{
		_Assert = a;
	}

	CFact::CFact(IBaseAssert *a, bool v) : CValueSet(1)
	{
		_Values[0] = new CBoolType(v);
		_Values[0]->incRef();
	}

	CFact::CFact(IBaseAssert *a, CVarSet *vars) : CValueSet( vars->size() )
	{
		std::list<IObjetOp *> *vals = vars->getValues();
		std::list<IObjetOp *>::iterator it_v = vals->begin();
		while ( it_v != vals->end() )
		{
			vals->push_back( *it_v );
			it_v++;
		}
		delete vals;
		_Assert = a;
	}

	IBaseAssert *CFact::getAssert() 
	{
		return _Assert;
	}

	void CFact::getDebugString(char *txt) const
	{
		char buf[512];
		_Assert->getDebugString(buf);
		sprintf(txt,"<CFact> %s\n", buf);
		for (sint32 i = 0; i < _NbValues; i++ )
		{
			strcat( txt, " , ");
			if ( _Values[i] )
			{
				_Values[i]->getDebugString( buf );

				strcat( txt, buf );
			}
			else strcat( txt, "NULL");
		}
	}

	void CFact::propagate()
	{
		if ( _Assert )
			_Assert->addFact( this );
	}

	bool CFact::operator== (const IBasicObjectIA &a) const
	{
		if ( ! (_Assert->getName() == ( (CFact &)a )._Assert->getName() ) )
			return false;

		if( ((CFact &) a)._NbValues != _NbValues )
			return false;

		for (sint32 i = 0; i < _NbValues; i++ )
		{
			IObjetOp *test = (*_Values[i]) != *((CValueSet &)a).getValue(i);
			bool test_result = test->isTrue();
			test->release();
			if ( test_result )
				return false;
/*			{
				CBoolType *result = new CBoolType ( false );
				result->incRef();
				return result;
			}*/
		}
/*		CBoolType *result = new CBoolType ( true );
		result->incRef();
		return result;*/
		return true;
	}

	CValueSet *CFact::asValueSet()
	{
		CValueSet *result = new CValueSet( _NbValues );
		for (sint32 i = 0; i < _NbValues; i++ )
		{
			result->setValue(i, _Values[i] );
			_Values[i]->incRef();
		}
		result->incRef();
		return result;
	}
}
