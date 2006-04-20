/** \file mem_displayer.cpp
 * TODO: File description
 *
 * $Id: mem_displayer.cpp,v 1.17.8.1 2006/04/20 15:36:36 boucher Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "stdmisc.h"

#include "nel/misc/types_nl.h"

#include <cstdio>

#include <iostream>
#include <fstream>
//#include <sstream>
#include <iomanip>
#include <string>

#include "nel/misc/mem_displayer.h"
#include "nel/misc/path.h"
#include "nel/misc/command.h"
#include "nel/misc/debug.h"

#ifdef NL_OS_WINDOWS
#	include <windows.h>
#	include <imagehlp.h>
#	pragma comment(lib, "imagehlp.lib")
#endif // NL_OS_WINDOWS

using namespace std;

namespace NLMISC {


//  UNTIL we found who to walk in the call stack wihtout error, we disactive this feature


#ifdef NL_OS_WINDOWS

static const uint32 stringSize = 1024;

static string getFuncInfo (DWORD funcAddr, DWORD stackAddr)
{
	string str ("NoSymbol");

	DWORD symSize = 10000;
	PIMAGEHLP_SYMBOL  sym = (PIMAGEHLP_SYMBOL) GlobalAlloc (GMEM_FIXED, symSize);
	::ZeroMemory (sym, symSize);
	sym->SizeOfStruct = symSize;
	sym->MaxNameLength = symSize - sizeof(IMAGEHLP_SYMBOL);

	DWORD disp = 0;
	if (SymGetSymFromAddr (GetCurrentProcess(), funcAddr, &disp, sym) == FALSE)
	{
		return str;
	}

	CHAR undecSymbol[stringSize];
	if (UnDecorateSymbolName (sym->Name, undecSymbol, stringSize, UNDNAME_COMPLETE | UNDNAME_NO_THISTYPE | UNDNAME_NO_SPECIAL_SYMS | UNDNAME_NO_MEMBER_TYPE | UNDNAME_NO_MS_KEYWORDS | UNDNAME_NO_ACCESS_SPECIFIERS ) > 0)
	{
		str = undecSymbol;
	}
	else if (SymUnDName (sym, undecSymbol, stringSize) == TRUE)
	{
		str = undecSymbol;
	}

	if (disp != 0)
	{
		str += " + ";
		str += toString ((uint32)disp);
		str += " bytes";
	}

	// replace param with the value of the stack for this param

	string parse = str;
	str = "";
	uint pos = 0;
	sint stop = 0;

	for (uint i = 0; i < parse.size (); i++)
	{
		if (parse[i] == '<')
			 stop++;
		if (parse[i] == '>')
			 stop--;

		if (stop==0 && (parse[i] == ',' || parse[i] == ')'))
		{
			char tmp[32];
			sprintf (tmp, "=0x%X", *((ULONG*)(stackAddr) + 2 + pos++));
			str += tmp;
		}
		str += parse[i];
	}
	GlobalFree (sym);

	return str;
}

static string getSourceInfo (DWORD addr)
{
	string str;

	IMAGEHLP_LINE  line;
	::ZeroMemory (&line, sizeof (line));
	line.SizeOfStruct = sizeof(line);

// It doesn't work in windows 98
/*	DWORD disp;
	if (SymGetLineFromAddr (GetCurrentProcess(), addr, &disp, &line))
	{
		str = line.FileName;
		str += "(" + toString (line.LineNumber) + ")";
	}
	else
*/	{
		IMAGEHLP_MODULE module;
		::ZeroMemory (&module, sizeof(module));
		module.SizeOfStruct = sizeof(module);

		if (SymGetModuleInfo (GetCurrentProcess(), addr, &module))
		{
			str = module.ModuleName;
		}
		else
		{
			str = "<NoModule>";
		}
		char tmp[32];
		sprintf (tmp, "!0x%X", addr);
		str += tmp;
	}
	
	return str;
}

