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

#include "logic/factbase.h"
#include "logic/varset.h"
#include "logic/ia_assert.h"
#include "logic/bool_assert.h"
#include "logic/fo_assert.h"
#include "logic/fact.h"
#include "logic/valueset.h"

namespace NLIALOGIC
{
	using namespace NLIAAGENT;
	
	CFactBase::CFactBase()
	{
	}

	CFactBase::~CFactBase() {
		while ( _Asserts.size() )
		{
			_Asserts.front()->release();
			_Asserts.pop_front();
		}
	}

	IBaseAssert *CFactBase::addAssert(IVarName &n, sint32 nb_vars)
	{
		IBaseAssert *my_assert = findAssert( n , nb_vars );
		if ( ! my_assert )
		{
			if ( nb_vars > 0)
				_Asserts.push_back( my_assert = new CFirstOrderAssert(n, nb_vars) );
			else
				_Asserts.push_back( my_assert = new CBoolAssert(n) );
		}
		return my_assert;
	}

	IBaseAssert *CFactBase::findAssert(IBaseAssert *a)
	{
		std::list<IBaseAssert *>::iterator it_a = _Asserts.begin();
		while ( it_a != _Asserts.end() )
		{
			if (  a->getName() == (*it_a)->getName()
					&& a->nbVars() == (*it_a)->nbVars() )
			{
				return  *it_a;
			}
			it_a++;
		}
		return NULL;
	}

	IBaseAssert *CFactBase::findAssert( IVarName &a_name, sint32 nb_vars)
	{
		std::list<IBaseAssert *>::iterator it_a = _Asserts.begin();
		while ( it_a != _Asserts.end() )
		{
			if (  a_name == ( *it_a )->getName()
				&& nb_vars == (*it_a)->nbVars() )
			{
				return  *it_a;
			}
			it_a++;
		}
		return NULL;
	}

	void CFactBase::addFact(IVarName &a_name , CValueSet *fp)
	{
		// Est-ce que l'assertion existe déja?
		IBaseAssert *assert = findAssert( a_name, fp->size() );

		// Si non la créer
		if ( !assert )
		{
			if ( !fp )
				_Asserts.push_back( assert = new CBoolAssert( a_name ) );	// 0 order assert
			else
			{
			  CStringVarName x((CStringVarName &)a_name);
				_Asserts.push_back( assert = new CFirstOrderAssert(x) );	// First order assert
				assert->addFact( fp );
			}
		}
	}
	
	void CFactBase::removeFact(CFact *fp) 
	{
		// Est-ce que l'assertion existe déja?
		IBaseAssert *assert = findAssert( fp->getAssert());

		if ( assert )
			assert->removeFact( fp );
	}

	const NLIAC::IBasicType *CFactBase::clone() const
	{
		CFactBase *result = new CFactBase;
		std::list<IBaseAssert *>::const_iterator it_a = _Asserts.begin();
		while ( it_a != _Asserts.end() )
		{
			result->_Asserts.push_back( (IBaseAssert *) (*it_a)->clone() );
			it_a++;
		}
		result->incRef();
		return result;
	}
	const NLIAC::IBasicType *CFactBase::newInstance() const
	{
		CFactBase *instance = new CFactBase();
		instance->incRef();
		return instance;
	}

	const NLIAC::CIdentType &CFactBase::getType() const
	{
		return IdFactBase;
	}

	void CFactBase::save(NLMISC::IStream &os)
	{
		IObjectIA::save( os );
		sint32 size = _Asserts.size();
		os.serial( size );
		std::list<IBaseAssert *>::const_iterator it_a = _Asserts.begin();
		while ( it_a != _Asserts.end() )
		{
			os.serial( (NLIAC::CIdentType &) (*it_a)->getType() );
			(*it_a)->save( os );
			it_a++;
		}
	}
			
	void CFactBase::load(NLMISC::IStream &is)
	{
		IObjectIA::load( is );

		sint32 nb_asserts;
		is.serial( nb_asserts );
		for (sint32 i = 0; i < nb_asserts; i++ )
		{
			NLIAC::CIdentTypeAlloc id;
			is.serial( id );
			IBaseAssert *tmp_val = (IBaseAssert *) id.allocClass();
			tmp_val->load( is );
			_Asserts.push_back( tmp_val );
		}
	}

	void CFactBase::getDebugString(char *txt) const
	{
		strcpy(txt,"CFactBase");
	}
	
	bool CFactBase::isEqual(const IBasicObjectIA &a) const
	{
		return false;
	}

	const IObjectIA::CProcessResult &CFactBase::run()
	{
		return IObjectIA::ProcessRun;
	}

	bool CFactBase::isTrue() const
	{
		return false;
	}


	std::list<CFact *> *CFactBase::getAssertFacts(IBaseAssert *a)
	{
		IBaseAssert *assert = findAssert( a );
		if ( assert )
		{
			return assert->getFacts();
		}
		return NULL;
	}

	void CFactBase::addAssert(IBaseAssert *a)
	{
		a->incRef();
		_Asserts.push_back(a);
	}
}
