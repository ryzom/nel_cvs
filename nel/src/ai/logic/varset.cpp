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

#include "nel/ai/logic/varset.h"
#include "nel/ai/logic/var.h"
#include "nel/ai/logic/valueset.h"
#include "nel/ai/logic/ai_assert.h"

namespace NLAILOGIC
{
	using namespace NLAIAGENT;

	CVarSet::CVarSet()
	{
		NbInstanciated = 0;
	}

	CVarSet::CVarSet(const CVarSet &vs)
	{
		for ( sint32 i = 0; i < (sint32)vs._Vars.size() ; i++ )
		{
			_Vars.push_back( (IBaseVar *) vs._Vars[ i ]->clone() );
		}
	}

	CVarSet::CVarSet( std::vector<IBaseVar *> &c)
	{
		for ( sint32 i = 0; i < (sint32)c.size() ; i++ )
		{
			_Vars.push_back( (IBaseVar *) c[ i ]->clone() );
		}
	}

	CVarSet::~CVarSet()
	{
		for ( sint32 i = 0; i < (sint32)_Vars.size() ; i++ )
		{
			_Vars[ i ]->release();
		}
	}


	const NLAIC::IBasicType *CVarSet::clone() const
	{
		NLAIC::IBasicInterface *m = new CVarSet(*this);
		return m;
	}


	const NLAIC::IBasicType *CVarSet::newInstance() const
	{
		return clone();
	}

	void CVarSet::save(NLMISC::IStream &os)
	{			
		IObjectIA::save(os);
		sint32 size = _Vars.size();
		os.serial( size );
		
		for (sint32 i = 0; i < (sint32)_Vars.size() ; i++ )
		{
			os.serial( (NLAIC::CIdentType &) _Vars[ i ]->getType() );
			_Vars[ i ]->save( os );
		}
	}

	void CVarSet::load(NLMISC::IStream &is)
	{
		IObjectIA::load(is);
		sint32 nb_vars;
		is.serial( nb_vars );

		for (sint32 i = 0; i < nb_vars; i++ )
		{
			NLAIC::CIdentTypeAlloc id;
			is.serial( id );
			IBaseVar *tmp_var = (IBaseVar *)id.allocClass();
			_Vars.push_back( tmp_var );
		}
	}

	void CVarSet::getDebugString(std::string &text) const
	{
		text += "CVarSet\n";
		std::string buf;

		std::vector<IBaseVar *>::const_iterator it_var = _Vars.begin();
		while ( it_var != _Vars.end() )
		{
 			(*it_var)->getDebugString( buf );
			text += buf;
			it_var++;
		}
	}
	
	float CVarSet::truthValue() const
	{
		if ( isTrue() )
			return 1.0 ;
		else
			return 0.0 ;
	}

	const IObjectIA::CProcessResult &CVarSet::run()
	{
		 return IObjectIA::ProcessRun;
	}

	bool CVarSet::isEqual(const CVarSet &a) const
	{
		return false;
	}

	bool CVarSet::isEqual(const IBasicObjectIA &a) const
	{
/*		if ( ! ( _Assert == ((CVarSet &)a)._Assert ) )
			return false;
*/
		for (sint32 i = 0; i < (sint32)_Vars.size() ; i++ )
		{	
			if (! ( ( *_Vars[ i ] ) == ( *((CVarSet &) a)._Vars[i]) ))
				return false;
		}
		return true;
	}

	void CVarSet::addVar(IBaseVar *var)
	{
		IBaseVar *tmp = (IBaseVar *) var->clone();
		_Vars.push_back( tmp );
	}

	void CVarSet::removeVar(IBaseVar *var)
	{
		std::vector<IBaseVar *>::iterator it_var = _Vars.begin();
		while ( it_var != _Vars.end() )
		{
			if ( var == *it_var )
			{
				( *it_var )->release();
				_Vars.erase( it_var );
				return;
			}
			it_var++;
		}
	}
	
	std::vector<IBaseVar *> *CVarSet::getVars()
	{
		std::vector<IBaseVar *> *result= new std::vector<IBaseVar *>;

		std::vector<IBaseVar *>::iterator it_var = _Vars.begin();
		while ( it_var != _Vars.end() )
		{
			result->push_back( (IBaseVar *) (*it_var)->clone() );
			it_var++;
		}
		return result;
	}

	CVarSet *CVarSet::unify(CVarSet *fp)
	{
		if ( fp->_Vars.size() != _Vars.size() )
			return false;

		std::vector<IBaseVar *>::iterator it_vt = _Vars.begin();
		std::vector<IBaseVar *>::iterator it_vu = fp->_Vars.begin();

		CVarSet *unified = new CVarSet();
				
		while ( it_vt != _Vars.end() )
		{
			IBaseVar *tmp_var = (IBaseVar *) (*it_vt)->clone();
			if ( tmp_var->unify( *it_vu , true ) )
				unified->addVar( tmp_var );
			else 
			{
				delete tmp_var;
				delete unified;
				return NULL;
			}
			it_vt++;
			it_vu++;
		}
		return unified;
	}

