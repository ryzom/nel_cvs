/** \file interpret_methodes.cpp
 *
 * $Id: interpret_methodes.cpp,v 1.19 2002/08/21 13:58:33 lecroart Exp $
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
#include "nel/ai/script/interpret_object.h"
#include "nel/ai/script/interpret_methodes.h"
#include "nel/ai/script/type_def.h"
#include <math.h>
#include <stdarg.h>

namespace NLAISCRIPT
{
	//const NLAIAGENT::TProcessStatement CParam::_RunState = NLAIAGENT::processIdle;

	CParam::CParam():_Param(0),_ParamInfo(NULL){}
	CParam::CParam(const CParam &p):NLAIAGENT::IObjectIA(p),_Param(p._Param.size()),_ParamInfo(NULL)
	{
		for(sint32 i = 0; i<p.size(); i++)
		{
			_Param[i] = (IOpType *)p._Param[i]->clone();//new NLAIC::CIdentType(*p._Param[i]);
		}
	}

	CParam::CParam(int count, ...):_ParamInfo(NULL)
	{
		va_list marker;
		
		va_start( marker, count );
		while(count --)
		{
			IOpType *o = va_arg( marker, IOpType *);
			_Param.push_back(o);
		}
	}

	bool CParam::operator == (const CParam &p) const
	{
		sint32 l;
		if(( l = _Param.size() ) != (sint32)p._Param.size()) return false;
		for(sint32 i = 0; i<l; i++)
		{
			IOpType &p1 = *_Param[i], &p2 = *p._Param[i];
			if(p1.getConstraintTypeOf() == NULL || p2.getConstraintTypeOf() == NULL) return false;
			if(!(*p1.getConstraintTypeOf() == *p2.getConstraintTypeOf())) return false;			
		}
		return true;
	}

	double CParam::eval(const CParam &p) const
	{
		sint32 l;
		double D = 0.0;		
		if(( l = _Param.size() ) != (sint32)p._Param.size()) return -1.0;
		double k = 1.0;
		for(sint32 i = 0; i<l; i++)
		{
			IOpType &p1 = *_Param[i], &p2 = *p._Param[i];
			double r = p1.eval(&p2);
			if( r < 0) return -1.0;
			D += (r*r)*k;
			k += 1.0;
			/*if(p1.getConstraintTypeOf() != NULL && p2.getConstraintTypeOf() != NULL)
			{				
				
				if(!(p1 == p2))
				{
					const NLAIC::CIdentType &idG = *p1.getConstraintTypeOf();
					const NLAIC::CIdentType &idD = *p2.getConstraintTypeOf();
					if(!(idG == idD))
					{
						if(((const NLAIC::CTypeOfObject &)idD) & NLAIC::CTypeOfObject::tAgentInterpret)
						{
							const IClassInterpret *o = (const IClassInterpret *)((CClassInterpretFactory *)idD.getFactory())->getClass();						
							bool type = false;						
							sint32 i = 0;						
							while(o != NULL)
							{
								i ++;							
								if( o->getType() == idG)
								{
									d = ((double)(o->sizeVTable() - i - 1))*k;
									D += d*d;
									type = true;
								}
								o = o->getBaseClass();
							}
							if(!type) return -1.0;
						}
						else
						{
							//NLAIC::CTypeOfObject o_t(tNombre | tString | tList | tLogic);
							if(((const NLAIC::CTypeOfObject &)idD) & ((const NLAIC::CTypeOfObject &)idG))
							{
								return 0.0;
							}
							else return -1.0;
						}
					}
				}
				else return 0.0;
				
				
			}*/		
		}
		if(D != 0.0) return sqrt(D);
		else return 0.0;
	}

	void CParam::operator = (const CParam &p)
	{
		for(sint32 i = 0; i < p.size()  ; i ++)
		{
			_Param.push_back((IOpType *)p[i]->clone());
		}
	}

	const NLAIC::IBasicType *CParam::clone() const
	{
		CParam *cl = new CParam( *this );
		return cl;
	}

	const NLAIC::IBasicType *CParam::newInstance() const
	{
		CParam *instance = new CParam();
		return instance;
	}


	const IOpType *CParam::operator[] (sint32 i) const
	{
		return _Param[i];
	}

	void CParam::push(const NLAIAGENT::IBaseGroupType &g)
	{
		NLAIAGENT::CConstIteratorContener  i = g.getConstIterator();

		while( !i.isInEnd() )
		{
			const NLAIAGENT::IObjectIA *o = i++;
			push(COperandSimple(new NLAIC::CIdentType(o->getType())));
		}
	}

	void CParam::push(const IOpType &i)
	{
		_Param.push_back((IOpType*)i.clone());
	}

	void CParam::push(IOpType *i)
	{
		_Param.push_back(i);
	}

	const IOpType &CParam::get() const
	{
		return *_Param.back();
	}

	void CParam::pop()
	{
		_Param.pop_back();
	}

	void CParam::clear()
	{
		while(_Param.size())
		{
			IOpType *i = _Param.back();
			i->release();
			pop();
		}
	}

	sint32 CParam::size() const
	{
		return _Param.size();
	}

	const NLAIAGENT::IObjectIA::CProcessResult &CParam::run()
	{
		return NLAIAGENT::IObjectIA::ProcessRun;;
	}

	CParam::~CParam()
	{			
		for(int i = 0; i < (int)_Param.size(); i++)
		{
			_Param[i]->release();
		}
	}

	void CParam::getDebugString(std::string &t) const
	{		
		getString(t);
	}

	void CParam::load(NLMISC::IStream &is)
	{
		sint32 n;
		int i;
		is.serial(n);
		for(i = 0; i < (int)_Param.size(); i++)
		{
			_Param[i]->release();
		}
		_Param.clear();
		_Param.resize(n);
		for(i = 0; i < (int)_Param.size(); i++)
		{			
			_Param[i] = (IOpType *)IOpType::loadIOpType(is);
		}
	}

	void CParam::save(NLMISC::IStream &os)
	{
		sint32 n = _Param.size();
		os.serial(n);
		for(int i = 0; i < (int)_Param.size(); i++)
		{
			_Param[i]->serial(os);
		}
	}

	bool CParam::isEqual(const NLAIAGENT::IBasicObjectIA &a) const
	{
		// TODO
		const CParam &p = (const CParam &)a;
		return *this == p;
	}

	const NLAIC::CIdentType &CParam::getType() const
	{
		return IdParam;
	}

	void CParam::getString(std::string &txt) const
	{
		txt += "(";
		if(size())
		{		
			int i;
			for( i = 0; i < size() - 1; i++)
			{
				const NLAIC::CIdentType *id = _Param[i]->getConstraintTypeOf();
				if(id) txt += (const char *)*id;
				else txt += "UNDEF";
				txt += ",";
			}
			const NLAIC::CIdentType *id = _Param[i]->getConstraintTypeOf();
			if(id) txt += (const char *)*id;
			else txt += "UNDEF";
		}
		txt += ")";
	}


	void CParam::setInfo(NLAIAGENT::IBaseGroupType *pInfo)
	{
		if(_ParamInfo != NULL) _ParamInfo->release();
		_ParamInfo = pInfo;
	}




