#ifndef GAME_PYTHON_LIB_H
#define GAME_PYTHON_LIB_H
/** \file pylib.h
 * class to run and manage python script
 *
 * $Id: pylib.h,v 1.10 2003/01/20 10:56:41 chafik Exp $
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

#include <Python.h>
#include "nel/ai/pyserver/pythonsupport.h"
#include "nel/ai/c/abstract_interface.h"


namespace NLAIPYSERVER
{		

#define PyCastMethod	struct _object *(__cdecl *)(struct _object *,struct _object *)

	class CPyExport
	{
	private:
		bool _DebugMode;
		
		const char *getNameSpace(const char *Name) const
		{
			return Name == NULL ? "__main__" : Name;
		}
				
		PyObject *loadAttruibut(char  *,char  *);
		PyObject *loadModule(char  *);
		sint32	convertResult(PyObject *Result,char *Format, void *Target);
		PyObject *debugFunction(PyObject *Func,PyObject *Arg);
		PyObject *debugCode(sint32 RunMode,const char *code,PyObject *ModDict);
		void printLastError();

	public:	

		CPyExport()
		{
			_DebugMode = false;
			//Py_Initialize();		
			_DebugMode = false;			
		}

		///Put the debug mode on
		bool	setDebug()
		{
			return (_DebugMode = true); 
		}

		///return the debug mode status
		sint32	getDebugStat()
		{
			return _DebugMode;
		}

		///Put the debug mode off
		bool	unSetDebug()
		{
			return (_DebugMode = false);
		}


		///test debug mode is on
		bool isDebug()
		{
			return (_DebugMode == true);
		}

		/**				
			Run Python script on debug/release mode depend on _DebugMode status.

			\param Code:		Python script on text mode, warrning, there are difference between UNIX & Window, on UNIX system 
								there are a \n\r to mark the end of line, and Python parser don't like the \r syntax, use the methode GetCodeFromFile
								to clear all \r.								

			\param NameSpace:	Name space od code to execute. NULL is the default Python name space that is __main__.

			\param ModeExec:	execution type file_input or eval_input.

			\param Format:		Argument format, for exempel that can bee: "O" (as Object), "i" (intger) ........ see python manual.
								
			\param ResultTarget:	The return of the result on eval_input mode.
\code
Sample:
	pExePythonExpression->RunCode(	"class C():\n"
					"\t def __init__(self):\n"
					"\t\t self.x = 5\n"
					"\t\t self.y = 25\n"
					"\t def M(self, nX, nY):\n"
					"\t\t return self.x*nX,self.y*nY\n");
	PyObject *PyObjetA;
	pExePythonExpression->RunCode("A = C()",NULL,eval_input,"O",&PyObjetA);
\endcode								

		  the method return	0 if Ok; -1  if fail.

		*/
		sint32 runCode(const char  *Code,char  *NameSpace = NULL,sint32 ModeExec = Py_file_input,char *Format = NULL,void *ResultTarget = NULL,sint32 Error = true );	
	
	/**
		Run a python method on a python class.		

		\param Obj:			Pointer of the module that contain method, see exemple on runCode or python manual. 

		\param MethodName:	Name of metod to run.

		\param ResFormat:	Type of expected result.

		\param ResTarget:		The return of the result.
							
		\param Format:		Argument format, for exempel that can bee: "O" (as Object), "i" (intger) ........ see python manual.
\code
Sample:
	PyObject *PyObjetA,*PyObjetResult;
	pExePythonExpression->RunCode("A = C()",NULL,eval_input,"O",&PyObjetA);
	pExePythonExpression->RunMethod(PyObjetA,"M","O",PyObjetResult,"ii",5,16);
\endcode

		The result is a Python tupule as (25,400);		
		The method return	0 if Ok; -1  if fail.
	*/		

		sint32 runMethod(PyObject *Obj,char  *MethodName,char *ResFormat, void *ResTarget,char *Format,...);
		
	/**
		Run a python function on an module.				

		\param ModuleName:		name of the module.

		\param MethodName:		name of method to run.

		\param ResFormat:	Type of expected result.

		\param ResTarget:		The return of the result.
							
		\param Format:		Argument format, for exempel that can bee: "O" (as Object), "i" (intger) ........ see python manual.

		The method return	0 if Ok; -1  if fail.
	*/
		sint32 runFunction(char *ModuleName,char  *MethodName,char *ResFormat, void *ResTarget,char *Format,...);

	
	/**	
		get valu of an attribut in an python object.

		\param Obj:		python object.

		\param AttribName: name of attribute to eval.

		\param ResFormat:	Type of expected result.

		\param ResTarget:		The return of the result.
							
\code
Sample:

	sint32 PyTheXOfA;
	sint32 PyTheYOfA;
	pExePythonExpression->GetMembreValue(PyObjetA,"X","i",&PyTheXOfA);
	pExePythonExpression->GetMembreValue(PyObjetA,"Y","i",&PyTheYOfA);
\endcode

		The method return	0 if Ok; -1  if fail.
	*/
		sint32	getMembreValue(PyObject *Obj,char  *AttribName,char *ResFormat, void *ResTarget);	
		
		
	/**
		Change attribut value of an Python object.

		\param Obj:		python object.

		\param AttribName: name of attribute to eval.

		\param ResTarget: The return of the result.

\code			
exemple:							
	pExePythonExpression->SetMembreValue(PyObjetA,"X","i",20);
	pExePythonExpression->SetMembreValue(PyObjetA,"Y","i",2);
	pExePythonExpression->RunMethod(PyObjetA,"M","O",PyObjetResult,"ii",5,16);
\endcode

		The method return	0 if Ok; -1  if fail.
	*/
		sint32	setMembreValue(PyObject *Obj,char  *AttribName,char *ResFormat, ...);	
		

		/// permet d'avoir un poiteur sur le contenu d'un fichier text, le pointeur est à detruir par l'utulisateur.
		char  *getCodeFromFile(const char *FileName);

		
		sint32 getGlobal(char  *NameSpace,char  *VarName,char *Format,void *VarValue); // la meme chose que GetMembreValue mais sur un module
		sint32 setGlobal(char  *NameSpace,char  *VarName,char  *Format, ...); // la meme chose que SetMembreValue mais sur un module
		PyObject *addModule(char  *,PyMethodDef *);
		PyObject *getRefVar(char  *VarName);		
		
		~CPyExport()
		{

		}

		public:
			static const char *getPathSeparator();
	};


	CPyExport *initPythonLib(NLAIC::IIO *Interface,char *pathWay);
	void endPythonInterface(CPyExport *Interface);
}
#endif
