/** \file interpret_object.cpp
 *
 * $Id: interpret_object.cpp,v 1.13 2002/08/21 13:58:33 lecroart Exp $
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


namespace NLAISCRIPT
{

	void CComponent::save(NLMISC::IStream &os)
	{
		os.serial( (NLAIC::CIdentType &) RegisterName->getType() );
		RegisterName->save( os );
		os.serial( (NLAIC::CIdentType &) ObjectName->getType() );
		ObjectName->save( os );
	}

	void CComponent::load(NLMISC::IStream &is)
	{
		NLAIC::CIdentTypeAlloc id;
		is.serial( id );
		RegisterName = (NLAIAGENT::IVarName *) id.allocClass();
		RegisterName->load( is );
		RegisterName->incRef();

		is.serial( id );
		ObjectName = (NLAIAGENT::IVarName *) id.allocClass();
		ObjectName->load( is );
		ObjectName->incRef();
	}


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

	IClassInterpret::IClassInterpret(const NLAIAGENT::IVarName &name, const IClassInterpret &a): IObjectIA(a), _Name( (NLAIAGENT::IVarName *) name.clone() )
	{				
		_IdType = new NLAIC::CIdentType( name.getString(), CClassInterpretFactory( this ) , NLAIC::CTypeOfObject::tAgent, 0);
	}

	IClassInterpret::IClassInterpret(NLAIC::CIdentType *idType):_Name(NULL),_IdType(idType)
	{
	}

	IClassInterpret::~IClassInterpret()
	{
		if (_Name)
			_Name->release();

		if(_IdType) 
			_IdType->release();
	}

	IClassInterpret::IClassInterpret():_Name(NULL),_IdType(NULL)
	{
	}

	const NLAIC::CIdentType &IClassInterpret::getType() const
	{
		return *_IdType;
	}		

	void IClassInterpret::setType(NLAIC::CIdentType *idType)
	{
		if(_IdType != NULL) _IdType->release();
		_IdType = idType;
		//_IdType->incRef();
	}

	void IClassInterpret::setType(const NLAIAGENT::IVarName &name, const IClassInterpret &a)
	{
		if(_IdType != NULL) _IdType->release();
		_IdType = new NLAIC::CIdentType( name.getString(), CClassInterpretFactory( this ), NLAIC::CTypeOfObject::tAgent, 0);
	}

	void IClassInterpret::setClassName(const NLAIAGENT::IVarName &name)
	{
		if(_Name != NULL)
		{
			_Name->release();				
		}
		_Name = (NLAIAGENT::IVarName *)name.clone();

	}

	const NLAIAGENT::IVarName *IClassInterpret::getClassName() const
	{
		return _Name;
	}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

	CClassInterpretFactory::CClassInterpretFactory(IClassInterpret *a) : _Inst(a)
	{			

/*#ifdef NL_DEBUG
		static sint turc = 0;
		nlinfo("%d CClassInterpretFactory(%0x)",turc++, _Inst);
#endif*/
		_Inst->incRef();
	}

	CClassInterpretFactory::CClassInterpretFactory(const IClassInterpret &a) : _Inst((IClassInterpret *)a.clone())
	{
	}

	CClassInterpretFactory::~CClassInterpretFactory()
	{
		_Inst->release();
	}	

	void CClassInterpretFactory::setClass(const NLAIC::IBasicInterface &c)
	{
		_Inst->release();
		_Inst = (IClassInterpret *)c.clone();
	}

	void CClassInterpretFactory::getDebugString(std::string &text) const
	{
		text += NLAIC::stringGetBuild("class factory: CClassInterpretFactory fabrique des instances pour l'interface %s",(const char *)_Inst->getType());
	}

	const NLAIC::IBasicType *CClassInterpretFactory::clone() const
	{
		 NLAIC::IBasicType *x = new CClassInterpretFactory(_Inst);
		 return x;
	}

	const NLAIC::IBasicType *CClassInterpretFactory::newInstance() const
	{
		return clone();
	}

	const NLAIC::CIdentType &CClassInterpretFactory::getType() const
	{
		return _Inst->getType();
	}

	const NLAIC::IBasicInterface *CClassInterpretFactory::createInstance() const 
	{
		return (const NLAIC::IBasicInterface *)_Inst->buildNewInstance();
	}

	const NLAIC::IBasicInterface *CClassInterpretFactory::getClass() const 
	{
		return _Inst;
	}
}
