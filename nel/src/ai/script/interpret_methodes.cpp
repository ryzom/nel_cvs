/** \file interpret_methodes.cpp
 *
 * $Id: interpret_methodes.cpp,v 1.1 2001/01/05 10:53:49 chafik Exp $
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
#include "script/interpret_object.h"
#include "script/interpret_methodes.h"
#include "script/type_def.h"
#include <math.h>

namespace NLIASCRIPT
{
	//const NLIAAGENT::TProcessStatement CParam::_RunState = NLIAAGENT::processIdle;

	CParam::CParam():_Param(0){}
	CParam::CParam(const CParam &p):NLIAAGENT::IObjectIA(p),_Param(p._Param.size())
	{
		for(sint32 i = 0; i<p.size(); i++)
		{
			_Param[i] = (IOpType *)p._Param[i]->clone();//new NLIAC::CIdentType(*p._Param[i]);
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
		double d;
		if(( l = _Param.size() ) != (sint32)p._Param.size()) return -1.0;
		double k = 1.0;
		for(sint32 i = 0; i<l; i++)
		{
			IOpType &p1 = *_Param[i], &p2 = *p._Param[i];
			if(p1.getConstraintTypeOf() != NULL && p2.getConstraintTypeOf() != NULL)
			{
				const NLIAC::CIdentType &idG = *p1.getConstraintTypeOf();
				const NLIAC::CIdentType &idD = *p2.getConstraintTypeOf();
				if(!(idG == idD))
				{
					if(((const NLIAC::CTypeOfObject &)idD) & NLIAC::CTypeOfObject::tAgentInterpret)
					{
						const IClassInterpret *o = (const IClassInterpret *)((CClassInterpretFactory *)idD.getFactory())->getClass();						
						bool type = false;
						/*for(sint32 i = o->sizeVTable() - 1; i >= 1; i--)
						{
							if( o->getInheritance(i)->getType() == idG)
							{
								d = ((double)(o->sizeVTable() - i - 1))*k;
								D += d*d;
								type = true;
							}							
						}*/
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
					else return -1.0;
				}
				k += 1.0;
			}			
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

	const NLIAC::IBasicType *CParam::clone() const
	{
		CParam *cl = new CParam( *this );
		cl->incRef();
		return cl;
	}

	const NLIAC::IBasicType *CParam::newInstance() const
	{
		CParam *instance = new CParam();
		instance->incRef();
		return instance;
	}


	const IOpType *CParam::operator[] (sint32 i) const
	{
		return _Param[i];
	}

	void CParam::push(const NLIAAGENT::IBaseGroupType &g)
	{
		NLIAAGENT::CIteratorContener  i = g.getIterator();

		while( !i.isInEnd() )
		{
			const NLIAAGENT::IObjectIA *o = i++;
			push(COperandSimple(new NLIAC::CIdentType(o->getType())));
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

	const NLIAAGENT::IObjectIA::CProcessResult &CParam::run()
	{
		return NLIAAGENT::IObjectIA::ProcessRun;;
	}

	CParam::~CParam()
	{			
		for(int i = 0; i < (int)_Param.size(); i++)
		{
			_Param[i]->release();
		}
	}

	void CParam::getDebugString(char *t) const
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
		tParam &p = (tParam &)_Param;
		sint32 n = _Param.size();
		os.serial(n);
		for(int i = 0; i < (int)_Param.size(); i++)
		{
			_Param[i]->serial(os);
		}
	}

	bool CParam::isEqual(const NLIAAGENT::IBasicObjectIA &a) const
	{
		// TODO
		const CParam &p = (const CParam &)a;
		return *this == p;
	}

	const NLIAC::CIdentType &CParam::getType() const
	{
		return IdParam;
	}

	void CParam::getString(char *txt) const
	{
		strcpy(txt,"(");
		if(size())
		{							
			for(sint32 i = 0; i < size() - 1; i++)
			{
				const NLIAC::CIdentType *id = _Param[i]->getConstraintTypeOf();
				if(id) strcat(txt,(const char *)*id);
				else strcat(txt,"UNDEF");
				strcat(txt,",");
			}
			const NLIAC::CIdentType *id = _Param[i]->getConstraintTypeOf();
			if(id) strcat(txt,(const char *)*id);
			else strcat(txt,"UNDEF");
		}
		strcat(txt,")");
	}


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////


	CMethodeName::CMethodeName(const NLIAAGENT::IVarName &name):_MethodeName((NLIAAGENT::IVarName *)name.clone()),_Code(NULL)
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
		_MethodeName = (NLIAAGENT::IVarName *)c._MethodeName->clone();
		_Code = c._Code;
		_Code->incRef();
	}

	const NLIAC::IBasicType *CMethodeName::clone() const
	{
		CMethodeName *cl = new CMethodeName( *this );
		cl->incRef();
		return cl;
	}

	const NLIAC::IBasicType *CMethodeName::newInstance() const
	{
		CMethodeName *instance = new CMethodeName;
		instance->incRef();
		return instance;
	}


	void CMethodeName::setName(const NLIAAGENT::IVarName &name)
	{
		if(_MethodeName != NULL)
		{
			_MethodeName->release();				
		}
		_MethodeName = (NLIAAGENT::IVarName *)name.clone();
	}

	const NLIAAGENT::IVarName &CMethodeName::getName() const
	{
		if(_MethodeName == NULL)
			throw NLIAE::CExceptionUnReference("invalide reference for methodeName in class CMethodeName");

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
		//_Code->incRef();
	}

	IOpCode &CMethodeName::getCode()
	{
		return *_Code;
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

	void CMethodeName::getDebugString(char *t) const
	{
		char txt[2048*4];
		_Param.getDebugString(txt);
		sprintf(t, "%s%s",_MethodeName->getString(),txt);
	}

	void CMethodeName::save(NLMISC::IStream &os)
	{	
		os.serial( (NLIAC::CIdentType &) _MethodeName->getType() );
		_MethodeName->save( os );

		os.serial( (NLIAC::CIdentType &) _Code->getType() );
		_Code->save( os );
	}

	void CMethodeName::load(NLMISC::IStream &is)
	{
		NLIAC::CIdentTypeAlloc id;
		is.serial( id );
		_MethodeName = (NLIAAGENT::IVarName *) id.allocClass();
		_MethodeName->load( is );
		_MethodeName->incRef();
		
		is.serial( id );
		_Code = (IOpCode *) id.allocClass();
		_Code->load( is );
		_Code->incRef();
	}	

	bool CMethodeName::isEqual(const NLIAAGENT::IBasicObjectIA &) const
	{
		// TODO
		return false;
	}

	const NLIAAGENT::IObjectIA::CProcessResult &CMethodeName::run()
	{
		return NLIAAGENT::IObjectIA::ProcessRun;
	}

	const NLIAC::CIdentType &CMethodeName::getType() const
	{
		return IdMethodeName;
	}

}

