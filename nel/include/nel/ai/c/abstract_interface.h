/** \file registry_type.h
 * Includes some basic class objects.
 *
 * $Id: abstract_interface.h,v 1.24 2002/08/21 13:58:17 lecroart Exp $
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

#ifndef NL_ABSTRACT_H
#define NL_ABSTRACT_H

#ifdef NL_OS_WINDOWS

#pragma warning (disable: 4666)
#pragma warning (disable: 4786) 
#pragma warning (disable: 4620) 
#pragma warning (disable: 4275)
#pragma warning (disable: 4251)

#endif

#include <map>
#include <iostream>
#include <string>
#include "nel/misc/file.h"
#include "nel/ai/e/ai_exception.h"

namespace NLAIC //Gen pour general.
{

	///this method give an char pointer hows it contents a string define by a const char* same as printf. It use for initialise string objects.
	char *stringBuild(const char *str, ...);	

	///this method give an char pointer hows it contents a string define by a const char* same as printf. It use for initialise string objects.
	std::string stringGetBuild(const char *str, ...);	

	/**
	IPointerGestion define a reference for an objects. It use for manager objects. 
	The object encapsulat the nomber of objects hows refernce this object.

	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000
	*/
	class IPointerGestion : public NLMISC::IStreamable
	{
	private:
		///Nomber of reference count
		sint32 _Ref;
	public:

		///Construct object with null reference.
		IPointerGestion():_Ref(1)
		{
		}

		///Construct object with null reference. because copy constructor don't conserve refernce.
		IPointerGestion(const IPointerGestion &):_Ref(1)
		{
		}
		
		///Inc a reference. We use this when we have to conserve a pointer memeory for exemple in a list.
		void incRef()
		{
			_Ref ++;
		}
		
		///Release allow to release the pointer. The last release when the _Ref is nul delete the object.
		virtual void release()
		{
			if(decRef() == 0) 
					delete this;
		}
		
		///Decrement the reference of an object.
		sint32 decRef()
		{
			return --_Ref;
		}

		///get the refence count.
		const sint32 &getRef() const
		{
			return _Ref;
		}

		virtual void getDebugString(std::string &t) const
		{
			t += "None";
		}

		virtual ~IPointerGestion()
		{						
		}

		
		/// \name NLMISC::IStreamable method.
		//@{
		virtual std::string	getClassName()
		{
			return std::string("<unnamed>");
		}
		virtual void serial(NLMISC::IStream	&) throw(NLMISC::EStream)
		{
		}
		//@}


	};
	
	class CIdentTypeAlloc;
	class IClassFactory;
	class IBasicInterface;
	class CTypeOfObject;
	class CTypeOfOperator;

	/**
	CIdentType define a type for objects and manage the allocation of class in the class factory. 
	Not that class is manage in a std::map object and this class allow us to manage objects in the std::map.

	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000
	*/
	class CIdentType: public IPointerGestion
	{	
	friend class CIdentTypeAlloc;
	private:
		///This is a string for naming the class.
		char *_Ident;		
		///This objects allow use to know what kind of objects it is (simple object, agent, scripted agent, operator ...).
		CTypeOfObject *_ObjType;
		///This objects allow use to know what kind of operation we can do with it (add, sub ...).
		CTypeOfOperator *_OpSupport;
		///The index of objects in the class gactory map.
		sint32 _Index;

	public:
		///Basic shared type need for lot of objects, and special for typing the retern method in the script.
		static CIdentType VoidType;

	private:

		///Construct object with all atribut. It use when we serial object.
		CIdentType(const char *ident,const CTypeOfObject &objType,const CTypeOfOperator &opSupport,sint32 index);

	public:
		///Construct object with a stream.
		CIdentType(NLMISC::IStream &);/// throw(NLMISC::EStream);
		/**Construct object with a name. Object must be mounted in the class factory else an exception wil occur. 
		CExceptionContainer share an CExceptionUnRegisterClassError exception object.
		*/
		CIdentType(const char *ident);/// throw (NLAIE::CExceptionContainer);
		/**
		Construct object. Object must be mounted in the class factory else an exception wil occur. 
		CExceptionContainer share an CExceptionUnRegisterClassError exception object.
		*/
		CIdentType(const char *ident,const IClassFactory &classCFactory,
									const CTypeOfObject &objType,
									const CTypeOfOperator &opSupport);/// throw (NLAIE::CExceptionContainer);
		///Copy contructor
		CIdentType(const CIdentType &i);
		~CIdentType();

		///Get class type name.
		operator const char * () const
		{
#ifdef NL_DEBUG
			if(this == NULL) return "<Inknown>";
#endif
			return _Ident;
		}
						
	
		///Make equality.
		void operator = (const CIdentType &a);

		///Test the equality.
		bool operator == (const CIdentType &a) const
		{			
			return _Index == a._Index;			
		}
		///This test is need for the std::map.
		bool operator < (const CIdentType &a) const
		{			
			return strcmp(_Ident,a._Ident) < 0;
		}						

		bool operator <(const char *a) const
		{
			return strcmp(_Ident,a) < 0;
		}

		bool operator >(const CIdentType &a) const
		{
			return strcmp(_Ident,a._Ident) > 0;
		}						

		sint32 cmp (const CIdentType & a) const
		{
			return strcmp(_Ident,a._Ident);
		}

		operator const CTypeOfObject &() const
		{
			return *_ObjType;
		}
		
		operator const CTypeOfOperator &() const
		{
			return *_OpSupport;
		}

		sint32 getIndex() const
		{
			return _Index;
		}		

		///Alloc an instance of a class.
		const IBasicInterface *allocClass() const;
		///Get the class factory associate.
		const IClassFactory *getFactory() const;
				
		/// \name NLMISC::IStreamable method.
		//@{
		virtual std::string	getClassName()
		{
			return std::string(_Ident);
		}
		virtual void serial(NLMISC::IStream	&f) throw(NLMISC::EStream);	
		//@}


		void addObjectType(sint32);
	};


	/**
	IBasicType is an abstract allows some basic fonctionality same as typing, clone and get a new instance.

	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000
	*/
	class IBasicType: public IPointerGestion
	{
	public:
		IBasicType()
		{
		}

		IBasicType(const IBasicType &a):IPointerGestion(a)
		{
		}
		/**
		getType return a unique string how represente the class, it can be the name of the class. This function is used for the sytem regstry class (see the definition of the template class Gen::CRegistry).
		*/
		virtual const CIdentType &getType() const = 0;

		/**
		This function allow a pointer copy, that mean that the new class have the sam attributs caracteristics as the owne.
		*/
		virtual const IBasicType *clone() const = 0;

		/**
		This function allow a new instance, that mean that the class is a class factory.
		*/
		virtual const IBasicType *newInstance() const = 0;		

		/**
		This is a Debug function, text is an character pointer to receive the debug text output, the debug text containe all think sensible to interset user. 
		*/		
		virtual void getDebugString(std::string &) const = 0;

		virtual const std::string getInfo()
		{
			std::string s;
			getDebugString(s);
			return s;
		}

		virtual ~IBasicType()
		{
		}
	};

	/**
	IBasicInterface is an abstract class need to give sens of objects manipulation,
	we separate it from the IBasicType because the we need a basic class for object and class factory and methods define in this class can't be use in the class factory.
	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000
	*/
	
	class IBasicInterface: public IBasicType
	{
	
	public:
		IBasicInterface()
		{
		}

		~IBasicInterface()
		{
		}

		IBasicInterface(const IBasicInterface &a): IBasicType(a)
		{
		}

		virtual void serial(NLMISC::IStream	&f) throw(NLMISC::EStream)
		{
			if ( f.isReading() )
				load( f );
			else
				save( f );
		}

		/**
		Save the class in a stream.
		*/
		virtual void save(NLMISC::IStream &) = 0;
		/**
		Load the class from a stream.
		*/		
		virtual void load(NLMISC::IStream &) = 0;			
	};
		
	inline void IBasicInterface::save(NLMISC::IStream &)
	{
	} 

	inline void IBasicInterface::load(NLMISC::IStream &)
	{
	}
		

	/**
	IClassFactory is an abstract hows define an class factory,
	
	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000
	*/
	class IClassFactory: public IBasicType
	{

	public:
		/**		
		This function allow a new instance, that mean that the class is a class factory.
		*/
		virtual const IBasicInterface *createInstance() const = 0;

		/**
		This method allow user to get the builder class where a builder is the class hows make new instance.
		*/
		virtual const IBasicInterface *getClass() const = 0;

		///Set the mirror class to build.
		virtual void setClass(const IBasicInterface &) = 0;

	};

	/**
	IClassFactory is an basic class factory. The class builder is an IBasicInterface and we construct new class by calling newInstance method class.	
	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000
	*/
	class CSelfClassFactory : public IClassFactory
	{
	private:
		///Class buider.
		IBasicInterface *_Inst;
	public:
		///Construct class with a builder class.
		CSelfClassFactory(const IBasicInterface &a): _Inst((IBasicInterface *)a.newInstance())
		{
			
		}

		///Create a new instance from the class builder.
		const IBasicInterface *createInstance() const 
		{
			return (const IBasicInterface *)_Inst->newInstance();
		}
		
		///Clone the class factory.
		const IBasicType *clone() const
		{
			 IBasicType *x = new CSelfClassFactory(*_Inst);
			 return x;
		}
		
		///get a new instance of the class factory.
		const IBasicType *newInstance() const
		{
			return clone();
		}		
		
		///Get debufg string.
		void getDebugString(std::string &text) const
		{			
			//sprintf(text,"CSelfClassFactory sur l'interface %s",(const char *)_Inst->getType());
			text = stringGetBuild("CSelfClassFactory sur l'interface %s",(const char *)_Inst->getType());
		}		

		///Get the type of the class, the type is the builder type.
		const CIdentType &getType() const
		{
			return _Inst->getType();
		}		

		///get the builder class.
		virtual const IBasicInterface *getClass() const
		{
			return _Inst;
		}			

		virtual void setClass(const IBasicInterface &inst)
		{
			_Inst->release();
			_Inst = (IBasicInterface *)inst.clone();			
		}

			
		~CSelfClassFactory()
		{
			_Inst->release();
		}
	};
	

	/**
	IIO is an basic class hows define the in put out put. Because IO depend of application we encapsulate sate in a classwhos user have to define it.
	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000
	*/
	class IIO: public IBasicInterface
	{

	public:
		IIO()
		{
		}
		virtual void Echo(char *CStringType, ...) const = 0;
		virtual const std::string InPut() const = 0;
		virtual ~IIO()
		{
		}
	};	

	void Out(const char *, ...);
	void setDefaultIIO(const IIO *);
	const IIO *getDefaultIIO();

	
	/**
	CIdentTypeAlloc is an allocator of IBasicInterface from stream class.
	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000
	*/
	class CIdentTypeAlloc
	{	
	private:		
		CIdentType *_Id;

	public:
		///Construct object with a stream.
		CIdentTypeAlloc(NLMISC::IStream &is):_Id(NULL){load(is);}
		CIdentTypeAlloc():_Id(NULL){}

		///Load object from stream.
		void load(NLMISC::IStream &is);

		///Get the object.
		const IBasicInterface *allocClass()
		{
			if(_Id != NULL) return _Id->allocClass();
			else return NULL;
		}		

		virtual ~CIdentTypeAlloc()
		{
			if(_Id != NULL) delete _Id;
		}

		///user can serial this object.
		virtual void serial(NLMISC::IStream	&f) throw(NLMISC::EStream);		
	};
	

	/// \name Basic stream method utility.
	//@{
	///Read an CIdentTypeAlloc from a stream.
	inline NLMISC::IStream &operator >> (NLMISC::IStream &is, CIdentTypeAlloc &o)
	{
		o.load(is);
		return is;
	}

	///Read an CIdentType from a stream.
	NLMISC::IStream &operator << (NLMISC::IStream &os, CIdentType &o);	

	///Write an IBasicInterface to a stream.
	inline NLMISC::IStream &operator << (NLMISC::IStream &os, IBasicInterface &o)
	{
		o.save(os);
		return os;
	}

	///read an IBasicInterface from a stream.
	inline NLMISC::IStream &operator >> (NLMISC::IStream &is, IBasicInterface &o)
	{
		o.load(is);
		return is;
	}	
	//@}
}
#endif
