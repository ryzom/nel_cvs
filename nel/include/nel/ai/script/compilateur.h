/** \file compilateur.h
 * Includes all for compiling a script.
 *
 * $Id: compilateur.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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

#ifndef NL_COMPILATEUR_H
#define NL_COMPILATEUR_H

#include "agent/agentpack.h"
#include "agent/agent_string.h"
#include "agent/agent_script.h"

#include "script/interpret_object.h"
#include "script/interpret_methodes.h"
#include "script/interpret_object_list.h"

//#include "pyserver/pylib.h"
#include "script/lexsupport.h"
#include "script/varstack.h"
#include "script/code_branche_run_debug.h"

#include "script/codage.h"
#include "script/constraint_debug.h"
#include "script/module.h"
#include "script/lex.h"
#include "logic/logic.h"
#include "fuzzy/fuzzy.h"
#include <fstream.h>



namespace NLIASCRIPT
{
	class IConstraint;

	/**
	Type of variable manipulation in expression evaluation.
	*/
	enum TVarOfType {
		///It hapend when you have an expression as x = 5.
		varTypeImediate,
		///The variable is a member of a class.
		varTypeMember,
		///The variable is computed in the stack.
		varTypeStackMember,
		///The variable is computed in the heap.
		varTypeHeapMember,
		///The variable is local.
		varTypeLocal,
		///Function manipulation.
		varForFunc,
		varUnknown,
		varTypeUndef
	};

	/**
	This structur define the type of an factor. 
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000	
	*/
	struct CFactorType
	{
		///if VarType is an varTypeImediate then Value contain the value of variable.
		NLIAAGENT::IObjetOp	*Value;
		///if VarType is an varTypeLocal then ValueVar is the pointer representation of this variable.
		const NLIAAGENT::IObjectIA	*ValueVar;
		TVarOfType		VarType;
		///This is use for the yacc parser.
		bool IsUsed;
		///This list containe the range of a variable memeber.
		std::list<sint32>	Member;
		///Name of the hiearchie variable for exemple pos.y.
		std::list<NLIASCRIPT::CStringType > MemberName;
		///It use when we acces to a stack statement.
		IOpType			*TypeStack;		
		CFactorType();		

		CFactorType(const CFactorType &v);

		~CFactorType()
		{
			del();
		}

		void del();		

		void set(const CFactorType &v);		

		const CFactorType *clone() const 
		{
			return new CFactorType(*this);		
		}
	};
	
	class IOpType;

	/**
	This class define compilation of a source code. He is heritage come from the lex parser.
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000	
	*/
	class CCompilateur: public yyFlexLexer
	{	
	private:
		///This define is use for define an argument of a method in the language. 
		typedef std::pair<NLIAAGENT::IVarName *,NLIAAGENT::IObjectIA *> pairType;

	private:
		///Stream that contain source code.
		std::istrstream*				_StreamBuffer;
		///IO for set out text or for get an input.
		NLIAC::IIO&						_Iterface;
		///Type of last statement.
		CFactorType						_LastFact;
		///Laste code parsed.
		IBlock*							_LastBloc;
		///Last string.
		NLIAAGENT::CStringVarName		_LastString,_LastBaseObjectDef;
		bool							_BaseObjectDef;
		///List of class define in the souce code.
		CListClass						_SelfClass;
		///temporary code parsed for code manager.
		std::list<IBlock*>				_LastSaveBloc,_LastBloc1,_LastBloc2,_LastBloc3;
		///Last affected variable.
		NLIAAGENT::IObjectIA*			_LastAffectation;
		///This variable allow to simulate the heap allocation.
		CIndexStackPointer				_VarState;			
		CStackPointer					_Stack,_Heap;
		std::list<NLIASCRIPT::CStringType>	_LasVarStr,_LasAffectationVarStr;
		std::list<IOpType *>			_TypeList;
		bool							_Error;
		sint32							_LastRegistered;		
		std::list<pairType>				_Attrib;
		sint32							_DecalageHeap;
		std::list<IConstraint *>		_MethodConstraint;
		std::list<IConstraint *>		_ConstraintType;		
		std::list<NLIAAGENT::IBaseGroupType *>	_LastStringParam;
		std::list<CParam *>				_Param;
		std::list<NLIAAGENT::CStringVarName >	_MethodName;
		IOpType							*_ExpressionType;		
		IOpType							*_FlotingExpressionType;	
		std::list<IOpType *>			_ExpressionTypeTmp;
		bool							_IsFacteurIsExpression;
		bool							_FacteurEval;
		bool							_IsVoid;
		sint32							_LastPosHeap;
		sint32							_LastTypeCall;
		IConstraint						*_LastbaseClass;
		bool							_Debug;
		const char						*_SourceFileName;
		NLIAAGENT::IObjectIA					*_ResultCompile;

		// Logique
		std::list<const NLIAAGENT::CStringVarName *> _LastLogicParam;
		NLIAAGENT::CStringVarName	_LastAssert;
		NLIALOGIC::CFactBase _FactBase;
		std::list<NLIALOGIC::CFactPattern  *> _LastPatterns;
		std::list<NLIAFUZZY::CFuzzyVar *> _FuzzVars;
		sint32 _LastFVarIndex;
		sint32 _NbLogicParams;
		bool   _InLineParse;
		tDicoStr *_Dictionary;

	public:	

		/**
		The perser need an IO interface, an  source code here defined by const char *str and sint32 size.
		Remark that the const char* fileName variable is necessary for the debug code version.
		*/
		CCompilateur(NLIAC::IIO &Iterface,const char *str, sint32 size, const char* fileName):
			_StreamBuffer(NULL),
			_Iterface(Iterface),						
			_LastBloc((IBlock*)NULL),			
			_LastString("_"),
			_LastBaseObjectDef("_"),
			_Debug(false),
			_SourceFileName(fileName),
			_LastAssert("")
		{
			LastyyText[0][0] = 0;
			LastyyText[1][0] = 0;
			//_VarState = NULL;
			InitStream((char *)str,size);
			yyLine = yyColone = 1;
			_Error = false;
			CCodeBrancheRun::InputOutput = &Iterface;
			_DecalageHeap = 0;
			_ExpressionType = NULL;
			_FlotingExpressionType = NULL;
			_IsFacteurIsExpression = false;
			_FacteurEval = false;
			_IsVoid = true;
			_ResultCompile = NULL;
			_InLineParse = false;
		}

		/**
		The perser need an IO interface, an  source code here defined by const char *str which it define a file name.
		Remark that the const char* fileName variable is necessary for the debug code version.
		*/
		CCompilateur(NLIAC::IIO &Iterface,const char *str, const char* fileName):
			_StreamBuffer(NULL),
			_Iterface(Iterface),			
			_LastBloc((IBlock*)NULL),			
			_LastString("_"),
			_LastBaseObjectDef("_"),
			_Debug(false),
			_SourceFileName(fileName),
			_LastAssert("")
		{					
			//_VarState = NULL;
			InitFromFile(str);
			yyLine = yyColone = 1;
			_Error = false;
			_DecalageHeap = 0;
			_ExpressionType = NULL;
			_FlotingExpressionType = NULL;
			_IsFacteurIsExpression = false;
			_FacteurEval = false;
			_IsVoid = true;	
			_ResultCompile = NULL;
			_InLineParse = false;
		}

		///Initialize source code from file.
		void InitFromFile(const char *FileName)
		{

			ifstream file;
			file.open(FileName,ios::in);
			sint32 Size = file.gcount();
			char *Chaine = NULL;
			if(Size)
			{
				Chaine = new char [Size];
				file.read(Chaine,Size);
				file.close();
				if(_StreamBuffer) delete _StreamBuffer;
				_StreamBuffer = new std::istrstream(Chaine,Size);			
			}		
		}

		///get the locale variable define by name.
		NLIAAGENT::IObjectIA *getVar(const char *name)
		{
			NLIAAGENT::IObjectIA *var = _LastBloc->getVar(name);
			if(var == NULL)
			{
				std::list<IBlock *>::reverse_iterator  i = _LastSaveBloc.rbegin();
				while(i != _LastSaveBloc.rend())
				{
					IBlock *k = *(i);
					var = k->getVar(name);
					k = *(i);
					if(var != NULL) break;
					i ++;
				}
			}
			return var;
		}

		///Initialize the std::stream.
		void InitStream(char *str, sint32 size)
		{
			if(_StreamBuffer) delete _StreamBuffer;
			_StreamBuffer = new std::istrstream(str,size);
		}		

		virtual ~CCompilateur()
		{			

			clean();
			if(_StreamBuffer) delete _StreamBuffer;
			cleanMethodConstraint();
			cleanTypeConstraint();			
			//_Heap -= (sint32)_Heap;
		}
		
		///This function is needed by the lex parser for initializ the read stream (see lex documment).
		virtual int	yywrap()
		{
			if(_StreamBuffer != NULL && yyin == NULL)
			{			
				yyin = _StreamBuffer;			
				return 0;
			}		
			return 1;
			
		}

		
		///Clean internal list.
		void cleanTypeList();
		///Initialize variable befor parsing.
		void clean();
		///Initialisze the constraint.
		void cleanMethodConstraint();
		void cleanTypeConstraint();		

		/**
		This method generate compiled code.
		Return is a NLIAAGENT::IObjectIA::CProcessResult where:

		ResultState cintaine a processIdle if the code is generate or ProcessError if there are a prsing error.
		If there are no problem Result contain the code to execut.
		*/
		NLIAAGENT::IObjectIA::CProcessResult Compile();

		///Yacc parser.
		int	yyparse();

		///Same as printf but out put is made to the IO interface.
		void Echo(char *Er,...);	       	

		///Occur when there are a parse error.
		void yyerror(char *Er)
		{

			_Iterface.Echo("%s at line %d et colone %d\n",Er,yyLine,yyColone);
			
			//_StreamBuffer->close();
			 _Error = true;			 
			//yyclearin;
		}

		///You can use this to consult the state of the parser.
		bool parseError()
		{
			return _Error == true;
		}


		/**
		Find a method in a class define by the baseClass pointer.
		Method is define by a name or hiarchie name (a hiarchie name is a list same as [myBaseClass myClass myMethod] here it define the string myBaseClass.myClass.myMethod()).
		param is a CParam object where it define the argument type of the method.
		*/
		CFunctionTag findMethode(const NLIAAGENT::IObjectIA *baseClass,NLIAAGENT::IBaseGroupType &listName,const CParam &param);
		/**
		Find a method the last class mounted by the parser.
		Method is define by a name or hiarchie name (a hiarchie name is a list same as [myBaseClass myClass myMethod] here it define the string myBaseClass.myClass.myMethod()).
		param is a CParam object where it define the argument type of the method.
		*/
		CFunctionTag findMethode(NLIAAGENT::IBaseGroupType &listName,const CParam &param);

		/**
		Search if a member variable is validete for a class defined by baseClass.
		The variable is define by a CStringType hiarchie name in the nameHierarchy argument (a hiarchie name is a list same as [myBaseClass myClass myVariable] where it define the string myBaseClass.myClass.myVariable = 5).
		If the var name is validate then ref contain how to make reference and type containe the type of the attribut.
		*/
		sint32 isValidateVarName(const NLIAAGENT::IObjectIA *baseClass,std::list<sint32> &ref,std::list<NLIASCRIPT::CStringType> &nameHierarchy,IOpType *&type);
		/**
		Search if a member variable is validete in the lase class parsed.
		The variable is define by a CStringType hiarchie name in the nameHierarchy argument (a hiarchie name is a list same as [myBaseClass myClass myVariable] where it define the string myBaseClass.myClass.myVariable = 5).
		If the var name is validate then ref contain how to make reference and type containe the type of the attribut.
		*/
		sint32 isValidateVarName(std::list<sint32> &ref,std::list<NLIASCRIPT::CStringType> &nameHierarchy,IOpType *&type);		

		/// Set the Debug Mode
		void setDebugMode(bool bmode){ _Debug = bmode;}
		///Get the debug state.
		bool getDebugMode() const {return _Debug;}

	protected:
		NLIAC::CIdentType getTypeOfClass(const NLIAAGENT::IVarName &className) throw (NLIAE::IException)
		{
			try
			{				
				NLIAC::CIdentType id(className.getString());
				return id;
			}
			catch (NLIAE::IException &err)
			{				
				IClassInterpret *cl= _SelfClass.find(&className);								
				if(cl == NULL) throw CExceptionHaveNoType(err.what());
				else
				{
					return getTypeOfClass(*cl->getInheritanceName());
				}				
			}
		}

		CCodeBrancheRun *getCode(bool isMain = false);
		sint32 runTypeConstraint();
		sint32 runMethodConstraint();
		void errorMethodConstraint();
		void errorTypeConstraint();
		
		sint32 getNMethodConstraint()	
		{
			return  _MethodConstraint.size();
		}

		NLIAAGENT::CIdMethod findMethode(sint32 inheritance,const NLIAAGENT::IObjectIA *className,const NLIAAGENT::IVarName &methodeName,const CParam &param);		
		//sint32 getInheritanceRange(const IClassInterpret *,NLIAAGENT::IBaseGroupType &) const;
		const IClassInterpret *getInheritanceRange(sint32 &,const IClassInterpret *,NLIAAGENT::IBaseGroupType &) const;
		const IClassInterpret *getInheritanceRange(sint32 &,const IClassInterpret *,const NLIAAGENT::IVarName &) const;
		sint32 findMethodei(NLIAAGENT::IBaseGroupType &listName,const CParam &param);
		const NLIAAGENT::IObjectIA *validateHierarchyMethode(std::list<sint32> &,sint32 &,const NLIAAGENT::IObjectIA *,NLIAAGENT::IBaseGroupType &listName);

		/// Tell the _lastBlock about a newLine (for Debug)
		virtual void setNewLine();
		
		
		//sint32 getCompementShift(IClassInterpret *classType);

		//Function for the yacc interpreter.		
		sint32 castVariable(const NLIAAGENT::CStringVarName&,NLIAAGENT::CStringVarName&);
		void ifInterrogation();
		void ifInterrogationPoint();
		void ifInterrogationEnd();				
		void interrogationEnd();
		sint32 affectation();
		sint32 processingVar();
		void allocExpression(IOpCode *op,bool gd = false);
		bool computContraint();
		void RegisterClass();
		sint32 affectationMember(sint32 member);
		sint32 affectationMember(IClassInterpret *,sint32 member);
		sint32 affectationiMember(const IClassInterpret *baseClass,std::list<NLIASCRIPT::CStringType> &varHName);
		sint32 PrivateError();
		const NLIAAGENT::IObjectIA *getValidateHierarchyBase(std::list<sint32> &ref,sint32 &h,std::list<NLIASCRIPT::CStringType> &listName);
		const NLIAAGENT::IObjectIA *getValidateHierarchyBase(const NLIAAGENT::IObjectIA *base,std::list<sint32> &ref,sint32 &h,std::list<NLIASCRIPT::CStringType> &listName);		
		void nameMethodeProcessing();		
		void initParam();
		bool registerMethod();

		IConstraint *getMethodConstraint(const NLIAAGENT::IBaseGroupType &g,const CParam &p) const;
		IConstraint *getMethodConstraint(const IConstraint &c) const;
		sint32 typeOfMethod();
		void setParamVarName();
		//void setParamVarName(NLIAC::CIdentType &id);
		void pushParamExpression();
		void setImediateVar();
		void setImediateVarNill();
		void setListVar();
		void setChaineVar();
		void setStackVar(const NLIAC::CIdentType &);
		void setMethodVar();
		bool buildObject();		
		void setTypeExpression();
		void setTypeExpression(NLIAC::CTypeOfOperator::TTypeOp op,const char *txtOp);
		void setTypeExpressionG();
		void setTypeExpressionD(NLIAC::CTypeOfOperator::TTypeOp op,const char *txtOp);
		void callFunction();
		void callSend();
		void initMessageManager();
		bool endMessageManager();
	};
}
#endif
