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

#include "nel/ai/logic/boolval.h"

namespace NLAILOGIC
{
	using namespace NLAIAGENT;


	IBaseBoolType::IBaseBoolType() 
	{			
	}

	IBaseBoolType::~IBaseBoolType()
	{
	}	

// ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


	

	CBoolType::CBoolType(bool value) : _Value( value )
	{
	}

	CBoolType::CBoolType(const CBoolType &cp)
	{
		_Value = cp._Value;
	}

	CBoolType::~CBoolType()
	{
	}
	
	const NLAIC::IBasicType *CBoolType::clone() const
	{
		NLAIC::IBasicInterface *m = new CBoolType( *this );
		return m;
	}

	const NLAIC::IBasicType *CBoolType::newInstance() const
	{

		return clone();
	}


	void CBoolType::save(NLMISC::IStream &os)
	{			
		IObjectIA::save(os);
		os.serial( (bool &) _Value );
	}

	void CBoolType::load(NLMISC::IStream &is)
	{
		IObjectIA::load( is );
//		is >> _Value;	
	}

	void CBoolType::getDebugString(char *text) const
	{
		strcpy( text ,"CBoolType<");
		if ( _Value )
			strcat( text , "true" );
		else
			strcat( text , "false" );
	}

	bool CBoolType::isTrue() const
	{
		return _Value;
	}

	float CBoolType::truthValue() const
	{
		if ( _Value )
			return 1.0;
		else
			return 0.0;

	}

	const IObjectIA::CProcessResult &CBoolType::run()
	{
		return IObjectIA::ProcessRun;
	}

	bool CBoolType::isEqual(const CBoolType &a) const
	{
		return ( a._Value == _Value );
	}

	bool CBoolType::isEqual(const IBasicObjectIA &a) const
	{
		return ( ((CBoolType &)a)._Value == _Value );
	}

	std::vector<IBaseVar *> *CBoolType::getVars()
	{
		return NULL;
	}

	const NLAIC::CIdentType &CBoolType::getType() const
	{
		return IdBoolType;
	}

	IObjetOp &CBoolType::operator += (const IObjetOp &a)
	{
		_Value = _Value || ((CBoolType &)a)._Value;
		return ( *this );
	}

	IObjetOp &CBoolType::operator -= (const IObjetOp &a)
	{
		_Value = _Value && ((CBoolType &)a)._Value;
		return ( *this );
	}

	IObjetOp &CBoolType::operator *= (const IObjetOp &a)
	{
		_Value = _Value ^ ((CBoolType &)a)._Value;
		return ( *this );
	}


	IObjetOp *CBoolType::operator < (IObjetOp &a) const
	{
		CBoolType *result = new CBoolType( _Value && !((CBoolType &)a)._Value );
		return result;
	}

	IObjetOp *CBoolType::operator > (IObjetOp &a) const
	{
		CBoolType *result = new CBoolType( !_Value && ((CBoolType &)a)._Value );
		return result;
	}

	IObjetOp *CBoolType::operator <= (IObjetOp &a) const
	{

		CBoolType *result = new CBoolType( !_Value || ((CBoolType &)a)._Value );
		return result;
	}

	IObjetOp *CBoolType::operator >= (IObjetOp &a) const
	{
		CBoolType *result = new CBoolType( _Value || !((CBoolType &)a)._Value );
		return result;
	}

	IObjetOp *CBoolType::operator ! () const
	{
		CBoolType *result = new CBoolType( !_Value );
		return result;
	}

	IObjetOp *CBoolType::operator != (IObjetOp &a) const
	{
		// Le truc pas beau qui embête Sam qui fait que des trucs beaux:
		CBoolType *result = new CBoolType( _Value ^ ((CBoolType &)a)._Value );
		return result;
	}

	CBoolTrue::CBoolTrue() : CBoolType( true )
	{
	}

	CBoolFalse::CBoolFalse() : CBoolType( false )
	{
	}

	IObjetOp *CBoolType::operator == (IObjetOp &a) const
	{
		CBoolType *result = new CBoolType( _Value == ((CBoolType &)a)._Value );
		return result;
	}
}
