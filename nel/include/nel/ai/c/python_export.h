/** \file registry.h
 * Includes class factory object for register class.
 *
 * $Id: python_export.h,v 1.2 2001/01/08 10:47:05 chafik Exp $
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
#ifndef _PY_EXPORT_H
#define _PY_EXPORT_H

#pragma warning (disable: 4666)
#include "pyserver/pylib.h"
#include "c/abstract_interface.h"

namespace NLAIC
{
	enum PyMemDeleteType {unDeleteMemory,deleteMemory};

	struct PyRyzomeDict
	{
		const char *Name;
		PyObject *Obj;
		PyRyzomeDict(const char *varName,PyObject *object):	Name (varName),Obj(object)
		{
		}
		PyRyzomeDict(const PyRyzomeDict &dict):	Name (dict.Name),Obj(dict.Obj)
		{
		}
	};

	template <class T>
	class ryzPyObjetDef
	{
	public:
		PyObject_HEAD	
		T  *Instance;
		//PyMemDeleteType memType;


	public:		
		ryzPyObjetDef(T *a)
		{
			Instance = a;
			Instance->incRef();
		}
		operator T *() const
		{
			return Instance;
		}

		~ryzPyObjetDef()
		{
			Instance->release();
		}
	};

	class PyCharStr: public IPointerGestion
	{
	private:
		char *_Str;
		sint32 _Size;
	public:
		PyCharStr(sint32 size):_Str(new char [size]),_Size(size)
		{
			memset(_Str,0,_Size);
		}

		operator char *() const
		{
			return _Str;
		}

		sint32 getSize() const
		{
			return _Size;
		}

		/*virtual void release()
		{
			if(decRef() == 0) delete this;
		}*/

		virtual ~PyCharStr()
		{
			delete []_Str;
		}
	};

	class PyOstream: public IPointerGestion
	{			
	public:
		PyOstream()
		{
		}
		
		virtual operator std::ostream &() const = 0;
		virtual void getDebug(char *) const = 0;
		virtual void save(char *typeName,void *type)
		{
			std::ostream &os = (std::ostream &)*this;
			switch(typeName[0])
			{
			case 'f':
				os << *((float *)type);
				break;

			case 'i':
				os << *((sint32 *)type);
				break;

			case 'c':
				os << *((char *)type);
				break;

			case 'b':
				os << *((bool *)type);
				break;

			case 's':
				os << *((char *)type);
				break;			
				
			}
		}

		/*virtual void release()
		{
			if(decRef() == 0) delete this;
		}*/

		virtual ~PyOstream()
		{
		}

	};
	class PyFileOstream:public PyOstream
	{
	private:		
		std::ostream &_Stream;
	public:

		PyFileOstream(const char *nameFile);		
		
		virtual operator std::ostream &() const
		{
			return _Stream;
		}
		virtual void getDebug(char *txt) const
		{
			sprintf(txt,"class PyMemOstream inst,ace at <%4x>",this);
		}

		/*virtual void release()
		{
			if(decRef() == 0) delete this;
		}*/

		virtual ~PyFileOstream()
		{
			delete &_Stream;
		}

	};
	class PyMemOstream:public PyOstream
	{
	private:
		std::ostream &_Stream;
	public:
		PyMemOstream(char *buffer,sint32 size);

		operator const char *() const;

		virtual operator std::ostream &() const
		{
			return _Stream;
		}

		virtual void getDebug(char *txt) const
		{
			sprintf(txt,"class PyMemOstream inst,ace at <%4x>",this);
		}

		/*virtual void release()
		{
			if(decRef() == 0) delete this;
		}*/

		virtual ~PyMemOstream()
		{
			delete &_Stream;
		}
		

	};

	class PyTypeOstream:public PyOstream
	{
	private:
		std::ostream &_Stream;
	public:
		PyTypeOstream(std::ostream &stream): _Stream(stream)
		{
		}

		virtual operator std::ostream &() const
		{
			return _Stream;
		}

		virtual void getDebug(char *txt) const
		{
			sprintf(txt,"class PyTypeOstream inst,ace at <%4x>",this);
		}

		/*virtual void release()
		{
			if(decRef() == 0) delete this;
		}*/

		virtual ~PyTypeOstream()
		{			
		}
		

	};

	class PyIstream: public IPointerGestion
	{
	private:		
	public:
		PyIstream()
		{
		}
		virtual operator NLMISC::IStream &() const = 0;
		virtual void getDebug(char *) const = 0;

		virtual void load(char *typeName,void *type)
		{
			NLMISC::IStream &is = (NLMISC::IStream &)*this;
			switch(typeName[0])
			{
			case 'f':
			case 'F':
				is.serial( *((float *)type) );
				break;

			case 'i':
			case 'I':
				is.serial( *((sint32 *)type) );
				break;

			case 'c':
			case 'C':
				is.serial( *((char *)type) );
				break;

			case 'b':
			case 'B':
				is.serial( *((char *)type) );
				break;

			case 's':
			case 'S':
				is.serial( *((char *)type) );
				break;			
				
			}
		}

		/*virtual void release()
		{
			if(decRef() == 0) delete this;
		}*/

		virtual ~PyIstream()
		{
		}

	};

	class PyFileIstream:public PyIstream
	{
	private:		
		NLMISC::IStream &_Stream;
	public:

		PyFileIstream(const char *nameFile);		
		
		virtual operator NLMISC::IStream &() const
		{
			return _Stream;
		}
		virtual void getDebug(char *txt) const
		{
			sprintf(txt,"class PyMemOstream inst,ace at <%4x>",this);
		}
		/*virtual void release()
		{
			if(decRef() == 0) delete this;
		}*/

		virtual ~PyFileIstream()
		{
			delete &_Stream;
		}

	};

	class PyMemIstream:public PyIstream
	{
	private:
		NLMISC::IStream &_Stream;
	public:
		PyMemIstream(const char *buffer);
		operator const char *() const;

		virtual operator NLMISC::IStream &() const
		{
			return _Stream;
		}

		virtual void getDebug(char *txt) const
		{
			sprintf(txt,"class PyMemIstream inst,ace at <%4x>",this);
		}
		/*virtual void release()
		{
			if(decRef() == 0) delete this;
		}*/
		virtual ~PyMemIstream()
		{		
			delete &_Stream;
		}

	};

	class PyTypeIstream:public PyIstream
	{
	private:
		NLMISC::IStream &_Stream;
	public:
		PyTypeIstream(NLMISC::IStream &stream): _Stream(stream)
		{
		}

		virtual operator NLMISC::IStream &() const
		{
			return _Stream;
		}

		virtual void getDebug(char *txt) const
		{
			sprintf(txt,"class PyTypeIstream inst,ace at <%4x>",this);
		}
		/*virtual void release()
		{
			if(decRef() == 0) delete this;
		}*/
		virtual ~PyTypeIstream()
		{			
		}
		

	};

	class CIdentType;
	class IBasicInterface;

	typedef ryzPyObjetDef<CIdentType> tPyIdentType;
	typedef ryzPyObjetDef<IBasicInterface> tPyiBasicInterface;
	typedef ryzPyObjetDef<PyOstream> tPyOstream;
	typedef ryzPyObjetDef<PyIstream> tPyIstream;
	typedef ryzPyObjetDef<PyCharStr> tPyCharStr;
	

	Py::PyExport *getPyServer();
	void setPyServer(Py::PyExport *);
	void addMethode(PyMethodDef *methodeDef);
	inline void addMethode(char *name,PyCFunction	func,sint32 flag = 1,char *doc = NULL)
	{
		PyMethodDef m = {name, (PyCastMethod)func, flag, doc};
		addMethode(&m);
	}
	void removeMethode(const char *methodeName);
	void addToDict(const PyRyzomeDict &dict);
	void removeFromDict(const char *name);
	void initPyExport();
	void releasePyExport();

	template<class T>
	ryzPyObjetDef<T> *CreateCyInstance(T *O,PyTypeObject *defType)
	{
		ryzPyObjetDef<T> *ThisClass;
		ThisClass = new ryzPyObjetDef<T>(O);
		if(ThisClass == NULL) return NULL;
		ThisClass->instance = O;		
		ThisClass->ob_type = defType;
		_Py_NewReference((PyObject *)ThisClass);
		return ThisClass;
	}
}

#endif