/** \file interpret_fuzzytype.cpp
 * Base class for the integration of fuzzy facts in conditions
 *
 * $Id: fuzzytype.cpp,v 1.4 2001/01/10 10:10:08 chafik Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "nel/ai/fuzzy/fuzzytype.h"

namespace NLAIFUZZY
{	
	using namespace NLAILOGIC;

	const NLAIC::IBasicType *FuzzyType::clone() const
	{
		NLAIC::IBasicInterface *m = new FuzzyType(*this);
		m->incRef();
		return m;
	}

	const NLAIC::IBasicType *FuzzyType::newInstance() const
	{
		return clone();
	}

	void FuzzyType::save(NLMISC::IStream &os)
	{			
		IBaseBoolType::save(os);
		double v = _Value;
		os.serial( v );
		v = _Threshold;
		os.serial( v ); 
	}

	void FuzzyType::load(NLMISC::IStream &is)
	{
		IBaseBoolType::load(is);
		is.serial(_Value);
		is.serial(_Threshold);
	}

	void FuzzyType::getDebugString(char *text) const
	{
		sprintf(text,"FuzzyType<%d>",_Value);
	}
	
	bool FuzzyType::isTrue() const
	{
		return ( _Value >= _Threshold ) ;
	}

	float FuzzyType::truthValue() const
	{
		return _Value;
	}

	const NLAIAGENT::IVarName * FuzzyType::getName() const
	{
		return NULL;
	}

	void FuzzyType::setName(const NLAIAGENT::IVarName &n)
	{
	}

	const NLAIAGENT::IObjectIA::CProcessResult &FuzzyType::run()
	{
		 return NLAIAGENT::IObjectIA::ProcessRun;
	}

	bool FuzzyType::isEqual(const FuzzyType &a) const
	{
		return ( a._Value == _Value );
	}

	bool FuzzyType::isEqual(const NLAIAGENT::IBasicObjectIA &a) const
	{
		return ( ((FuzzyType &)a)._Value == _Value );
	}

	std::vector<IBaseVar *> *FuzzyType::getVars()
	{
		return new std::vector<IBaseVar *>;
	}

	const NLAIC::CIdentType &FuzzyType::getType() const
	{
		return IdFuzzyType;
	}

	NLAIAGENT::IObjetOp &FuzzyType::operator += (const NLAIAGENT::IObjetOp &a)
	{
		_Value += ((FuzzyType &)a)._Value;
		return ( *this );
	}

	NLAIAGENT::IObjetOp &FuzzyType::operator -= (const NLAIAGENT::IObjetOp &a)
	{
		_Value -= ((FuzzyType &)a)._Value;
		return ( *this );
	}

	NLAIAGENT::IObjetOp &FuzzyType::operator *= (const NLAIAGENT::IObjetOp &a)
	{
		_Value *= ((FuzzyType &)a)._Value;
		return ( *this );
	}

	NLAIAGENT::IObjetOp *FuzzyType::operator < (NLAIAGENT::IObjetOp &a) const
	{
		return new CBoolType( _Value < ((FuzzyType &)a)._Value );
	}

	NLAIAGENT::IObjetOp *FuzzyType::operator > (NLAIAGENT::IObjetOp &a) const
	{
		return new CBoolType( _Value > ((FuzzyType &)a)._Value );
	}

	NLAIAGENT::IObjetOp *FuzzyType::operator <= (NLAIAGENT::IObjetOp &a) const
	{
		return new CBoolType( _Value <= ((FuzzyType &)a)._Value );
	}

	NLAIAGENT::IObjetOp *FuzzyType::operator >= (NLAIAGENT::IObjetOp &a) const
	{
		return new CBoolType( _Value >= ((FuzzyType &)a)._Value );
	}

	NLAIAGENT::IObjetOp *FuzzyType::operator ! () const
	{
		return new CBoolType( !_Value );
	}

	NLAIAGENT::IObjetOp *FuzzyType::operator != (NLAIAGENT::IObjetOp &a) const
	{
		return new CBoolType( _Value != ((FuzzyType &)a)._Value );
	}

	NLAIAGENT::IObjetOp *FuzzyType::operator == (NLAIAGENT::IObjetOp &a) const
	{
		return new CBoolType( _Value == ((FuzzyType &)a)._Value );
	}

}