	CVarSet *CVarSet::unify(CValueSet *fp)
	{
		if ( fp->size() != (sint32)_Vars.size() )
			return NULL;

/*		vector<IBaseVar *> *tmp_Vars = new vector<IObjetOp *>;
		vector<IBaseVar *>::iterator it_vt = _Vars.begin();

		CVarSet *unified = new CVarSet(NULL);
				
		while ( it_vt != _Vars.end() )
		{
			IBaseVar *tmp_var = (IBaseVar *) (*it_vt)->clone();
			if ( tmp_var->unify( *it_vu , true ) )
				unified->addVar( tmp_var );
			else 
			{
				delete tmp_var;
				delete unified;
				return NULL;
			}
			it_vt++;
			it_vu++;
		}
		return unified;
		*/
		return NULL;
	}


	const NLAIC::CIdentType &CVarSet::getType() const
	{
		return IdVarSet;
	}

	sint32 CVarSet::size()
	{
		return _Vars.size();
	}

	IObjetOp *CVarSet::operator ! () const
	{
		return new CBoolType( ! ( NbInstanciated == 0 ) );
	}

	IObjetOp *CVarSet::operator != (IObjetOp &a) const
	{
		return new CBoolType( ! isEqual( a ) );	
	}

	bool CVarSet::isTrue() const
	{
		std::vector<IBaseVar *>::const_iterator it_var = _Vars.begin();
		while ( it_var != _Vars.end() )
		{
			if ( ! (*it_var)->getValue() )
				return false;
			it_var++;
		}
		return true;
	}

	bool CVarSet::isUnified() 
	{
		return ( NbInstanciated == 0 );
	}

	std::list<IObjetOp *> *CVarSet::getValues()
	{
		std::list<IObjetOp *> *result = new std::list<IObjetOp *>;

		IObjetOp *tmp_val;

		for ( sint32 i = 0; i < (sint32)_Vars.size(); i++ )
		{
			tmp_val = _Vars[ i ];
			IObjetOp *val;
			if ( (val = _Vars[ i ]->getValue()) )
				result->push_back( val );
		}

		return result;
	}


	CValueSet *CVarSet::asCValueSet()
	{
		CValueSet *result = new CValueSet( _Vars );
		return result;
	}

	sint32 CVarSet::undefined() 
	{
		sint32 nb_undef = _Vars.size();
		for ( sint32 i = 0; i < (sint32) _Vars.size(); i++ )
			if ( _Vars[i]->getValue() != NULL )
				nb_undef--;
		return nb_undef;
	}

	IBaseVar *CVarSet::operator[](sint32 i)
	{
		return _Vars[i];
	}


////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

	CFactPattern::CFactPattern(IBaseAssert *assert)
	{
		_Assert = assert;
	}

	CFactPattern::CFactPattern(const CFactPattern &fp) : CVarSet(fp)
	{
		_Assert = fp._Assert;	
	}

	CFactPattern::CFactPattern(const CVarSet &fp) : CVarSet(fp)
	{
		_Assert = NULL;	
	}

	IBaseAssert *CFactPattern::getAssert()
	{
		return _Assert;
	}

	void CFactPattern::setAssert(IBaseAssert *a)
	{
		_Assert = a;
	}

	CVarSet *CFactPattern::unify(CFactPattern *vs)
	{
		if ( vs->getAssert() != _Assert )
			return NULL;
		else
			return CVarSet::unify(vs);
	}

	bool CFactPattern::isEqual(const IBasicObjectIA &a) const
	{
		if ( ! ( _Assert == ((CFactPattern &)a)._Assert ) )
			return false;

		for (sint32 i = 0; i < (sint32)_Vars.size() ; i++ )
		{	
			if (! ( ( *_Vars[ i ] ) == ( *((CFactPattern &) a)._Vars[i]) ))
				return false;
		}
		return true;
	}


	void CFactPattern::init(IObjectIA *params)
	{
		if ( ((IBaseGroupType *)params)->size() < 2 ) 
		{
		//	throw Exc::
		}

		// Assertion
		IObjectIA * arg = (IObjectIA *) ((IBaseGroupType *)params)->popFront();
		
		// Variables
		while (	 ((IBaseGroupType *)params)->size() )
		{
			arg = (IObjectIA *) ((IBaseGroupType *)params)->popFront();
			_Vars.push_back( (IBaseVar *) arg->clone() );
			arg->release();
		}
	}

	const NLAIC::CIdentType &CFactPattern::getType() const
	{
		return IdFactPattern;
	}

	const NLAIC::IBasicType *CFactPattern::clone() const
	{
		NLAIC::IBasicInterface *m = new CFactPattern(*this);
		return m;
	}

	const NLAIC::IBasicType *CFactPattern::newInstance() const
	{
		return clone();
	}

	void CFactPattern::getDebugString(std::string &text) const
	{
		std::string buf("NULL");
		if ( _Assert )
			_Assert->getDebugString( buf );

		text += NLAIC::stringGetBuild("CFactPattern Assert<%s>\n", buf.c_str());

		std::vector<IBaseVar *>::const_iterator it_var = _Vars.begin();
		while ( it_var != _Vars.end() )
		{
			buf[0] = 0;
 			(*it_var)->getDebugString( buf );
			text += buf;
			it_var++;
		}
	}

	void CFactPattern::propagate()
	{
		_Assert->addFact(this);
	}
}