static DWORD __stdcall GetModuleBase(HANDLE hProcess, DWORD dwReturnAddress)
{
	IMAGEHLP_MODULE moduleInfo;

	if (SymGetModuleInfo(hProcess, dwReturnAddress, &moduleInfo))
		return moduleInfo.BaseOfImage;
	else
	{
		MEMORY_BASIC_INFORMATION memoryBasicInfo;

		if (::VirtualQueryEx(hProcess, (LPVOID) dwReturnAddress,
			&memoryBasicInfo, sizeof(memoryBasicInfo)))
		{
			DWORD cch = 0;
			char szFile[MAX_PATH] = { 0 };

		 cch = GetModuleFileNameA((HINSTANCE)memoryBasicInfo.AllocationBase,
								 szFile, MAX_PATH);

		if (cch && (lstrcmp(szFile, "DBFN")== 0))
		{
			 if (!SymLoadModule(hProcess,
				   NULL, "MN",
				   NULL, (DWORD) memoryBasicInfo.AllocationBase, 0))
				{
					DWORD dwError = GetLastError();
//					nlinfo("Error: %d", dwError);
				}
		}
		else
		{
		 if (!SymLoadModule(hProcess,
			   NULL, ((cch) ? szFile : NULL),
			   NULL, (DWORD) memoryBasicInfo.AllocationBase, 0))
			{
				DWORD dwError = GetLastError();
//				nlinfo("Error: %d", dwError);
			 }

		}

		 return (DWORD) memoryBasicInfo.AllocationBase;
	  }
//		else
//			nlinfo("Error is %d", GetLastError());
	}

	return 0;
}


static void displayCallStack (CLog *log)
{
	static string symbolPath;

	DWORD symOptions = SymGetOptions();
	symOptions |= SYMOPT_LOAD_LINES; 
	symOptions &= ~SYMOPT_UNDNAME;
	SymSetOptions (symOptions);

	//
	// Create the path where to find the symbol
	//

	if (symbolPath.empty())
	{
		CHAR tmpPath[stringSize];

		symbolPath = ".";

		if (GetEnvironmentVariable ("_NT_SYMBOL_PATH", tmpPath, stringSize))
		{
			symbolPath += ";";
			symbolPath += tmpPath;
		}

		if (GetEnvironmentVariable ("_NT_ALTERNATE_SYMBOL_PATH", tmpPath, stringSize))
		{
			symbolPath += ";";
			symbolPath += tmpPath;
		}

		if (GetEnvironmentVariable ("SYSTEMROOT", tmpPath, stringSize))
		{
			symbolPath += ";";
			symbolPath += tmpPath;
			symbolPath += ";";
			symbolPath += tmpPath;
			symbolPath += "\\system32";
		}
	}

	//
	// Initialize
	//

	if (SymInitialize (GetCurrentProcess(), NULL, FALSE) == FALSE)
	{
		nlwarning ("DISP: SymInitialize(%p, '%s') failed", GetCurrentProcess(), symbolPath.c_str());
		return;
	}

	CONTEXT context;
	::ZeroMemory (&context, sizeof(context));
	context.ContextFlags = CONTEXT_FULL;

	if (GetThreadContext (GetCurrentThread(), &context) == FALSE)
	{
		nlwarning ("DISP: GetThreadContext(%p) failed", GetCurrentThread());
		return;
	}
	
	STACKFRAME callStack;
	::ZeroMemory (&callStack, sizeof(callStack));
	callStack.AddrPC.Mode      = AddrModeFlat;
	callStack.AddrPC.Offset    = context.Eip;
	callStack.AddrStack.Mode   = AddrModeFlat;
	callStack.AddrStack.Offset = context.Esp;
	callStack.AddrFrame.Mode   = AddrModeFlat;
	callStack.AddrFrame.Offset = context.Ebp;

	for (uint32 i = 0; ; i++)
	{
		BOOL res = StackWalk (IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(), &callStack,
			NULL, NULL, SymFunctionTableAccess, GetModuleBase, NULL);

/*		if (res == FALSE)
		{
			DWORD r = GetLastError ();
			nlinfo ("%d",r);
		}
*/
		if (i == 0)
		   continue;

		if (res == FALSE || callStack.AddrFrame.Offset == 0)
			break;
	
		string symInfo, srcInfo;

		symInfo = getFuncInfo (callStack.AddrPC.Offset, callStack.AddrFrame.Offset);
		srcInfo = getSourceInfo (callStack.AddrPC.Offset);

		log->displayNL ("   %s : %s", srcInfo.c_str(), symInfo.c_str());
	}
}

#else // NL_OS_WINDOWS

