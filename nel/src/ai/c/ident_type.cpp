/** \file ident_type.cpp
 *
 * $Id: ident_type.cpp,v 1.20 2002/08/21 13:58:34 lecroart Exp $
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

#include <stdio.h>
#include <iostream.h>
#include <stdio.h>
#include <stdarg.h>


#include "nel/ai/c/abstract_interface.h" 
#include "nel/ai/e/ai_exception.h" 
#include "nel/ai/c/registry_class.h"
#include "nel/ai/c/registry_type.h"

namespace NLAIC
{

	//freopen( "file.txt", "w", stdout ); 
	char *stringBuild(const char *str, ...)
	{		
		char	temp[64*1024];
		va_list argument;
		va_start (argument, str);
		
		vsprintf(temp, str, argument);
		char *s = new char [strlen(temp)+1];
		strcpy(s,temp);
		return s;
	}

	const IIO *DefaultIIO = NULL;
	void setDefaultIIO(const IIO *io)
	{
		DefaultIIO = io;
	}

	const IIO *getDefaultIIO()
	{
		return DefaultIIO;
	}

	void Out(const char *str,...)
	{		
		char	temp[64*1024];
		va_list argument;
		va_start (argument, str);
		
		vsprintf(temp, str, argument);
		if(DefaultIIO != NULL) 
			DefaultIIO->Echo(temp);
		else
		{
			fprintf(stderr,temp);
			fflush(stderr);
		}
	}

	std::string stringGetBuild(const char *str, ...)
	{
		char	temp[64*1024];
		va_list argument;
		va_start (argument, str);
		
		vsprintf(temp, str, argument);			

		return std::string(temp);

	}

	CIdentType CIdentType::VoidType = CIdentType("VOID",CTypeOfObject::tObject,CTypeOfOperator(0),-1);

	CIdentType::~CIdentType()
	{
		if(_Ident != NULL) delete (char *)_Ident;
		if(_ObjType != NULL) _ObjType->release();
		if(_OpSupport != NULL) _OpSupport->release();
	}

	CIdentType::CIdentType(NLMISC::IStream &f)/// throw(NLMISC::EStream)
	{
		_ObjType = NULL;
		_OpSupport = NULL;
		_Ident = NULL;
		serial(f);
	}

	CIdentType::CIdentType(const char *ident,const CTypeOfObject &objType,const CTypeOfOperator &opSupport,sint32 index): _ObjType((CTypeOfObject *)objType.clone()),
															      _OpSupport((CTypeOfOperator *)opSupport.clone()),
															      _Index(-1)
	{
		_Ident = new char [strlen(ident) + 1];
		strcpy((char *)_Ident,ident);
		_Index = index;
	}
	
	CIdentType::CIdentType(const char *ident,const IClassFactory &classCFactory,const CTypeOfObject &objType,const CTypeOfOperator &opSupport): _ObjType((CTypeOfObject *)objType.clone()),
																		    _OpSupport((CTypeOfOperator *)opSupport.clone()),
																		    _Index(-1)
	{	
		_Ident = new char [strlen(ident) + 1];
		strcpy((char *)_Ident,ident);
				
		try
		{				
			CRegistry *r = getRegistry();
			_Index = r->registerClass(*this,classCFactory);
		}
		catch (NLAIE::IException &err)
		{
			delete _Ident;
			_Ident = NULL;
			_ObjType->release();
			_OpSupport->release();
			throw NLAIE::CExceptionContainer(err.what());
		}		
	}

	
	CIdentType::CIdentType(const char *ident): _Index(-1)
	{
		_Ident = new char [strlen(ident) + 1];
		strcpy((char *)_Ident,ident);

		try
		{				
			CRegistry *r = getRegistry();			
			_Index = r->getNumIdent(*this);
			const CIdentType &id= r->getIdent(_Index);
			_ObjType = (CTypeOfObject *)&((const CTypeOfObject &)id);
			_ObjType->incRef();
			_OpSupport = (CTypeOfOperator *)&((const CTypeOfOperator &)id);
			_OpSupport->incRef();
		}
		catch (NLAIE::IException &err)
		{
			delete _Ident;
			_Ident = NULL;
			_ObjType = NULL;
			_OpSupport = NULL;
			NLAIE::CExceptionContainer e(err.what());
			throw e;
		}		
	}

	CIdentType::CIdentType(const CIdentType &i):_ObjType((CTypeOfObject *)i._ObjType->clone()),_OpSupport((CTypeOfOperator *)i._OpSupport->clone()),_Index(-1)
	{
		_Index = i._Index;
		_Ident = new char [strlen(i._Ident) + 1];
		strcpy((char *)_Ident,i._Ident);
	}

	const IBasicInterface *CIdentType::allocClass() const
	{
		return getRegistry()->createInstance(_Index);
	}

	const IClassFactory *CIdentType::getFactory() const
	{
		return getRegistry()->getFactory(_Index);
	}

	void CIdentType::addObjectType(sint32 t)
	{
		if ( _ObjType != NULL )
			_ObjType->addType(t);
	}

	
	void CIdentType::operator = (const CIdentType &i)
	{
		delete _Ident;
		_Ident = new char [strlen(i._Ident) + 1];
		strcpy((char *)_Ident,i._Ident);		
		_Index = i._Index;
	}	

	void CIdentType::serial(NLMISC::IStream	&f) throw(NLMISC::EStream)
	{
		if(!f.isReading())
		{			
			f.serial(_Index);
			std::string x(_Ident);
			f.serial(x);
		}
		else
		{
			std::string s;
			sint32 n;			
			f.serial(n);
			f.serial(s);

			if(_Ident) delete _Ident;
			
			_Ident = new char [strlen(s.data()) + 1];
			strcpy((char *)_Ident,s.data());
			try
			{				
				_Index = getRegistry()->getNumIdent(*this);
				const CIdentType &id= getRegistry()->getIdent(_Index);
				if(_ObjType) _ObjType->release();
				if(_OpSupport) _OpSupport->release();

				_ObjType = (CTypeOfObject *)&((const CTypeOfObject &)id);
				_ObjType->incRef();
				_OpSupport = (CTypeOfOperator *)&((const CTypeOfOperator &)id);
				_OpSupport->incRef();
			}
			catch (NLAIE::IException &)
			{
				throw NLMISC::EStream();
			}
			
		}
	}

	void CIdentTypeAlloc::load(NLMISC::IStream &is) 
	{
		if(_Id != NULL) 
			delete _Id;

		/*sint32 i;
		is.serial(i);		
		std::string cn;
		is.serial( cn );
		char *className = new char [strlen(cn.data()) + 1];
		strcpy( className, cn.data() );		

		CTypeOfOperator op(is);
		CTypeOfObject ty(is);*/
		
		_Id = new CIdentType(is);
		//delete [] className;
	}	


	void CIdentTypeAlloc::serial(NLMISC::IStream &is)  throw(NLMISC::EStream)
	{
		if(is.isReading()) load(is);
		else
		{
			if(_Id != NULL) 
			{
				_Id->serial(is);
			}
			else
			{
				throw NLMISC::EStream();
			}
		}
		
	}	

	NLMISC::IStream &operator << (NLMISC::IStream &os, CIdentType &o)
	{		
		o.serial(os);
		return os;
	}
	
}
