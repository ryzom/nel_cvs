#ifndef _GAME_PYTHON_LIB_H_
#define _GAME_PYTHON_LIB_H_

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

	//!Met le Mode debug à ON
		bool	setDebug()
		{
			return (_DebugMode = true); 
		}

		sint32	getDebugStat()
		{
			return _DebugMode;
		}

	//!Met le Mode debug à OFF 
		bool	unSetDebug()
		{
			return (_DebugMode = false);
		}


	//!Check the debug mode state.
		bool isDebug()
		{
			return (_DebugMode == true);
		}

	/**	
		Execution d'instruction Python dans le mode (Debug/Release) on fonction de l'etat de _DebugMode

		Code:				est le texte contenant les instruction Python à executer. 
							Attention generalement les editeurs windows produit des fin 
							de ligne marquer par les caractéres \n\r, l'interpréteur Python 
							n'aime pas du tout le caractaire \r en fin de ligne donc 
							il faut filtrer le texte à executer avant l'appelle à cette procedure,
							c'est à dire convertire tout \r en \n. Ou utiliser la methode GetCodeFromFile.

		NameSpace:			Le name space du code à executer. NULL c'est le name space Python par
							defaut c'est à dire __main__.

		ModeExec:			Le type d'excecution. Ce type peut être par defaut execution normal (file_input),
							ou evalution d'expression (eval_input).

		Format:			Le type de parametre dans le cas ou l'execution se fait avec eval_input.
							ce type peut être	"O" (Objet);
												"i" (intger) ........ ; voir la doc Python.
							
		ResultTarget:	Pointeur sur un pointeur pour resevoir le resultat d'evaluation.
\code
Exemple:
	pExePythonExpression->RunCode(	"class C():\n"
					"\t def __init__(self):\n"
					"\t\t self.x = 5\n"
					"\t\t self.y = 25\n"
					"\t def M(self, nX, nY):\n"
					"\t\t return self.x*nX,self.y*nY\n");
	PyObject *PyObjetA;
	pExePythonExpression->RunCode("A = C()",NULL,eval_input,"O",&PyObjetA);
\endcode

							avec eval_input PyObjetA est le pointeur sur l'instance de l'Objet Python A crée dans Python

	  return:
			0	Ok;
			-1  ehec.

	*/
		sint32 runCode(const char  *Code,char  *NameSpace = NULL,sint32 ModeExec = Py_file_input,char *Format = NULL,void *ResultTarget = NULL,sint32 Error = true );	
	
	/**
		Execution d'une methode membre d'un objet Python dans le mode (Debug/Release) on fonction de l'etat de _DebugMode

		Obj:				pointeur sur une instance d'un Objet python (voir l'exemple plus haut). 

		MethodName:		Le nom de la methode membre à executer (je rappelle qu'en python tout est public).

		ResFormat:		Type du resultat attendu.

		ResTarget:		pointeur pour resevoir le resultat de l'appelle de la methode.
							
		Format:			Le type de l'argument par exemple: ResFormat "(iiO)" (2 sint32 et un pointeur sur un objet python). 						
							viennent ensuite se déposer sur la pile les valeurs de l'argument.
\code
exemple:
	PyObject *PyObjetA,*PyObjetResult;
	pExePythonExpression->RunCode("A = C()",NULL,eval_input,"O",&PyObjetA);
	pExePythonExpression->RunMethod(PyObjetA,"M","O",PyObjetResult,"ii",5,16);
\endcode

							Le resulta sera un tupule Python du type(25,400);

		return:
			0	Ok;
			-1  ehec.

		

	*/		

		sint32 runMethod(PyObject *Obj,char  *MethodName,char *ResFormat, void *ResTarget,char *Format,...);
	

	/**
		Execution d'une fonctio membre d'un module Python dans le mode (Debug/Release) on fonction de l'etat de _DebugMode

		ModuleName:		Nom du module contenant la fonction à executer.

		MethodName:		Le nom de la fonction à executer.

		ResFormat:		Type du resultat attendu.

		ResTarget:		pointeur pour resevoir le resultat de l'appelle de la fonction.
							
		Format:			Le type de l'argument par exemple: ResFormat "(iiO)" (2 sint32 et un pointeur sur un objet python). 						
							viennent ensuite se déposer sur la pile les valeurs de l'argument.
		return:
			0	Ok;
			-1  ehec.
	*/
		sint32 runFunction(char *ModuleName,char  *MethodName,char *ResFormat, void *ResTarget,char *Format,...);

	
	/**	
		Obtenir la valeur d'un attribut dans un objet Python.

		Obj:				pointeur sur une instance d'un Objet python (voir l'exemple plus haut).

		AttribName:		Le nom de l'attribut à obtenir.

		ResFormat:		Type du resultat attendu.		

		ResTarget:		pointeur pour resevoir le resultat de l'appelle de la fonction.
							
\code
exemple:

	sint32 PyLeXDeA;
	sint32 PyLeYDeA;
	pExePythonExpression->GetMembreValue(PyObjetA,"X","i",&PyLeXDeA);
	pExePythonExpression->GetMembreValue(PyObjetA,"Y","i",&PyLeYDeA);
\endcode

		return:
			0	Ok;
			-1  ehec.
	*/
		sint32	getMembreValue(PyObject *Obj,char  *AttribName,char *ResFormat, void *ResTarget);	
		
		
	/**
		Changer la valeur d'un attribut dans un objet Python.

		Obj:				pointeur sur une instance d'un Objet python (voir l'exemple plus haut).

		AttribName:		Le nom de l'attribut à obtenir.

		ResFormat:		Type du resultat attendu.
							vient ensuite la valeur de l'attribut.

\code			
exemple:							
	pExePythonExpression->SetMembreValue(PyObjetA,"X","i",20);
	pExePythonExpression->SetMembreValue(PyObjetA,"Y","i",2);
	pExePythonExpression->RunMethod(PyObjetA,"M","O",PyObjetResult,"ii",5,16);
\endcode

							Le resulta sera un tupule Python du type(100,30);
		return:
			0	Ok;
			-1  ehec.
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
