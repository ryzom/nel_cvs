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

#include "nel/ai/logic/fact.h"
#include "nel/ai/logic/ai_assert.h"
#include "nel/ai/logic/varset.h"
#include "nel/ai/agent/object_type.h"

namespace NLAILOGIC
{
	using namespace NLAIAGENT;

	CFact::CFact() : CValueSet()
	{
		_Assert = NULL;
		_AssertName = NULL;
	}	


	CFact::CFact(IBaseAssert *a) : CValueSet(a->nbVars() )
	{
		_Assert = a;
		_AssertName = NULL;
	}

	CFact::CFact(IBaseAssert *a, CValueSet *vals) : CValueSet( *vals )
	{
		_Assert = a;
		_AssertName = NULL;
	}

	CFact::CFact(IBaseAssert *a, bool v) : CValueSet(1)
	{
		_Assert = a;
		_Values[0] = new CBoolType(v);
		_AssertName = NULL;
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
		_AssertName = NULL;
	}


	CFact::CFact(NLAIAGENT::IVarName &name) : CValueSet()
	{
		_AssertName = (NLAIAGENT::IVarName *) name.clone();
		_Assert = NULL;
	}

	CFact::CFact(sint32 i,NLAIAGENT::IVarName *a) : CValueSet(i )
	{
		//_Assert = a;
		_AssertName = (NLAIAGENT::IVarName *) a;
		_Assert = NULL;
	}

	CFact::CFact(NLAIAGENT::IVarName &a, CValueSet *vals) : CValueSet( *vals )
	{
		_AssertName = (NLAIAGENT::IVarName *) a.clone();
		_Assert = NULL;
	}

	CFact::CFact(NLAIAGENT::IVarName &a, bool v) : CValueSet(1)
	{
		_Values[0] = new CBoolType(v);
		_AssertName = (NLAIAGENT::IVarName *) a.clone();
		_Assert = NULL;
	}

	CFact::CFact(NLAIAGENT::IVarName &a, CVarSet *vars) : CValueSet( vars->size() )
	{
		_Assert = NULL;
		std::list<IObjetOp *> *vals = vars->getValues();
		std::list<IObjetOp *>::iterator it_v = vals->begin();
		while ( it_v != vals->end() )
		{
			vals->push_back( *it_v );
			it_v++;
		}
		delete vals;
		_AssertName = (NLAIAGENT::IVarName *) a.clone();
	}

	CFact::~CFact()
	{
		if(_AssertName != NULL) _AssertName->release();
/*		if ( _Assert != NULL )
			_Assert->release();
			*/
	}


	IBaseAssert *CFact::getAssert() 
	{
		return _Assert;
	}

	NLAIAGENT::IVarName &CFact::getAssertName()
	{
		return *_AssertName;
	}

	void CFact::getDebugString(std::string &txt) const
	{
		std::string buf;
		if(_Assert != NULL) _Assert->getDebugString(buf);
		else buf = "NULL";
		txt += NLAIC::stringGetBuild("<CFact> %s\n", buf.c_str());
		for (sint32 i = 0; i < _NbValues; i++ )
		{
			txt += " , ";
			if ( _Values[i] )
			{
				_Values[i]->getDebugString( buf );

				 txt += buf;
			}
			else txt += "NULL";
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
/*			IObjetOp *test = (*_Values[i]) == *((CValueSet &)a).getValue(i);
			bool test_result = test->isTrue();
			test->release();
			if ( test_result )
				return false;
				*/

			if ( ! ( (*_Values[i]) == *((CValueSet &)a).getValue(i) ) )
				return false;
		}
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
		return result;
	}

	NLAIAGENT::TQueue CFact::isMember(const NLAIAGENT::IVarName *className,const NLAIAGENT::IVarName *funcName,const NLAIAGENT::IObjectIA &params) const
	{

#ifdef NL_DEBUG	
		std::string nameP;
		std::string nameM;
	funcName->getDebugString(nameM);
	params.getDebugString(nameP);

	const char *dbg_class_name = (const char *) getType();
#endif
		NLAIAGENT::TQueue r;
		if(className == NULL)
		{
			if( (*funcName) == NLAIAGENT::CStringVarName( "Constructor" ) )
			{					
				NLAIAGENT::CObjectType *c = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( CFact::IdFact ) );					
				r.push( NLAIAGENT::CIdMethod( 0 + CValueSet::getMethodIndexSize(), 0.0, NULL, c) );					
			}
		}

		if ( r.empty() )
			return CValueSet::isMember(className, funcName, params);
		else
			return r;
	}

	///\name Some IObjectIA method definition.
	//@{		
	NLAIAGENT::IObjectIA::CProcessResult CFact::runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *)
	{
		return IObjectIA::CProcessResult();
	}

	NLAIAGENT::IObjectIA::CProcessResult CFact::runMethodeMember(sint32 index, NLAIAGENT::IObjectIA *p)
	{
		NLAIAGENT::IBaseGroupType *param = (NLAIAGENT::IBaseGroupType *)p;

		switch(index - CValueSet::getMethodIndexSize())
		{
		case 0:
			{					

				NLAIAGENT::CStringType *name = (NLAIAGENT::CStringType *) param->getFront()->clone();
				param->popFront();
#ifdef NL_DEBUG
				const char *dbg_name = name->getStr().getString();
#endif
				// If the constructor() function is explicitely called and the object has already been initialised
				if ( _AssertName )
					_AssertName->release();
//				_Args.clear();

				_AssertName = (NLAIAGENT::IVarName *) name->getStr().clone();
/*				std::list<const NLAIAGENT::IObjectIA *> args;
				while ( param->size() )
				{
					_Args.push_back( (NLAIAGENT::IObjectIA *) param->getFront() );
					param->popFront();
				}
				*/
				return IObjectIA::CProcessResult();		
			}
			break;
		}

		return IObjectIA::CProcessResult();
	}

	sint32 CFact::getMethodIndexSize() const
	{
		return CValueSet::getMethodIndexSize() + 1;
	}

	const NLAIC::CIdentType &CFact::getType() const
	{
		return IdFact;
	}

	//@}

	CFact::CFact(const CFact &fact) : CValueSet(fact)
	{
		if ( fact._AssertName != NULL )
			_AssertName = (NLAIAGENT::IVarName *) fact._AssertName->clone();
		else
			_AssertName = NULL;
		_Assert = fact._Assert;
	}

	const NLAIC::IBasicType *CFact::clone() const
	{
		return new CFact( *this );
	}

	const NLAIC::IBasicType *CFact::newInstance() const
	{
		return new CFact();
	}
}
