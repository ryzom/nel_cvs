/** \file fuzzyset.cpp
 * Fuzzy sets: triangle, trapeze...
 *
 * $Id: fuzzyset.cpp,v 1.11 2002/08/21 13:58:33 lecroart Exp $
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
#include "nel/ai/fuzzy/fuzzyset.h"

namespace NLAIFUZZY
{
	
	void IFuzzySet::addFact(double value)
	{
		_Facts.push_back( value );
	}

	double IFuzzySet::agregate()
	{
		if ( _Facts.size() )
		{
			// min
			double min = 1;
			while ( _Facts.size() )
			{
				if ( _Facts.front() < min )
					min = _Facts.front();
				_Facts.pop_front();
			}
			_Value = min;
			return min;
		}
		else 
		{	
			_Value = 0.0;
			return 0.0;
		}
	}

	double IFuzzySet::getValue()
	{
		return _Value;
	}

	CFuzzyInterval::CFuzzyInterval(char *name,double min, double max) : IFuzzySet(name)
	{
		_Min = min;
		_Max = max;
	}

	CFuzzyInterval::CFuzzyInterval(const CFuzzyInterval &cp) : IFuzzySet(cp._Name)
	{
		_Min = cp._Min;
		_Max = cp._Max;
	}

	void CFuzzyInterval::init(NLAIAGENT::IObjectIA *params)
	{
		if ( ((NLAIAGENT::IBaseGroupType *)params)->size() != 3 ) 
		{
		//	throw Exc::
		}
		NLAIAGENT::IObjectIA  * arg = (NLAIAGENT::IObjectIA  *) ((NLAIAGENT::IBaseGroupType *)params)->popFront();
		_Min = (double) ((NLAIAGENT::DDigitalType *) arg )->getValue();
		arg->release();
		arg = (NLAIAGENT::IObjectIA  *) ((NLAIAGENT::IBaseGroupType *)params)->popFront();
		_Max = (double) ((NLAIAGENT::DDigitalType *) arg)->getValue();
		arg->release();
	}

	double CFuzzyInterval::membership(double val)
	{
		if ( val >= _Min && val <= _Max )
			return 1.0;
		else
			return 0.0;
	}

	bool CFuzzyInterval::isIn(double val)
	{
		if ( val >= _Min && val <= _Max )
			return true;
		else
			return false;
	}

	double CFuzzyInterval::surface()
	{
		return ( _Max - _Min );
	}

	double CFuzzyInterval::center()
	{
		return ( _Min + _Max ) /2;
	}


	const NLAIC::IBasicType *CFuzzyInterval::clone() const 
	{
		CFuzzyInterval *clone = new CFuzzyInterval( *this );
		return clone;
	}

	const NLAIC::IBasicType *CFuzzyInterval::newInstance() const
	{
		NLAIC::IBasicType *instance = new CFuzzyInterval( *this );

		return instance;
	}

	const NLAIC::CIdentType &CFuzzyInterval::getType() const
	{
		return IdFuzzyInterval;
	}

	void CFuzzyInterval::save(NLMISC::IStream &os)
	{
		IFuzzySet::save(os);
		os.serial( (double &) _Min );
		os.serial( (double &) _Max );
	}

	void CFuzzyInterval::load(NLMISC::IStream &is)
	{
	}

	void CFuzzyInterval::getDebugString(std::string &txt) const
	{
		txt += NLAIC::stringGetBuild("%s [%f , %f]", _Name, _Min, _Max);
	}

	bool CFuzzyInterval::isEqual(const NLAIAGENT::IBasicObjectIA &a) const
	{
		return ( ((CFuzzyInterval &)a)._Min == _Min && ((CFuzzyInterval &)a)._Max == _Max );
	}
	
	const NLAIAGENT::IObjectIA::CProcessResult &CFuzzyInterval::run()
	{
		return NLAIAGENT::IObjectIA::ProcessRun;
	}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////


	CRightFuzzySet::CRightFuzzySet(char *name, double x1,double x2,double x3)  : IFuzzySet(name)
	{
		_X1 = x1;
		_X2 = x2;
		_X3 = x3; 
	}

	CRightFuzzySet::CRightFuzzySet(const CRightFuzzySet &cp) : IFuzzySet(cp._Name)
	{
		_X1 = cp._X1;
		_X2 = cp._X2;
		_X3 = cp._X3;
	}

	void CRightFuzzySet::init(NLAIAGENT::IObjectIA *params)
	{
		if ( ((NLAIAGENT::IBaseGroupType *)params)->size() != 3 ) 
		{
		//	throw Exc::
		}
		NLAIAGENT::IObjectIA *arg = (NLAIAGENT::IObjectIA *) ((NLAIAGENT::IBaseGroupType *)params)->popFront();
		_X1 = (double) ((NLAIAGENT::DDigitalType *) arg)->getValue();
		arg->release();
		arg = (NLAIAGENT::IObjectIA *) ((NLAIAGENT::IBaseGroupType *)params)->popFront();
		_X2 = (double) ((NLAIAGENT::DDigitalType *) arg)->getValue();
		arg->release();
		arg = (NLAIAGENT::IObjectIA *) ((NLAIAGENT::IBaseGroupType *)params)->popFront();
		_X3 = (double) ((NLAIAGENT::DDigitalType *) arg)->getValue();
		arg->release();
	}


	double CRightFuzzySet::membership(double val)
	{
		if ( val >= _X2 && val <= _X3 )
			return 1.0;

		if ( val > _X1 && val < _X2 ) 
		{
			return ( ( val - _X1 ) / ( _X2 - _X1 ) );
		}

		return 0.0;
	}

	bool CRightFuzzySet::isIn(double val)
	{
		return ( val >= _X1 && val <= _X3 );
	}

	double CRightFuzzySet::surface()
	{
		return (_X2 - _X1) /2 + (_X3 - _X2);
	}
	
	double CRightFuzzySet::center()
	{
		return ( ( (_X1 + _X2) /4 + (_X2 + _X3) /2 ) /1.5);
	}

	const NLAIC::IBasicType *CRightFuzzySet::clone() const
	{ 
		CRightFuzzySet *clone = new CRightFuzzySet( *this );
		return clone;
	}
	const NLAIC::IBasicType *CRightFuzzySet::newInstance() const
	{
		NLAIC::IBasicType *instance = new CRightFuzzySet( *this );
		return instance;
	}

	const NLAIC::CIdentType &CRightFuzzySet::getType() const
	{
		return IdRightFuzzySet;
	}

	void CRightFuzzySet::save(NLMISC::IStream &os)
	{
		IFuzzySet::save(os);
		os.serial( (double &) _X1 );
		os.serial( (double &) _X2 );
		os.serial( (double &) _X3);
	}

	void CRightFuzzySet::load(NLMISC::IStream &is)
	{
	}

	void CRightFuzzySet::getDebugString(std::string &txt) const
	{
		txt += NLAIC::stringGetBuild("CRightFuzzySet %s [%f , %f , %f]", _Name, _X1, _X2, _X3);
	}
	
	bool CRightFuzzySet::isEqual(const NLAIAGENT::IBasicObjectIA &a) const
	{
		return ( ((CRightFuzzySet &)a)._X1 == _X1 && 
				 ((CRightFuzzySet &)a)._X2 == _X2 && 
				 ((CRightFuzzySet &)a)._X3 == _X3 );
	}

	const NLAIAGENT::IObjectIA::CProcessResult &CRightFuzzySet::run()
	{
		return NLAIAGENT::IObjectIA::ProcessRun;
	}



	NLAIAGENT::tQueue IFuzzySet::isMember(const NLAIAGENT::IVarName *className,const NLAIAGENT::IVarName *funcName,const NLAIAGENT::IObjectIA &params) const
	{

#ifdef NL_DEBUG	
	std::string nameP;
	std::string nameM;
	funcName->getDebugString(nameM);
	params.getDebugString(nameP);

	const char *dbg_class_name = (const char *) getType();
#endif
		NLAIAGENT::tQueue r;
		if(className == NULL)
		{
			if( (*funcName) == NLAIAGENT::CStringVarName( "Constructor" ) )
			{					
				NLAIAGENT::CObjectType *c = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( CTriangleFuzzySet::IdTriangleFuzzySet ) );
				r.push( NLAIAGENT::CIdMethod( 0 + IObjectIA::getMethodIndexSize(), 0.0, NULL, c) );
			}

			if( (*funcName) == NLAIAGENT::CStringVarName( "Membership" ) )
			{					
				NLAIAGENT::CObjectType *c = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIAGENT::DigitalType::IdDigitalType ) );
				r.push( NLAIAGENT::CIdMethod( 1 + IObjectIA::getMethodIndexSize(), 0.0, NULL, c) );
			}

		}

		if ( r.empty() )
			return IObjectIA::isMember(className, funcName, params);
		else
			return r;
	}

	NLAIAGENT::IObjectIA::CProcessResult IFuzzySet::runMethodeMember(sint32 index, NLAIAGENT::IObjectIA *p)
	{
		switch(index - IObjectIA::getMethodIndexSize() )
		{
		case 0:
			{					
				init(p);
			}
			break;

		case 1:

			break;
		}

		return NLAIAGENT::IObjectIA::CProcessResult();
	}



/*
	const NLAIC::CIdentType CompositeiFuzzySet::idCompositeiFuzzySet = NLAIC::CIdentType("CompositeiFuzzySet",NLAIC::CSelfClassCFactory(CompositeiFuzzySet()),
		NLAIC::CTypeOfObject::tObject,
		NLAIC::CTypeOfOperator::opAdd |
		NLAIC::CTypeOfOperator::opSub |
		NLAIC::CTypeOfOperator::opEq );
	
	CompositeiFuzzySet::CompositeiFuzzySet()
	{
	}
	
	CompositeiFuzzySet::CompositeiFuzzySet(const CompositeiFuzzySet &cp)
	{
		list<IFuzzySet *>::const_iterator it_f = cp._iFuzzySets.begin();
		while ( it_f  != cp._iFuzzySets.end() )
		{
			_iFuzzySets.push_back( (IFuzzySet *) (*it_f)->clone() );
			it_f++;
		}
	}

	void CompositeiFuzzySet::addiFuzzySet(IFuzzySet *f)
	{
		_iFuzzySets.push_back(f);
	}

	double CompositeiFuzzySet::membership(double val)
	{
		list<IFuzzySet *>::iterator it_f = _iFuzzySets.begin();
		while ( it_f  != _iFuzzySets.end() )
		{
			if ( (*it_f)->isIn( val ) )
				return (*it_f)->membership( val );
			it_f++;
		}
		return 0.0;
	}

	bool CompositeiFuzzySet::isIn(double val )
	{
		list<IFuzzySet *>::iterator it_f = _iFuzzySets.begin();
		while ( it_f  != _iFuzzySets.end() )
		{
			if ( (*it_f)->isIn( val ) )
				return true;
			it_f++;
		}
		return false;
	}

	double CompositeiFuzzySet::center()
	{
		// TODO
		return 0.0;
	}

	bool CompositeiFuzzySet::isEqual(const NLAIAGENT::IBasicObjectIA &a) const
	{
		return false;
	}

	const NLAIC::IBasicType *CompositeiFuzzySet::clone() const 
	{
		return new CompositeiFuzzySet( *this );
	}

	const NLAIC::IBasicType *CompositeiFuzzySet::newInstance() const
	{
		return new CompositeiFuzzySet( *this ); 
	}

	const NLAIC::CIdentType &CompositeiFuzzySet::getType() const
	{
		return idCompositeiFuzzySet;
	}

	void CompositeiFuzzySet::save(NLMISC::IStream &os)
	{
	}

	void CompositeiFuzzySet::load(NLMISC::IStream &is)
	{
	}

	void CompositeiFuzzySet::getDebugString(char *txt) const
	{
		sprintf(txt,"CompositeiFuzzySet");
	}

	const TProcessStatement &CompositeiFuzzySet::run()
	{
		return NLAIAGENT::IObjectIA::AgentIdle;
	}
*/
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////



	CTriangleFuzzySet::CTriangleFuzzySet(char *name, double x1,double x2,double x3)  : IFuzzySet(name)
	{
		_X1 = x1;
		_X2 = x2;
		_X3 = x3;
	}

	CTriangleFuzzySet::CTriangleFuzzySet(const CTriangleFuzzySet &cp) : IFuzzySet(cp._Name)
	{
		_X1 = cp._X1;
		_X2 = cp._X2;
		_X3 = cp._X3;
	}

	void CTriangleFuzzySet::init(NLAIAGENT::IObjectIA *params)
	{
#ifdef NL_DEBUG
		std::string buf;
		params->getDebugString(buf);
#endif

		sint32 i = ((NLAIAGENT::IBaseGroupType *)params)->size();
		if ( i != 3 ) 
		{
		//	throw Exc::
		}
		NLAIAGENT::IObjectIA *arg = (NLAIAGENT::IObjectIA *) ((NLAIAGENT::IBaseGroupType *)params)->popFront();
		_X1 = (double) ((NLAIAGENT::DDigitalType *) arg)->getValue();
		arg->release();

		arg = (NLAIAGENT::IObjectIA *) ((NLAIAGENT::IBaseGroupType *)params)->popFront();
		_X2 = (double) ((NLAIAGENT::DDigitalType *) arg)->getValue();
		arg->release();

		arg = (NLAIAGENT::IObjectIA *) ((NLAIAGENT::IBaseGroupType *)params)->popFront();
		_X3 = (double) ((NLAIAGENT::DDigitalType *) arg)->getValue();
		arg->release();
	}


	double CTriangleFuzzySet::membership(double val)
	{
		if ( val > _X1 && val < _X2 )
		{
			return ( ( val - _X1 ) / ( _X2 - _X1 ) );
		}

		if ( val > _X2 && val < _X3 )
		{
			return ( ( val - _X2 ) / ( _X3 - _X2 ) );
		}

		return 0.0;
	}

	bool CTriangleFuzzySet::isIn(double val)
	{
		return ( val > _X1 && val < _X2 );
	}

	double CTriangleFuzzySet::surface()
	{
		return (_X3 - _X2) /2;
	}

	double CTriangleFuzzySet::center()
	{
		return ( ( (_X1 + _X2) /2 * (_X2 - _X1) ) +
				 ( (_X2 + _X3) /2 * (_X3 - _X2) ) ) 
				 / (_X3 - _X1);
	}

	const NLAIC::IBasicType *CTriangleFuzzySet::clone() const
	{
		CTriangleFuzzySet *clone = new CTriangleFuzzySet( *this );
		return clone;
	}
	const NLAIC::IBasicType *CTriangleFuzzySet::newInstance() const
	{
		NLAIC::IBasicType *instance = new CTriangleFuzzySet( *this );
		return instance;
	}

	const NLAIC::CIdentType &CTriangleFuzzySet::getType() const
	{
		return IdTriangleFuzzySet;
	}
	void CTriangleFuzzySet::save(NLMISC::IStream &os)
	{
		IFuzzySet::save(os);
		os.serial( (double &) _X1 );
		os.serial( (double &) _X2 );
		os.serial( (double &) _X3 );
	}

	void CTriangleFuzzySet::load(NLMISC::IStream &is)
	{
		is.serial( _X1 );
		is.serial( _X2 );
		is.serial( _X3 );
	}

	void CTriangleFuzzySet::getDebugString(std::string &txt) const
	{
		txt += NLAIC::stringGetBuild("CTriangleFuzzySet %s [%f , %f , %f]", _Name, _X1, _X2, _X3);
	}
	
	bool CTriangleFuzzySet::isEqual(const NLAIAGENT::IBasicObjectIA &a) const
	{
		return ( ((CTriangleFuzzySet &)a)._X1 == _X1 && ((CTriangleFuzzySet &)a)._X2 == _X2 && ((CTriangleFuzzySet &)a)._X3 == _X3 );
	}

	const NLAIAGENT::IObjectIA::CProcessResult &CTriangleFuzzySet::run()
	{
		return NLAIAGENT::IObjectIA::ProcessRun;
	}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
	
	
	CLeftFuzzySet::CLeftFuzzySet(char *name, double p1,double p2,double p3) : IFuzzySet(name)
	{
		_X1 = p1;
		_X2 = p2;
		_X3 = p3;
	}

	CLeftFuzzySet::CLeftFuzzySet( const CLeftFuzzySet &cp ) : IFuzzySet(cp._Name)
	{
		_X1 = cp._X1;
		_X2 = cp._X2;
		_X3 = cp._X3;
	}

	void CLeftFuzzySet::init(NLAIAGENT::IObjectIA *params)
	{
		if ( ((NLAIAGENT::IBaseGroupType *)params)->size() != 3 ) 
		{
		//	throw Exc::
		}
		
		NLAIAGENT::IObjectIA *arg = (NLAIAGENT::IObjectIA *) ((NLAIAGENT::IBaseGroupType *)params)->popFront();
		_X1 = (double) ((NLAIAGENT::DDigitalType *) arg)->getValue();
		arg->release();

		arg = (NLAIAGENT::IObjectIA *) ((NLAIAGENT::IBaseGroupType *)params)->popFront();
		_X2 = (double) ((NLAIAGENT::DDigitalType *) arg)->getValue();
		arg->release();

		arg = (NLAIAGENT::IObjectIA *) ((NLAIAGENT::IBaseGroupType *)params)->popFront();
		_X3 = (double) ((NLAIAGENT::DDigitalType *) arg)->getValue();
		arg->release();
	}

	CLeftFuzzySet::~CLeftFuzzySet()
	{
	}

	double CLeftFuzzySet::membership(double val)
	{
		if ( val >= _X1 && val <= _X2 )
			return 1.0;

		if ( val > _X2 && val < _X3 )
		{
			return ( ( val - _X2 ) / ( _X3 - _X2 ) );
		}

		return 0.0;
	}

	bool CLeftFuzzySet::isIn(double val)
	{
		return ( val >= _X1 && val <= _X3 );
	}

	double CLeftFuzzySet::surface()
	{
		return (_X2 - _X1) + (_X3 - _X2) /2;
	}

	double CLeftFuzzySet::center()
	{
		return ( ( (_X1 + _X2) /2 + (_X2 + _X3) /4 ) /1.5);
	}

	const NLAIC::IBasicType *CLeftFuzzySet::clone() const
	{ 
		CLeftFuzzySet *clone = new CLeftFuzzySet( *this );
		return clone;
	}
	const NLAIC::IBasicType *CLeftFuzzySet::newInstance() const
	{
		NLAIC::IBasicType *instance = new CLeftFuzzySet( *this );
		return instance;
	}

	const NLAIC::CIdentType &CLeftFuzzySet::getType() const
	{
		return IdLeftFuzzySet;
	}

	void CLeftFuzzySet::save(NLMISC::IStream &os)
	{
		IFuzzySet::save(os);
		os.serial( (double &) _X1 );
		os.serial( (double &) _X2 );
		os.serial( (double &) _X3 );
	}

	void CLeftFuzzySet::load(NLMISC::IStream &is)
	{
	}

	void CLeftFuzzySet::getDebugString(std::string &txt) const
	{
		txt += NLAIC::stringGetBuild("CLeftFuzzySet %s [%f , %f , %f]", _Name, _X1, _X2, _X3);
	}
	
	bool CLeftFuzzySet::isEqual(const NLAIAGENT::IBasicObjectIA &a) const
	{
		return ( ((CLeftFuzzySet &)a)._X1 == _X1 && ((CLeftFuzzySet &)a)._X2 == _X2 && ((CLeftFuzzySet &)a)._X3 == _X3 );
	}

	const NLAIAGENT::IObjectIA::CProcessResult &CLeftFuzzySet::run()
	{
		return NLAIAGENT::IObjectIA::ProcessRun;
	}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////



	CTrapezeFuzzySet::CTrapezeFuzzySet(char *name, double p1,double p2,double p3, double p4) : IFuzzySet(name)
	{
		_X1 = p1;
		_X2 = p2;
		_X3 = p3;
		_X4 = p4;
	}
 
	CTrapezeFuzzySet::CTrapezeFuzzySet(const CTrapezeFuzzySet &cp) : IFuzzySet(cp._Name)
	{
		_X1 = cp._X1;
		_X2 = cp._X2;
		_X3 = cp._X3;
		_X4 = cp._X4;
	}

	void CTrapezeFuzzySet::init(NLAIAGENT::IObjectIA *params)
	{
		if ( ((NLAIAGENT::IBaseGroupType *)params)->size() != 3 ) 
		{
		//	throw Exc::
		}
		NLAIAGENT::IObjectIA *arg = (NLAIAGENT::IObjectIA *) ((NLAIAGENT::IBaseGroupType *)params)->popFront();
		_X1 = (double) ((NLAIAGENT::DDigitalType *) arg)->getValue();
		arg->release();
		arg = (NLAIAGENT::IObjectIA *) ((NLAIAGENT::IBaseGroupType *)params)->popFront();
		_X2 = (double) ((NLAIAGENT::DDigitalType *) arg)->getValue();
		arg->release();
		arg = (NLAIAGENT::IObjectIA *) ((NLAIAGENT::IBaseGroupType *)params)->popFront();
		_X3 = (double) ((NLAIAGENT::DDigitalType *) arg)->getValue();
		arg->release();
		arg = (NLAIAGENT::IObjectIA *) ((NLAIAGENT::IBaseGroupType *)params)->popFront();
		_X4 = (double) ((NLAIAGENT::DDigitalType *)arg)->getValue();
		arg->release();

	}

	CTrapezeFuzzySet::~CTrapezeFuzzySet()
	{
	}

	double CTrapezeFuzzySet::membership(double val)
	{
		if ( val >= _X2 && val <= _X3 )
			return 1.0;

		if ( val > _X1 && val < _X2 )
		{
			return ( ( val - _X1 ) / ( _X2 - _X1 ) );
		}

		if ( val > _X3 && val < _X2 )
		{
			return ( ( val - _X3 ) / ( _X4 - _X3 ) );
		}

		return 0.0;
	}

	bool CTrapezeFuzzySet::isIn(double val)
	{
		return ( val > _X1 && val < _X4 );
	}

	double CTrapezeFuzzySet::surface()
	{
		return (_X2 - _X1) /2 + (_X3 - _X2) + (_X4 - _X3) / 2;
	}

	double CTrapezeFuzzySet::center()
	{
		return (_X1 + _X2) /2 + (_X2 + _X3) /4;
	}

	const NLAIC::IBasicType *CTrapezeFuzzySet::clone() const
	{ 
		CTrapezeFuzzySet *clone = new CTrapezeFuzzySet( *this );
		return clone;
	}
	const NLAIC::IBasicType *CTrapezeFuzzySet::newInstance() const
	{
		NLAIC::IBasicType *instance = new CTrapezeFuzzySet( *this );
		return instance;
	}

	const NLAIC::CIdentType &CTrapezeFuzzySet::getType() const
	{
		return IdTrapezeFuzzySet;
	}

	void CTrapezeFuzzySet::save(NLMISC::IStream &os)
	{
		IFuzzySet::save(os);
		os.serial( (double &) _X1 );
		os.serial( (double &) _X2 );
		os.serial( (double &) _X3 );
		os.serial( (double &) _X4 );
	}

	void CTrapezeFuzzySet::load(NLMISC::IStream &is)
	{
	}

	void CTrapezeFuzzySet::getDebugString(std::string &txt) const
	{
		txt += NLAIC::stringGetBuild("CTrapezeFuzzySet %s [%f , %f , %f , %f]", _Name, _X1, _X2, _X3, _X4);
	}
	
	bool CTrapezeFuzzySet::isEqual(const NLAIAGENT::IBasicObjectIA &a) const
	{
		return ( ((CTrapezeFuzzySet &)a)._X1 == _X1 && 
				 ((CTrapezeFuzzySet &)a)._X2 == _X2 && 
				 ((CTrapezeFuzzySet &)a)._X3 == _X3 &&
				 ((CTrapezeFuzzySet &)a)._X4 == _X4 );
	}

	const NLAIAGENT::IObjectIA::CProcessResult &CTrapezeFuzzySet::run()
	{
		return NLAIAGENT::IObjectIA::ProcessRun;
	}
}
