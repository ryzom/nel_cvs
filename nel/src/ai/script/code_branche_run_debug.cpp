/** \file code_branche_run_debug.cpp
 *
 * $Id: code_branche_run_debug.cpp,v 1.1 2001/01/05 10:53:49 chafik Exp $
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

#include "script/code_branche_run_debug.h"
#include "script/varstack.h"

namespace NLIASCRIPT
{
	/*
	 * Constructor
	 */

	void CCodeBrancheRunDebug::initCBRD()
	{
		_LineInSourceCodeArray = new int[_Count];
		for(sint32 i = 0 ;i < _Count; i ++) 
		{
			_LineInSourceCodeArray[i] = 0; // It's a mark, because 0 could'nt be a true value.
		}
	}

	CCodeBrancheRunDebug::CCodeBrancheRunDebug(sint32 N,const IOpCode &op):
	CCodeBrancheRun(N, op),
	_SourceFileName("")
	{
		initCBRD();
	}

	CCodeBrancheRunDebug::CCodeBrancheRunDebug(sint32 N, const char* sourceCodeFileName):
	CCodeBrancheRun(N),
	_SourceFileName(sourceCodeFileName)
	{
		initCBRD();
	}

	CCodeBrancheRunDebug::CCodeBrancheRunDebug(const tListCode &l):
	CCodeBrancheRun(l),
	_SourceFileName("")
	{
		initCBRD();
	}	

	CCodeBrancheRunDebug::CCodeBrancheRunDebug(const CCodeBrancheRunDebug &l):
	CCodeBrancheRun(l),
	_LineInSourceCodeArray(NULL),
	_SourceFileName(l._SourceFileName)
	{
		initCBRD();
		for(sint32 i = 0 ;i < _Count; i ++) 
		{
			_LineInSourceCodeArray[i] = l._LineInSourceCodeArray[i];
		}
	}

	CCodeBrancheRunDebug::~CCodeBrancheRunDebug()
	{
		delete []_LineInSourceCodeArray;
	}

	const NLIAC::IBasicType* CCodeBrancheRunDebug::clone() const 
	{
		NLIAC::IBasicType *x = new CCodeBrancheRunDebug(*this);
		x->incRef();
		return x;
	}

	const NLIAC::IBasicType *CCodeBrancheRunDebug::newInstance() const 
	{
		NLIAC::IBasicType *x = new CCodeBrancheRunDebug(1,CHaltOpCode());
		x->incRef();
		return x;
	}

	void CCodeBrancheRunDebug::save(NLMISC::IStream &os)
	{
		CCodeBrancheRun::save(os);

		for(int i = 0 ;i < _Count; i ++)
		{		
			sint32 n = _LineInSourceCodeArray[i];
			os.serial( n );
		}
		std::string x( _SourceFileName );
		os.serial(x );
		
	}

	void CCodeBrancheRunDebug::load(NLMISC::IStream &is)
	{		
		int i;
		CCodeBrancheRun::load(is);
		delete[] _LineInSourceCodeArray;
		_LineInSourceCodeArray = new int[_Count];
		for(i = 0 ;i < _Count; i ++)
		{
			sint32 n;
			is.serial( n );
			_LineInSourceCodeArray[i] = (int)n;
		}
		is.serial(_SourceFileName );		
	}

	const NLIAAGENT::IObjectIA::CProcessResult& CCodeBrancheRunDebug::run(NLIAAGENT::IObjectIA &self)
	{		
		CStackPointer	stack,heap;
		CCodeContext p(stack,heap,this,&self,InputOutput);
		CVarPStack::_LocalTableRef = &heap[0];
		stack --;
		
		run(p);

		stack -= (sint32)stack; 
		heap -= (sint32)heap; 
		return _RunState;
	}	

	const NLIAAGENT::IObjectIA::CProcessResult &CCodeBrancheRunDebug::run(CCodeContext &p)
	{
		NLIAAGENT::TProcessStatement i = NLIAAGENT::processIdle;
		
		p.ContextDebug.Active = true; // We are in Debug mode.
		p.ContextDebug.callStackPush(this);

		_RunState.Result = NULL;
		while(i != NLIAAGENT::processEnd)
		{	
			i = runOpCode(p);
		}
		
		_RunState.ResultState = NLIAAGENT::processIdle;
		return _RunState;
	}
	NLIAAGENT::TProcessStatement CCodeBrancheRunDebug::runOpCode(CCodeContext &p)
	{
		char* buf;
		p.ContextDebug.Active = true; // We are in Debug mode.

		IOpCode &op = nextCode();

/*
#ifdef _DEBUG		
		char chaine[1024*8];
		op.getDebugResult(chaine,p);
		InputOutput->Echo("Operation %s\n", chaine);
#endif			
*/
		if (_LineInSourceCodeArray[_Ip-1] != 0)
		{
			fixContextDebugMode(p);

			// We print info debug only if we are running the stepByStep codeBrancheRun.
			if (   p.ContextDebug.getDebugMode() == stepByStepMode 
				&& p.ContextDebug.getCallStackTopIndex() == p.ContextDebug.getStepIndex())
			{
				printCurrentSourceCodeLine();

				bool executOpcode = false;

				while (!executOpcode)
				{
					// Wait command
					InputOutput->Echo("(db) ");
					buf = InputOutput->InPut();
					executOpcode = readCommandLine(p, buf);
					if (0 != buf[0])	p.ContextDebug.setLastCommandLine(buf);
				}
			}
		}

		if (p.ContextDebug.getDebugMode() == stopMode)
		{
			return NLIAAGENT::processEnd;
		}
		else
		{
			return op.runOpCode(p);
		}
	}

	void CCodeBrancheRunDebug::getDebugResult(char *str,CCodeContext &p) const
	{		
		_TableCode[_Ip]->getDebugResult(str,p);
	}
	
	const NLIAC::CIdentType& CCodeBrancheRunDebug::getType() const
	{		
		return IdCodeBrancheRunDebug;
	}

	void CCodeBrancheRunDebug::initCode(const tListCode &l)
	{
		del();
		_Count = l.size();

		tListCode::const_iterator i;		
		_TableCode = new IOpCode * [l.size()];
		i = l.begin();
		sint32 k = 0;
		while(i != l.end())
		{
			_TableCode[k ++] = (IOpCode *)*i++;
		}		
	}

	void CCodeBrancheRunDebug::initCode(const CCodeBrancheRunDebug &l)
	{
		del();

		_Ip = 0;
		_Count = l._Count;
		
		_TableCode = new IOpCode * [_Count];		
		for(sint32 i = 0; i < _Count;i ++)
		{
			_TableCode[i ++] = (IOpCode *)l._TableCode[i]->clone();
		}
	}

	void CCodeBrancheRunDebug::setLineCode(int ligne, int index)
	{
		_LineInSourceCodeArray[index] = ligne;
	}

	const char* CCodeBrancheRunDebug::getSourceFileName() const
	{
		return _SourceFileName.c_str();
	}

	void CCodeBrancheRunDebug::fixContextDebugMode(CCodeContext &P) const
	{
		// We are in step by step mode on the current CCodeBrancheRun :
		if (   P.ContextDebug.getBreakPointValue(_LineInSourceCodeArray[_Ip-1],getSourceFileName()) // After a BreakPoint.
			|| P.ContextDebug.getDebugMode() == stepInMode	// After a stepInMode.
			|| P.ContextDebug.getDebugMode() == stepOutMode && (P.ContextDebug.getCallStackTopIndex() < P.ContextDebug.getStepIndex()) // After leaving a branche in stepOutMode.
			|| P.ContextDebug.getDebugMode() == stepByStepMode && P.ContextDebug.getCallStackTopIndex() < P.ContextDebug.getStepIndex()) // If we reach a higer level branch in stepBySteplMode.
		{
			P.ContextDebug.setDebugMode(stepByStepMode);
			P.ContextDebug.setStepIndex(P.ContextDebug.getCallStackTopIndex());
		}
	}

	void CCodeBrancheRunDebug::printSourceCodeLine(int first, int last) const
	{
		FILE* f;
		sint32 i, j, k, size, lineNumber;
		char* buf;
		char* lineTxt;

		lineNumber = this->getCurrentSourceCodeLineNumber();
		// Read the file
		f = fopen(getSourceFileName(),"rb");
		fseek(f,0,SEEK_END);
		size = ftell(f);
		rewind(f);
		buf = new char [size + 3];
		fread(buf+1, sizeof( char ), size, f);
		fclose(f);
		buf[0] = ' ';
		buf[size] = '\n';
		buf[size+1] = 0;

		// first should be lowest than last.
		if (first > last) last = first;

		// Seek the good line
		i = 0; // Line start
		j = 0; // Line end
		k = 0; // Line number
		lineTxt = new char[1024];

		// Seek the first line
		while (k != first && j < size)
		{
			j++;
			i = j;
			while (buf[j] != '\n')
			{
				j++;
			}
			k++;
		}

		// Print until the last line
		while (k != last+1 && j < size)
		{
			strncpy (lineTxt, buf+i,j-i);
			lineTxt[j-i] = 0;

			// Print debug informations
			if ( k == lineNumber)
			{
				InputOutput->Echo("%d ->\t %s\n", k, lineTxt);
			}
			else
			{
				InputOutput->Echo("%d   \t %s\n", k, lineTxt);
			}

			j++;
			i = j;
			while (buf[j] != '\n')
			{
				j++;
			}
			k++;
		}
		delete[] buf;
		delete[] lineTxt;
	}

	void CCodeBrancheRunDebug::printCurrentSourceCodeLine() const
	{
		uint16 lineNumber = getCurrentSourceCodeLineNumber();
		printSourceCodeLine(lineNumber, lineNumber);
	}

	uint16	CCodeBrancheRunDebug::getCurrentSourceCodeLineNumber() const
	{
		if (_Ip ==0)
			return 0; // This shouldn't appen.
		
		if (_LineInSourceCodeArray[_Ip-1] != 0)
		{
			return (_LineInSourceCodeArray[_Ip-1]);
		}
		else
		{
			sint32 i = 2;
			while (_LineInSourceCodeArray[_Ip-i] == 0)
			{
				i++;
			}
			return _LineInSourceCodeArray[_Ip-i];
		}
	}

	void CCodeBrancheRunDebug::printVariable (CCodeContext &P, const char* c) const
	{
		sint32 begin;
		const NLIAAGENT::IObjectIA* base;
		const NLIAAGENT::IVarName* ivn;
		NLIAAGENT::CStringType* st;
		NLIAAGENT::CStringVarName* svn;
		char* buf = new char[1024*8];
		char* bufA = new char[1024*8];
		char* bufX = new char[1024*8];
		int i;
		int j;
		char*  dotPos;
		buf[0] = 0;
		bufA[0] = 0;
		bufX[0] = 0;

		// Wee look if it's a composed varaible. If so, we just store the first part in buf and the second in bufX.
		strcpy (buf, c);
		dotPos =  strchr(buf,'.');
		if(dotPos != NULL)
		{
			strcpy(bufX, dotPos+1);
			strchr(buf,'.')[0]= '\0';
		}

		// Seek in the heap from the bottom the buf in order to know if it's a local variable.
		begin = (int)P.ContextDebug.HeapDebug;
		for (i = begin; i>=0; i--)
		{
			base = P.ContextDebug.HeapDebug[i];
			if (base)
			{
				if (base->getType() == NLIAAGENT::CStringType::IdStringType)
				{
					st = (NLIAAGENT::CStringType*) base;
					ivn = &(st->getStr());
					if (ivn->getType() == NLIAAGENT::CStringVarName::IdStringVarName)
					{
						svn = (NLIAAGENT::CStringVarName*) ivn;
						if (strcmp(svn->getString(), buf) == 0)
						{
							break;
						}
					}
				}
			}
		}

		base = NULL;
		if (i>=0)
		{
			// It's a local variable
			base = P.Heap[i];
		}
		else
		{
			j = P.Self->getStaticMemberIndex(NLIAAGENT::CStringVarName(buf));
			if (j >= 0)
			{
				// It's a static member.
				base = P.Self->getStaticMember(j);
			}
		}

		if (base != NULL)
		{
			// We found a reference.
			if (bufX[0] == 0)
			{
				// The variable wasn't composit.
				base->getDebugString(buf);
				InputOutput->Echo("%s\n",buf);
			}
			else
			{
				// The variable was composit.
				dotPos =  strchr(bufX,'.');
				while (dotPos != NULL)
				{
					strcpy(buf,bufX);
					strcpy(bufX, dotPos+1);
					strcpy(bufA,buf);
					strchr(bufA,'.')[0]= '\0';
					j = base->getStaticMemberIndex(NLIAAGENT::CStringVarName(bufA));
					if (j >= 0)
					{
						base =  base->getStaticMember(j);
					}
					dotPos =  strchr(bufX,'.');
				}
				j = base->getStaticMemberIndex(NLIAAGENT::CStringVarName(bufX));
				if (j >= 0)
				{
					base->getStaticMember(j)->getDebugString(buf);
					InputOutput->Echo("%s\n",buf);
				}
			}
		}

		delete[] buf;
		delete[] bufA;
		delete[] bufX;
	}
	void CCodeBrancheRunDebug::printArgs (CCodeContext &P) const
	{
		if (P.Param.size() >0 )
		{
			NLIAAGENT::CIteratorContener Ita = ((NLIAAGENT::CGroupType *) P.ContextDebug.Param.back())->getIterator();
			NLIAAGENT::CIteratorContener Itb = ((NLIAAGENT::CGroupType *) P.Param.back())->getIterator();;
			char buf[1024*8];
			while(!Ita.isInEnd())
			{			
				NLIAAGENT::CStringType *o = (NLIAAGENT::CStringType *)Ita++;
				NLIAAGENT::CStringType *p = (NLIAAGENT::CStringType *)Itb++;
				p->getDebugString(buf);
				InputOutput->Echo("%s = %s\n", o->getStr().getString(), buf);
			}
		}
	}

	bool CCodeBrancheRunDebug::readCommandLine(CCodeContext &P, const char* buf)
	{
		char* sourceFileName;
		char* varName;
		uint16 lineNumber;
		sint16 first, last;
		bool ret = false;

		switch (buf[0])
		{
		case 'p' :
			// p(rint) expression	| Inspect variables / attributes, evaluate expressions, call functions/methods.
			varName = new char[1024];
			sscanf(buf+1, "%s", varName);
			printVariable(P,varName);
			delete[] varName;
			break;
		case 'w' :
			// w(here)				| Dump the call stack : the context of the current program location.
			P.ContextDebug.callStackPrint(InputOutput);
			break;
		case 'u' :
			// u(p)					| Travel up (to newer) frames in the call stack.
			P.ContextDebug.stepIndexUp();
			break;
		case 'd' :
			// d(own)				| Travel down (to older) frames in the call stack.
			P.ContextDebug.stepIndexDown();
			break;
		case 'l' :
			// l(ist) first?, last?	| List the current context's source code file : 11 lines, or around lines numbers.
			first = last = 0;
			sscanf(buf+1, "%d %d",&first, &last);
			if (0 == first) first = this->getCurrentSourceCodeLineNumber() - 6;
			if (0 >  first) first = 0;
			if (0 == last) last = first + 11;
			printSourceCodeLine(first, last);
			break;
		case 'b' :
			// b(reak) line?		| Set more breakpoints on functions or line numbers, or list active breaks.
			sourceFileName = new char[1024];
			sourceFileName[0] = 0;
			lineNumber = 0;
			sscanf(buf+1,"%d %s",&lineNumber,sourceFileName);
			if (0 == lineNumber)
			{
				P.ContextDebug.printActiveBeaks(InputOutput);
			}
			else
			{
				if (sourceFileName[0])
				{
					sint32 h = strcmp(sourceFileName, getSourceFileName());
					P.ContextDebug.addBreakPoint(lineNumber,sourceFileName);
				}
				else
				{
					P.ContextDebug.addBreakPoint(lineNumber,getSourceFileName());
				}
			}
			delete[] sourceFileName;
			break;
		case 'e' :
			// e(rase) line?		| Erase a specific breakpoint (or all of them)
			sourceFileName = new char[1024];
			sourceFileName[0] = 0;
			sscanf(buf+1,"%d %s",&lineNumber,sourceFileName);
			if (sourceFileName[0])
			{
				sint32 h = strcmp(sourceFileName, getSourceFileName());
				P.ContextDebug.eraseBreakPoint(lineNumber,sourceFileName);
			}
			else
			{
				P.ContextDebug.eraseBreakPoint(lineNumber,getSourceFileName());
			}
			delete[] sourceFileName;
			break;
		case 's' :
			// s(tep)				| Run the next line and stop; stop in called functions.
			P.ContextDebug.setDebugMode(stepInMode);
			ret = true;
			break;
		case 'n' :
			// n(ext)				| Run the next line and stop, but don't stop in functions it calls.
			P.ContextDebug.setDebugMode(stepByStepMode);
			ret = true;
			break;
		case 'r' :
			// r(eturn)				| Continue silently until the current function returns.
			P.ContextDebug.setDebugMode(stepOutMode);
			ret = true;
			break;
		case 'c' :
			// c(ontinue)			| Continue programm execution until an error, breakpoint, or exit.
			P.ContextDebug.setDebugMode(runMode);
			ret = true;
			break;
		case 'a' :
			// a(rgs)				| Print arguments passed to the current context's function.
			printArgs(P);
			break;
		case 'q' :
			// q(uit)				| Quit the debugger; also terminates the code being debugged.
			P.ContextDebug.setDebugMode(stopMode);
			ret = true;
			break;
		case 0 :
			// <enter-key>			| Repeat the last command.
			ret = this->readCommandLine(P, P.ContextDebug.getLastCommandLine());
			break;
		case 'h' :
			// h(elp) command ?		| Get command help; you can forget the rest if you remember 'h'.

			InputOutput->Echo("p(rint) expr.\t| Inspect variables / attributes.\n");
			InputOutput->Echo("w(here)\t\t| Dump the call stack.\n");
			InputOutput->Echo("u(p)\t\t| Travel up (to newer) frames in the call stack.\n");
			InputOutput->Echo("d(own)\t\t| Travel down (to older) frames in the call stack.\n");
			InputOutput->Echo("l(ist) f?, l?\t| List the current context's source code file.\n");
			InputOutput->Echo("b(reak) line?\t| Set more breakpoints on line numbers, or list active breaks.\n");
			InputOutput->Echo("e(rase) line?\t| Erase a specific breakpoint (or all of them)\n");
			InputOutput->Echo("s(tep)\t\t| Run the next line and stop; stop in called functions.\n");
			InputOutput->Echo("n(ext)\t\t| Run the next line and stop, don't stop in functions it calls.\n");
			InputOutput->Echo("r(eturn)\t| Continue silently until the current function returns.\n");
			InputOutput->Echo("c(ontinue)\t| Continue programm execution until breakpoint, or exit.\n");
			InputOutput->Echo("a(rgs)\t\t| Print arguments passed to the current context's function.\n");
			InputOutput->Echo("q(uit)\t\t| Quit the debugger; also terminates the code being debugged.\n");
			InputOutput->Echo("<enter-key>\t| Repeat the last command.\n");
			InputOutput->Echo("h(elp) command ?| Get command help; you can forget the rest if you remember 'h'.\n");
			break;
		}

		return ret;
	}
}
