/** \file pythonmethode.cpp
 *
 * $Id: pythonmethode.cpp,v 1.19 2002/08/21 13:58:34 lecroart Exp $
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
#include "nel/ai/pyserver/pylib.h"
#include "object.h"

extern PyTypeObject StackDef;
extern PyMethodDef M_STACK[];

/*
from RysonSytemeIO import *
a = u'samos \u1234'
UnicodeString(a)
*/

namespace NLAIPYSERVER
{

	/*PyObject *PyUnicode_FromObject(register PyObject *obj)
	{
		const char *s;
		sint32 len;
    
		if (obj == NULL) {
		PyErr_BadInternalCall();
		return NULL;
		}
		else if (PyUnicode_Check(obj)) {
		Py_INCREF(obj);
		return obj;
		}
		else if (PyString_Check(obj)) {
		s = PyString_AS_STRING(obj);
		len = PyString_GET_SIZE(obj);
		}
		else if (PyObject_AsCharBuffer(obj, &s, &len)) {
		
		if (PyErr_ExceptionMatches(PyExc_TypeError))
			PyErr_SetString(PyExc_TypeError,
					"coercing to Unicode: need string or charbuffer");
		return NULL;
		}
		if (len == 0) {
		Py_INCREF(unicode_empty);
		return (PyObject *)unicode_empty;
		}
		return PyUnicode_DecodeUTF8(s, len, "strict");
	}*/


	static NLAIC::IIO *Interface;
	static PyObject *print(PyObject *Self,PyObject *Arg)
	{
		char *Text = NULL;
		char T[2] = {'\n',0};
		/*Py_UNICODE u[200];
		PyObject *argi;
		argi = PyTuple_GetItem (Arg, 0);		
		PyUnicodeObject *self = (PyUnicodeObject *)PyUnicode_FromObject(argi);

		if (self != NULL)
		{
			sint32 i = 0;
			char str[200];
			while(self->str[i])
			{
				u[i] = self->str[i];
				str[i] = (char)self->str[i];
				i ++;
			}
			u[i] = 0;
			str[i] = 0;
		}

		if(PyArg_ParseTuple(Arg,"u",&u))
		{
			//u = (short *)Text;
		}		
		else*/
		if(!PyArg_ParseTuple(Arg,"s",&Text))
		{
			Text = T;
		}

		Interface->Echo("%s",Text);
		PyObject *R = Py_BuildValue("i",true);
		return R;
	}

	static PyObject *printNbr(PyObject *Self,PyObject *Arg)
	{
		sint32 Text;

		if(!PyArg_ParseTuple(Arg,"i",&Text)) return NULL;
		NLAIC::Out("%0x",Text);
		return Py_BuildValue("i",true);
	}

	static PyObject *read(PyObject *Self,PyObject *Arg)
	{		
		return Py_BuildValue("s",Interface->InPut().data());
	}

/*	static PyObject *getUnicodeString(PyObject *Self,PyObject *Arg)
	{
		Py_UNICODE u[200];
		PyObject *argi;
		argi = PyTuple_GetItem (Arg, 0);		
		PyUnicodeObject *self = (PyUnicodeObject *)PyUnicode_FromObject(argi);
		if (self != NULL)
		{
			sint32 i = 0;
			char str[200];
			while(self->str[i])
			{
				u[i] = self->str[i];
				str[i] = (char)self->str[i];
				i ++;
			}
			u[i] = 0;
			str[i] = 0;
		}


		return Py_BuildValue("i",true);
	}
*/





	static struct PyMethodDef M_Constructeur[] =
	{
		{"Echo", print ,METH_VARARGS},
		{"Nbr", printNbr ,METH_VARARGS},
		{"Read", read ,METH_VARARGS},
		//{"UnicodeString",getUnicodeString,METH_VARARGS},
		{NULL,NULL}
	};



	PyMethodDef *getLib()
	{
		return M_Constructeur;
	}
	
	static PyObject *Dictionarry = NULL;

	CPyExport *initPythonLib(NLAIC::IIO *P,char *pathWay)
	{
		//char S[MAX_PATH + 1];
		//HKEY	hKey;

		Interface = P;		

		Py_Initialize();

#ifdef NL_DEBUG
		char *k = (char *)Py_GetPlatform();
		k = (char *)Py_GetVersion();
		k = (char *)Py_GetProgramFullPath();
		k = (char *)Py_GetPath();
		k = (char *)Py_GetProgramName();
		k = (char *)Py_GetPrefix();
		k = (char *)Py_GetExecPrefix();
#endif	

		std::string PathPython = Py_GetPath();
		//PathPython = new char[ strlen(Py_GetPath() ) + strlen(pathWay) + 3];
		//memset(PathPython,0,strlen(Py_GetPath() ) + strlen(pathWay) + 3);		

		/*strcpy(S,PathPython);
		strcat(S,"/PyLib");*/

		//const char delem[2] = {DELIM,0};	

		CPyExport *m_pPyLib = new CPyExport;
				
		PathPython += CPyExport::getPathSeparator();
/*#ifdef NL_OS_WINDOWS
		//sprintf(PathPython,"%s;%s",Py_GetPath(),pathWay);
		PathPython += ";";
#else
		//sprintf(PathPython,"%s:%s",Py_GetPath(),pathWay);
		PathPython += ":";
#endif*/
		PathPython += pathWay;						

		PySys_SetPath((char *)PathPython.c_str());

#ifdef NL_DEBUG
		NLAIC::Out("%s\n",(char *)Py_GetPath());
		k = (char *)Py_GetPath();
#endif 
		
		Dictionarry = Py_InitModule("RysonSytemeIO",M_Constructeur);

		char CodeExec[] =	"import sys\n"
							"from RysonSytemeIO import *\n"
							"class PyWindowInOut:\n"
							"\tdef write(self, s, tags=(), mark='insert'):\n"
 							"\t\tEcho(s)\n\n"
							"\tdef writelines(self, l):\n"
							"\t\tmap(self.write, l)\n\n"
							"\tdef read(self, length = None):\n"
 							"\t\treturn Read()\n\n"
							"\tdef readline(self, length = None):\n"
							"\t\treturn Read()\n\n"
							"\tdef readlines(self, length = None):\n"
							"\t\treturn Read()\n\n\n"
							"sys.stdout = PyWindowInOut()\n"
							"sys.stderr = PyWindowInOut()\n"
							"sys.stdin = PyWindowInOut()\n"
							"sys.argv = ' '\n"
							"sys.argc = 1\n";
							
		m_pPyLib->runCode(CodeExec);

		char CodeExec2[] =	"import sys\n"
							"print 'Hello this is a window trace for python input/output statement ............................................................................'\n";

		
		m_pPyLib->runCode(CodeExec2);

		
		return m_pPyLib;
	}

	void endPythonInterface(CPyExport *Int) 
	{
		Py_DECREF(Dictionarry);
		delete Int;
		Interface->release();

	}
}
