/** \file registry.h
 * Includes class factory object for register class.
 *
 * $Id: python_export.h,v 1.9 2003/01/31 14:59:43 chafik Exp $
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

#ifdef NL_OS_WINDOWS
#pragma warning (disable: 4666)
#endif

#include "nel/ai/pyserver/pylib.h"
#include "nel/ai/c/abstract_interface.h"

namespace NLAIC
{
	enum PyMemDeleteType {unDeleteMemory,deleteMemory};

	/**
	Template class to allow export of C++ object under Python script
	*/
	template <class T>
	class CPyTemplateDef
	{
	public:

		///Python header attribut.
		//{@		
		PyObject_HEAD
		//@}
		T  *Instance;

	public:		
		CPyTemplateDef(T *a)
		{
			Instance = a;
			Instance->incRef();
		}
		operator T *() const
		{
			return Instance;
		}

		~CPyTemplateDef()
		{
			Instance->release();
		}
	};	

	/**
	Template method allow to have a pointer to a class C++ under Python, this is nead to export C++ class to python class.
	*/
	template<class T>
	CPyTemplateDef<T> *CreatePyObjectInstance(T *o,PyTypeObject *defType)
	{
		CPyTemplateDef<T> *ThisClass;
		ThisClass = new CPyTemplateDef<T>(o);
		if(ThisClass == NULL) return NULL;		
		ThisClass->Instance = o;
		ThisClass->ob_type = defType;
		_Py_NewReference((PyObject *)ThisClass);
		return ThisClass;
	}
}

#endif
