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

#include "logic/bool_assert.h"
#include "logic/boolval.h"
#include "logic/fact.h"
#include "logic/varset.h"

namespace NLAILOGIC
{
	using namespace NLAIAGENT;

	CBoolAssert::CBoolAssert( IVarName &n, bool truth) : IBaseAssert( n )
	{
		_Value = truth;
	}
	
	CBoolAssert::CBoolAssert( const CBoolAssert &a) : IBaseAssert( *a._Name)
	{
		_Value = a._Value;
	}

	CBoolAssert::~CBoolAssert()
	{
	}

	void CBoolAssert::addFact(CVarSet *f)
	{
		_Value =( (CBoolType *) (*f)[0] )->isTrue();		
	}

	void CBoolAssert::addFact(CFact *f)
	{
		_Value =( (CBoolType *) f->getValue(0) )->isTrue();
	}

	void CBoolAssert::removeFact(CFact *f)
	{
		_Value = ! ( (CBoolType *) f->getValue(0) )->isTrue();
	}


	void CBoolAssert::addFact(CValueSet *val)
	{
		_Value =( (CBoolType *) val->getValue(0) )->isTrue();
	}

	bool CBoolAssert::isTrue() const
	{
		return _Value;
	}

	const NLAIC::CIdentType &CBoolAssert::getType() const 
	{
		return IdBoolAssert;
	}

	const NLAIC::IBasicType *CBoolAssert::clone() const
	{
		CBoolAssert *clone = new CBoolAssert( *this );
		clone->incRef();
		return clone;
	}

	const NLAIC::IBasicType *CBoolAssert::newInstance() const
	{
	  NLAIAGENT::CStringVarName x("");
		CBoolAssert *instance = new CBoolAssert(x);
		instance->incRef();
		return instance;
	}

	void CBoolAssert::save(NLMISC::IStream &os)
	{
		IObjetOp::save( os );
		os.serial( (bool &) _Value );
	}

	void CBoolAssert::load(NLMISC::IStream &is)
	{
		IObjetOp::load( is );
		is.serial( _Value );
	}


	void CBoolAssert::getDebugString(char *text) const
	{
		strcpy( text, "<CBoolAssert> = ");
		if ( _Value )
			strcat( text, "true" );
		else
			strcat( text, "false" );
	}

	bool CBoolAssert::isEqual(const IBasicObjectIA &a) const
	{
		return ( ((CBoolAssert &)a)._Value == _Value );
	}

	const IObjectIA::CProcessResult &CBoolAssert::run()
	{
		return IObjectIA::ProcessRun;
	}

	sint32 CBoolAssert::nbVars() const
	{
		return 0;
	}

	std::list<CFact *> *CBoolAssert::getFacts() const
	{
		return new std::list<CFact *>;	
	}
}