////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////


	CMethodeName::CMethodeName(const NLAIAGENT::IVarName &name):_MethodeName((NLAIAGENT::IVarName *)name.clone()),_Code(NULL)
	{
		_TypeOfMethode = NULL;
	}
	CMethodeName::CMethodeName():_MethodeName(NULL),_Code(NULL)
	{
		_TypeOfMethode = NULL;
	}
	CMethodeName::~CMethodeName()
	{
		if ( _MethodeName != NULL ) 
				_MethodeName->release();

		if ( _Code != NULL ) 
				_Code->release();

		if ( _TypeOfMethode != NULL )
				_TypeOfMethode->release();


	}

	CMethodeName::CMethodeName(const CMethodeName &c)
	{
		_MethodeName = (NLAIAGENT::IVarName *) c._MethodeName->clone();

		if ( c._TypeOfMethode )
			_TypeOfMethode = (NLAISCRIPT::IOpType *) c._TypeOfMethode->clone();

		_MethodeName = (NLAIAGENT::IVarName *)c._MethodeName->clone();

		_Code = c._Code;
		_Code->incRef();
	}

	const NLAIC::IBasicType *CMethodeName::clone() const
	{
		CMethodeName *cl = new CMethodeName( *this );
		return cl;
	}

	const NLAIC::IBasicType *CMethodeName::newInstance() const
	{
		CMethodeName *instance = new CMethodeName;
		return instance;
	}


	void CMethodeName::setName(const NLAIAGENT::IVarName &name)
	{
		if(_MethodeName != NULL)
		{
			_MethodeName->release();				
		}
		_MethodeName = (NLAIAGENT::IVarName *)name.clone();
	}

	const NLAIAGENT::IVarName &CMethodeName::getName() const
	{
		if(_MethodeName == NULL)
			throw NLAIE::CExceptionUnReference("invalide reference for methodeName in class CMethodeName");

		return *_MethodeName;
	}
	
	void CMethodeName::setParam(const CParam &p)
	{
		_Param = p;
	}

	CParam &CMethodeName::getParam()
	{
		return _Param;
	}

	void CMethodeName::setCode(IOpCode *c)
	{			
		_Code = c;
	}

	IOpCode *CMethodeName::getCode()
	{
		return _Code;
	}

	void CMethodeName::setTypeOfMethode(IOpType *id)
	{
		if ( _TypeOfMethode != NULL )
				_TypeOfMethode->release();
		
		_TypeOfMethode = id;		
	}
	const IOpType *CMethodeName::getTypeOfMethode() const
	{
		return _TypeOfMethode;
	}

	void CMethodeName::getDebugString(std::string &t) const
	{
		std::string txt;
		_Param.getDebugString(txt);
		t += std::string(_MethodeName->getString()) + txt;
	}

	void CMethodeName::save(NLMISC::IStream &os)
	{	
		os.serial( (NLAIC::CIdentType &) _MethodeName->getType() );
		_MethodeName->save( os );

		os.serial( (NLAIC::CIdentType &) _Code->getType() );
		_Code->save( os );
	}

	void CMethodeName::load(NLMISC::IStream &is)
	{
		NLAIC::CIdentTypeAlloc id;
		is.serial( id );
		_MethodeName = (NLAIAGENT::IVarName *) id.allocClass();
		_MethodeName->load( is );
		_MethodeName->incRef();
		
		is.serial( id );
		_Code = (IOpCode *) id.allocClass();
		_Code->load( is );
		_Code->incRef();
	}	

	bool CMethodeName::isEqual(const NLAIAGENT::IBasicObjectIA &) const
	{
		// TODO
		return false;
	}

	const NLAIAGENT::IObjectIA::CProcessResult &CMethodeName::run()
	{
		return NLAIAGENT::IObjectIA::ProcessRun;
	}

	const NLAIC::CIdentType &CMethodeName::getType() const
	{
		return IdMethodeName;
	}
}