static void displayCallStack (CLog *log)
{
	log->displayNL ("no call stack info available");
}

#endif // NL_OS_WINDOWS


/*
 * Constructor
 */
CMemDisplayer::CMemDisplayer (const char *displayerName) : IDisplayer (displayerName), _NeedHeader(true), _MaxStrings(50), _CanUseStrings(true)
{
	setParam (50);
}

void CMemDisplayer::setParam (uint32 maxStrings)
{
	_MaxStrings = maxStrings;
}


// Log format: "2000/01/15 12:05:30 <ProcessName> <LogType> <ThreadId> <FileName> <Line> : <Msg>"
void CMemDisplayer::doDisplay ( const CLog::TDisplayInfo& args, const char *message )
{
//	stringstream	ss;
	string str;
	bool			needSpace = false;

	if (!_CanUseStrings) return;

	if (_NeedHeader)
	{
		str += HeaderString();
		_NeedHeader = false;
	}

	if (args.Date != 0)
	{
		str += dateToHumanString(args.Date);
		needSpace = true;
	}

	if (!args.ProcessName.empty())
	{
		if (needSpace) { str += " "; needSpace = false; }
		str += args.ProcessName;
		needSpace = true;
	}

	if (args.LogType != CLog::LOG_NO)
	{
		if (needSpace) { str += " "; needSpace = false; }
		str += logTypeToString(args.LogType);
		needSpace = true;
	}

	// Write thread identifier
	if ( args.ThreadId != 0 )
	{
		if (needSpace) { str += " "; needSpace = false; }
#ifdef NL_OS_WINDOWS
		str += NLMISC::toString("%5x", args.ThreadId);
#else
		str += NLMISC::toString("%d", args.ThreadId);
#endif
		needSpace = true;
	}

	if (args.FileName != NULL)
	{
		if (needSpace) { str += " "; needSpace = false; }
		str += CFile::getFilename(args.FileName);
		needSpace = true;
	}

	if (args.Line != -1)
	{
		if (needSpace) { str += " "; needSpace = false; }
		str += NLMISC::toString(args.Line);
		needSpace = true;
	}
	
	if (needSpace) { str += " : "; needSpace = false; }

	str += message;

	// clear old line
	while (_Strings.size () > _MaxStrings)
	{
		_Strings.pop_front ();
	}

	_Strings.push_back (str);
}

void CMemDisplayer::write (CLog *log, bool quiet)
{
	if (log == NULL)
		log = InfoLog;

	if ( ! quiet )
	{
		log->forceDisplayRaw ("------------------------------------------------------------------------------\n");
		log->forceDisplayRaw ("----------------------------------------- display MemDisplayer history -------\n");
		log->forceDisplayRaw ("------------------------------------------------------------------------------\n");
	}
	for (deque<string>::iterator it = _Strings.begin(); it != _Strings.end(); it++)
	{
		log->forceDisplayRaw ((*it).c_str());
	}
	if ( ! quiet )
	{
		log->forceDisplayRaw ("------------------------------------------------------------------------------\n");
		log->forceDisplayRaw ("----------------------------------------- display MemDisplayer callstack -----\n");
		log->forceDisplayRaw ("------------------------------------------------------------------------------\n");
		displayCallStack(log);
		log->forceDisplayRaw ("------------------------------------------------------------------------------\n");
		log->forceDisplayRaw ("----------------------------------------- end of MemDisplayer display --------\n");
		log->forceDisplayRaw ("------------------------------------------------------------------------------\n");
	}
}

void CMemDisplayer::write (string &str, bool crLf)
{
	for (deque<string>::iterator it = _Strings.begin(); it != _Strings.end(); it++)
	{
		str += (*it);
		if ( crLf )
		{
			if ( (!str.empty()) && (str[str.size()-1] == '\n') )
			{
				str[str.size()-1] = '\r';
				str += '\n';
			}
		}
	}
}


void	CLightMemDisplayer::doDisplay ( const CLog::TDisplayInfo& args, const char *message )
{
	//stringstream	ss;
	string str;
	//bool			needSpace = false;
	
	if (!_CanUseStrings) return;
	
	str += message;
	
	// clear old line
	while (_Strings.size () >= _MaxStrings)
	{
		_Strings.pop_front ();
	}
	
	_Strings.push_back (str);
}


} // NLMISC
