/** \file pylib.cpp
 *
 * $Id: pylib.cpp,v 1.10 2003/01/07 17:46:20 miller Exp $
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
#include <stdarg.h>
#include <iostream.h>
#include <string.h>
#include "nel/ai/pyserver/pylib.h"
#include "osdefs.h"

namespace NLAIPYSERVER
{		
	PyObject *CPyExport::loadModule(char *name)
	{
		char *Name = (char *)getNameSpace(name);

		PyObject *Module;

		Module = PyDict_GetItemString(PyImport_GetModuleDict(),Name);


		if(strcmp(Name,"__main__") == 0) return PyImport_AddModule(Name);
		else	
		if(Module != NULL && PyDict_GetItemString(Module,"__dummy__")) return Module;	
		else
		{	
			Module = PyImport_ImportModule(Name);
			return Module;
		}
	}

	PyObject *CPyExport::loadAttruibut(char *ModeName,char *Attribut)
	{
		PyObject *Module;

		ModeName = (char *)getNameSpace((const char *)ModeName);
		Module = loadModule(ModeName);
		if(Module == NULL) return NULL;
		return PyObject_GetAttrString(Module,Attribut);
	}

	sint32 CPyExport::convertResult(PyObject *Result,char *Format, void *Target)
	{
		if(Result)
		{
			if(!Target)//nodefaultlib:libcpmtd.lib
			{
				Py_DECREF(Result);
				return 0;
			}
			if(!PyArg_Parse(Result,Format,Target))
			{
				Py_DECREF(Result);
				return -1;
			}
			/*if(!strcmp(Format,"O"))
			{
				Py_DECREF(Result);
				return 0;
			}*/
			return 0;
		}

		return -1;
	}

	PyObject *CPyExport::debugFunction(PyObject *Func,PyObject *Arg)
	{
		/*sint32 oops,res;
		PyObject *Result = NULL;

		oops = _PyTuple_Resize(&Arg,(1 + PyTuple_Size(Arg)));
		oops |= PyTuple_SetItem(Arg,0,Func);

		if(oops) return NULL;

		res = runFunction("pdb","runcall","O",&Result,"O",Arg);*/

		return NULL;//(res != 0) ? NULL : Result;
	}

	PyObject *CPyExport::debugCode(sint32 RunMode,const char *code,PyObject *ModDict)
	{
		sint32 Stat;
		PyObject *Result;
		const char *pdbName = (RunMode == Py_eval_input ? "runeval":"run");
		

		//UnSetDebug();
		Stat = runFunction("pdb",(char *)pdbName,"O",&Result,"(sOO)",code,ModDict,ModDict);

		//SetDebug();

		return (Stat != 0) ? NULL : Result;
	}

	sint32 CPyExport::runFunction(char *ModuleName,char *MethodName,char *ResFormat, void *ResTarget,char *Format,...)
	{	

		PyObject *Func,*Arg,*Result;
		va_list ArgList;
		va_start(ArgList,Format);
		
		if((Func = loadAttruibut(ModuleName,MethodName)) == NULL ) return -1;

		if((Arg = Py_VaBuildValue(Format,ArgList)) == NULL)
		{
			Py_DECREF(Func);
			return -1;
		}

		if(isDebug() && strcmp(ModuleName,"pdb"))
		{
			Result = debugFunction(Func,Arg);
		}
		else
		{
			Result = PyEval_CallObject(Func,Arg);
			if(!Result) 
						PyErr_Print ();
		}
		Py_DECREF(Func);
		Py_DECREF(Arg);
		return convertResult(Result,ResFormat,ResTarget); 
	}

	sint32 CPyExport::runCode(const char *Code,char *NameSpace,sint32 ModeExec,char *Format, void *Target,sint32 Error)
	{
		PyObject *Module,*Dict,*Result;
		
		Module = loadModule(NameSpace);
		if(Module == NULL) return -1;

		Dict = PyModule_GetDict(Module);

		if(Dict == NULL) return -1;

		if(isDebug())
		{
			Result = debugCode(ModeExec,Code,Dict);			
		}
		else Result = PyRun_String((char *)Code,ModeExec,Dict,Dict);
		
		if(Result == NULL)
		{		
			/*if(Error) PrintLastError();		
			else PyErr_Print ();*/
			PyErr_Print ();
			return -1;
		}
		else 
		{
			if(ModeExec == Py_file_input)
			{
				Py_XDECREF(Result);
				return 0;
			}
			else 
			return convertResult(Result,Format,Target);
		}

	}

	sint32 CPyExport::runMethod(PyObject *Obj,char *MethodName,char *ResFormat, void *ResTarget,char *Format,...)
	{	
		if(Obj)
		{
			/*PyObject *s = PyObject_Str(Obj);
			char *S = PyString_AsString(s);*/
			PyObject *Method,*Arg,*Result;
			va_list ArgList;
			va_start(ArgList,Format);

			if((Method = PyObject_GetAttrString(Obj,MethodName)) == NULL) 
			{
				PyErr_Print ();
				return -1;
			}

			if((Arg = Py_VaBuildValue(Format,ArgList)) == NULL) 
			{	
				PyErr_Print();
				Py_DECREF(Method);
				return -1;
			}

			if(isDebug())
			{
				Result = debugFunction(Method,Arg);			
			}
			else 
			{
				Result = PyEval_CallObject(Method,Arg);
				if(!Result) 
						PyErr_Print ();
			}
			
			Py_DECREF(Method);
			Py_DECREF(Arg);

			return convertResult(Result,ResFormat,ResTarget); 
		}
		return -1;
	}

	sint32	CPyExport::getMembreValue(PyObject *Obj,char *AttribName,char *ResFormat, void *ResTarget)
	{
		PyObject *Attrib;
		
		if((Attrib = PyObject_GetAttrString(Obj,AttribName)) == NULL) 
		{
			PyErr_Print ();
			return -1;
		}

		return convertResult(Attrib,ResFormat,ResTarget);  
	}

	sint32	CPyExport::setMembreValue(PyObject *Obj,char *AttribName,char *ResFormat, ...)
	{
		PyObject *Val;

		va_list ArgList;
		va_start(ArgList,ResFormat);
		if((Val = Py_VaBuildValue(ResFormat,ArgList)) == NULL) return -1;
		sint32 Res = PyObject_SetAttrString(Obj,AttribName,Val);
		Py_DECREF(Val);
		return Res;	
	}

	sint32 CPyExport::getGlobal(char *NameSpace,char *VarName,char *Format,void *VarValue)
	{	
		PyObject *Var;

		Var = loadAttruibut(NameSpace,VarName);
		return convertResult(Var,Format,VarValue);
	}


	sint32 CPyExport::setGlobal(char *NameSpace,char *VarName,char *Format, ...)
	{
		sint32 Result;
		PyObject *Module,*Var;

		va_list CVals;
		va_start(CVals,Format);

		if( (Module = loadModule(NameSpace)) == NULL) 
													return -1;

		Var = Py_VaBuildValue(Format,CVals);
		va_end(CVals);

		if(Var == NULL) 
				return -1;

		Result = PyObject_SetAttrString(Module,VarName,Var);
		Py_DECREF(Var);

		return Result;
	}

	PyObject *CPyExport::getRefVar(char *VarName)
	{
		PyObject *Objet;
		std::string text;
		text = NLAIC::stringGetBuild("%s",VarName);
		runCode(text.c_str(),NULL,Py_eval_input,"O",&Objet);
		if(Objet)
		{
			Py_INCREF(Objet);
			return Objet;
		}
		return NULL;
	}


	PyObject *CPyExport::addModule(char *ModuleName,PyMethodDef *DefCodeModule)
	{
		return Py_InitModule(ModuleName,DefCodeModule);
	}

	char *CPyExport::getCodeFromFile(const char *FileName)
	{
		FILE* f;
		char *buf;
		char* code;
		sint32 si;
		uint16 ui,uk;

		f = fopen(FileName,"rb");
		si=CFile::getFileSize (f);
//		fseek(f,0,SEEK_END);
//		si = ftell(f);
//		rewind(f);
		buf = new char [si + 1];
		fread(buf, sizeof( char ), si, f);
		fclose(f);
		buf[si] = 0;

		code = new char[strlen(buf)];
		for(ui=0,uk=0; ui < strlen(buf); ui++)
		{
			if(buf[ui] != '\r') 
			{
				code[uk] = buf[ui];
				uk ++;
			}
		}	
		code[uk] = 0;
		return code;
		delete buf;
	}

	const char *CPyExport::getPathSeparator()
	{
		static const char s[] = {DELIM,0};
		return s;
	}
}
