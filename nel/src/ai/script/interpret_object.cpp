/** \file object.cpp
 *
 * $Id: interpret_object.cpp,v 1.1 2001/01/05 10:53:49 chafik Exp $
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


namespace NLIASCRIPT
{

	void CComponent::save(NLMISC::IStream &os)
	{
		os.serial( (NLIAC::CIdentType &) RegisterName->getType() );
		RegisterName->save( os );
		os.serial( (NLIAC::CIdentType &) ObjectName->getType() );
		ObjectName->save( os );
	}

	void CComponent::load(NLMISC::IStream &is)
	{
		NLIAC::CIdentTypeAlloc id;
		is.serial( id );
		RegisterName = (NLIAAGENT::IVarName *) id.allocClass();
		RegisterName->load( is );
		RegisterName->incRef();

		is.serial( id );
		ObjectName = (NLIAAGENT::IVarName *) id.allocClass();
		ObjectName->load( is );
		ObjectName->incRef();
	}


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

	IClassInterpret::IClassInterpret(const NLIAAGENT::IVarName &name, const IClassInterpret &a): IObjectIA(a), _Name( (NLIAAGENT::IVarName *) name.clone() )
	{				
		_IdType = new NLIAC::CIdentType( name.getString(), CClassInterpretFactory( this ) , NLIAC::CTypeOfObject::tAgent, 0);
		_IdType->incRef();
	}

	IClassInterpret::IClassInterpret(NLIAC::CIdentType *idType):_IdType(idType),_Name(NULL)
	{
	}

	IClassInterpret::~IClassInterpret()
	{
		if (_Name)
			_Name->release();

		if(_IdType) 
			_IdType->release();
	}

	IClassInterpret::IClassInterpret():_IdType(NULL), _Name(NULL)
	{
	}

	const NLIAC::CIdentType &IClassInterpret::getType() const
	{
		return *_IdType;
	}		

	void IClassInterpret::setType(NLIAC::CIdentType *idType)
	{
		_IdType = idType;
		_IdType->incRef();
	}

	void IClassInterpret::setType(const NLIAAGENT::IVarName &name, const IClassInterpret &a)
	{
		const char *txt = name.getString();
		_IdType = new NLIAC::CIdentType( name.getString(), CClassInterpretFactory( this ), NLIAC::CTypeOfObject::tAgent, 0);
		_IdType->incRef();
	}

	void IClassInterpret::setClassName(const NLIAAGENT::IVarName &name)
	{
		if(_Name != NULL)
		{
			_Name->release();				
		}
		_Name = (NLIAAGENT::IVarName *)name.clone();

	}

	const NLIAAGENT::IVarName *IClassInterpret::getClassName() const
	{
		return _Name;
	}


///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

	CClassInterpretFactory::CClassInterpretFactory(IClassInterpret *a) : _inst(a)
	{			
		_inst->incRef();
	}

	CClassInterpretFactory::~CClassInterpretFactory()
	{
		_inst->release();
	}	

	void CClassInterpretFactory::getDebugString(char *text) const
	{
		sprintf(text,"class factory: CClassInterpretFactory fabrique des instances pour l'interface %s",(const char *)_inst->getType());
	}

	const NLIAC::IBasicType *CClassInterpretFactory::clone() const
	{
		 NLIAC::IBasicType *x = new CClassInterpretFactory(_inst);
		 x->incRef();
		 return x;
	}

	const NLIAC::IBasicType *CClassInterpretFactory::newInstance() const
	{
		return clone();
	}

	const NLIAC::CIdentType &CClassInterpretFactory::getType() const
	{
		return _inst->getType();
	}

	const NLIAC::IBasicInterface *CClassInterpretFactory::createInstance() const 
	{
		return (const NLIAC::IBasicInterface *)_inst->buildNewInstance();
	}

	const NLIAC::IBasicInterface *CClassInterpretFactory::getClass() const 
	{
		return _inst;
	}
}