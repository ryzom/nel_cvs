/** \file debug.cpp
 * This file contains all features that help us to debug applications
 *
 * $Id: debug.cpp,v 1.58 2002/08/23 12:25:16 lecroart Exp $
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

#ifdef HAVE_NELCONFIG_H
#  include "nelconfig.h"
#endif // HAVE_NELCONFIG_H

#include "stdmisc.h"
#include "nel/misc/log.h"
#include "nel/misc/displayer.h"
#include "nel/misc/mem_displayer.h"
#include "nel/misc/command.h"
#include "nel/misc/report.h"
#include "nel/misc/path.h"

#ifdef NL_OS_WINDOWS
#	define _WIN32_WINDOWS	0x0410
#	define WINVER			0x0400
#	include <windows.h>
#	include <direct.h>
#	include <imagehlp.h>
#	pragma comment(lib, "imagehlp.lib")
#	define getcwd(_a, _b) (_getcwd(_a,_b))
#elif defined NL_OS_UNIX
#	include <unistd.h>
#	define IsDebuggerPresent
#endif

#include <stdarg.h>

using namespace std;
 
// If you don't want to add default displayer, put 0 instead of 1. In this case, you
// have to manage yourself displayer (in final release for example, we have to put 0)
// Alternatively, you can use --without-logging when using configure to set
// it to 0.
#ifndef NEL_DEFAULT_DISPLAYER
#define NEL_DEFAULT_DISPLAYER 1
#endif // NEL_DEFAULT_DISPLAYER

// Put 0 if you don't want to display in file "log.log"
// Alternatively, you can use --without-logging when using configure to set
// it to 0.
#ifndef NEL_LOG_IN_FILE
#define NEL_LOG_IN_FILE 1
#endif // NEL_LOG_IN_FILE

#define DEFAULT_DISPLAYER NEL_DEFAULT_DISPLAYER

#define LOG_IN_FILE NEL_LOG_IN_FILE

// If true, debug system will trap crashs even if the appli is in debugger
static const bool TrapCrashInDebugger = false;

namespace NLMISC 
{

// Need an assert in the macro
bool DebugNeedAssert = false;

CLog *ErrorLog = NULL;
CLog *WarningLog = NULL;
CLog *InfoLog = NULL;
CLog *DebugLog = NULL;
CLog *AssertLog = NULL;

CMemDisplayer *DefaultMemDisplayer = NULL;
CMsgBoxDisplayer *DefaultMsgBoxDisplayer = NULL;

static CStdDisplayer *sd = NULL;
static CFileDisplayer *fd = NULL;

void nlFatalError (const char *format, ...)
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format, 256/*NLMISC::MaxCStringSize*/);

	NLMISC::DebugNeedAssert = NLMISC::DefaultMsgBoxDisplayer==0;

	NLMISC::ErrorLog->displayNL (str);

	if (NLMISC::DebugNeedAssert)
		NLMISC_BREAKPOINT;
}

void nlError (const char *format, ...)
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format, 256/*NLMISC::MaxCStringSize*/);

	NLMISC::DebugNeedAssert = NLMISC::DefaultMsgBoxDisplayer==0;

	NLMISC::ErrorLog->displayNL (str);

	if (NLMISC::DebugNeedAssert)
		NLMISC_BREAKPOINT;
}

// the default behavior is to display all in standard output and to a file named "log.log";

void initDebug2 (bool logInFile)
{
	static bool alreadyInit = false;

	if (!alreadyInit)
	{
#if DEFAULT_DISPLAYER

		// put the standard displayer everywhere

#ifdef NL_DEBUG
		DebugLog->addDisplayer (sd);
#endif // NL_DEBUG
		InfoLog->addDisplayer (sd);
		WarningLog->addDisplayer (sd);
		AssertLog->addDisplayer (sd);
		ErrorLog->addDisplayer (sd);

		// put the memory displayer everywhere

		// use the memory displayer and bypass all filter (even for the debug mode)
		DebugLog->addDisplayer (DefaultMemDisplayer, true);
		InfoLog->addDisplayer (DefaultMemDisplayer, true);
		WarningLog->addDisplayer (DefaultMemDisplayer, true);
		AssertLog->addDisplayer (DefaultMemDisplayer, true);
		ErrorLog->addDisplayer (DefaultMemDisplayer, true);

		// put the file displayer only if wanted

#if LOG_IN_FILE
		if (logInFile)
		{
#ifdef NL_DEBUG
			DebugLog->addDisplayer (fd);
#endif // NL_DEBUG
			InfoLog->addDisplayer (fd);
			WarningLog->addDisplayer (fd);
			AssertLog->addDisplayer (fd);
			ErrorLog->addDisplayer (fd);
		}
#endif // LOG_IN_FILE

		// put the message box only in release for error

		if (TrapCrashInDebugger || !IsDebuggerPresent ())
		{
			AssertLog->addDisplayer (DefaultMsgBoxDisplayer);
			ErrorLog->addDisplayer (DefaultMsgBoxDisplayer);
		}

#endif // DEFAULT_DISPLAYER
		alreadyInit = true;
	}
	else
	{
		nlwarning ("NLMISC::initDebug2() already called");
	}
}

#ifdef NL_OS_WINDOWS

//
//
//

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


class EDebug : public ETrapDebug
{
public:	
	
	EDebug() { _Reason = "Nothing about EDebug"; }

	~EDebug () { }

	EDebug(EXCEPTION_POINTERS * pexp) : m_pexp(pexp) { nlassert(pexp != 0); createWhat(); }
	EDebug(const EDebug& se) : m_pexp(se.m_pexp) { createWhat(); }

	void createWhat ()
	{
		string shortExc, longExc, subject;
		string addr, ext;
		sint skipNFirst = 0;
		_Reason = "";

		if (m_pexp == NULL)
		{
			_Reason = "Unknown exception, don't have context.";
		}
		else
		{
			switch (m_pexp->ExceptionRecord->ExceptionCode)
			{
			case EXCEPTION_ACCESS_VIOLATION          : shortExc="Access Violation"; longExc="The thread attempted to read from or write to a virtual address for which it does not have the appropriate access";
				ext = ", thread attempts to ";
				ext += m_pexp->ExceptionRecord->ExceptionInformation[0]?"write":"read";
				if (m_pexp->ExceptionRecord->ExceptionInformation[1])
					ext += toString(" at 0x%X",m_pexp->ExceptionRecord->ExceptionInformation[1]);
				else
					ext += " at <NULL>";
				break;
			case EXCEPTION_DATATYPE_MISALIGNMENT     : shortExc="Datatype Misalignment"; longExc="The thread attempted to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries, 32-bit values on 4-byte boundaries, and so on"; break;
			case EXCEPTION_BREAKPOINT                : shortExc="Breakpoint"; longExc="A breakpoint was encountered"; break;
			case EXCEPTION_SINGLE_STEP               : shortExc="Single Step"; longExc="A trace trap or other single-instruction mechanism signaled that one instruction has been executed"; break;
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED     : shortExc="Array Bounds Exceeded"; longExc="The thread attempted to access an array element that is out of bounds, and the underlying hardware supports bounds checking"; break;
			case EXCEPTION_FLT_DENORMAL_OPERAND      : shortExc="Float Denormal Operand"; longExc="One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value"; break;
			case EXCEPTION_FLT_DIVIDE_BY_ZERO        : shortExc="Float Divide By Zero"; longExc="The thread attempted to divide a floating-point value by a floating-point divisor of zero"; break;
			case EXCEPTION_FLT_INEXACT_RESULT        : shortExc="Float Inexact Result"; longExc="The result of a floating-point operation cannot be represented exactly as a decimal fraction"; break;
			case EXCEPTION_FLT_INVALID_OPERATION     : shortExc="Float Invalid Operation"; longExc="This exception represents any floating-point exception not included in this list"; break;
			case EXCEPTION_FLT_OVERFLOW              : shortExc="Float Overflow"; longExc="The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type"; break;
			case EXCEPTION_FLT_STACK_CHECK           : shortExc="Float Stack Check"; longExc="The stack overflowed or underflowed as the result of a floating-point operation"; break;
			case EXCEPTION_FLT_UNDERFLOW             : shortExc="Float Underflow"; longExc="The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type"; break;
			case EXCEPTION_INT_DIVIDE_BY_ZERO        : shortExc="Integer Divide By Zero"; longExc="The thread attempted to divide an integer value by an integer divisor of zero"; break;
			case EXCEPTION_INT_OVERFLOW              : shortExc="Integer Overflow"; longExc="The result of an integer operation caused a carry out of the most significant bit of the result"; break;
			case EXCEPTION_PRIV_INSTRUCTION          : shortExc="Privileged Instruction"; longExc="The thread attempted to execute an instruction whose operation is not allowed in the current machine mode"; break;
			case EXCEPTION_IN_PAGE_ERROR             : shortExc="In Page Error"; longExc="The thread tried to access a page that was not present, and the system was unable to load the page. -ie. the program or memory mapped file couldn't be paged in because it isn't accessable any more. Device drivers can return this exception if something went wrong with the read (i.e hardware problems)"; break;
			case EXCEPTION_ILLEGAL_INSTRUCTION       : shortExc="Illegal Instruction"; longExc="The thread tried to execute an invalid instruction -such as MMX opcodes on a non MMX system. Branching to an invalid location can cause this -something stack corruption often causes"; break;
			case EXCEPTION_NONCONTINUABLE_EXCEPTION  : shortExc="Noncontinuable Exception"; longExc="The thread attempted to continue execution after a noncontinuable exception occurred"; break;
			case EXCEPTION_STACK_OVERFLOW            : shortExc="Stack Overflow"; longExc="Stack overflow. Can occur during errant recursion, or when a function creates a particularly large array on the stack"; break;
			case EXCEPTION_INVALID_DISPOSITION       : shortExc="Invalid Disposition"; longExc="Whatever number the exception filter returned, it wasn't a value the OS knows about"; break;
			case EXCEPTION_GUARD_PAGE                : shortExc="Guard Page"; longExc="Memory Allocated as PAGE_GUARD by VirtualAlloc() has been accessed"; break;
			case EXCEPTION_INVALID_HANDLE            : shortExc="Invalid Handle"; longExc=""; break;
			case CONTROL_C_EXIT                      : shortExc="Control-C"; longExc="Lets the debugger know the user hit Ctrl-C. Seemingly for console apps only"; break;
			case STATUS_NO_MEMORY                    : shortExc="No Memory"; longExc="Called by HeapAlloc() if you specify HEAP_GENERATE_EXCEPTIONS and there is no memory or heap corruption";
				ext = ", unable to allocate ";
				ext += toString ("%d bytes", m_pexp->ExceptionRecord->ExceptionInformation [0]);
				break;
			case STATUS_WAIT_0                       : shortExc="Wait 0"; longExc=""; break;
			case STATUS_ABANDONED_WAIT_0             : shortExc="Abandoned Wait 0"; longExc=""; break;
			case STATUS_USER_APC                     : shortExc="User APC"; longExc="A user APC was delivered to the current thread before the specified Timeout interval expired"; break;
			case STATUS_TIMEOUT                      : shortExc="Timeout"; longExc=""; break;
			case STATUS_PENDING                      : shortExc="Pending"; longExc=""; break;
			case STATUS_SEGMENT_NOTIFICATION         : shortExc="Segment Notification"; longExc=""; break;
			case STATUS_FLOAT_MULTIPLE_FAULTS        : shortExc="Float Multiple Faults"; longExc=""; break;
			case STATUS_FLOAT_MULTIPLE_TRAPS         : shortExc="Float Multiple Traps"; longExc=""; break;
			case STATUS_ILLEGAL_VLM_REFERENCE        : shortExc="Illegal VLM Reference"; longExc=""; break;
			case 0xE06D7363                          : shortExc="Microsoft C++ Exception"; longExc="Microsoft C++ Exception"; break;	// cpp exception
			case 0xACE0ACE                           : shortExc=""; longExc="";
				if (m_pexp->ExceptionRecord->NumberParameters == 1)
					skipNFirst = m_pexp->ExceptionRecord->ExceptionInformation [0];
				break;	// just want the stack
			default                                  : shortExc="Unknown Exception"; longExc="Unknown Exception "+toString("0x%X", m_pexp->ExceptionRecord->ExceptionCode); break;
			};

			if(m_pexp->ExceptionRecord != NULL)
			{
				if (m_pexp->ExceptionRecord->ExceptionAddress)
					addr = toString(" at 0x%X", m_pexp->ExceptionRecord->ExceptionAddress);
				else
					addr = " at <NULL>";
			}

			string progname;
			if(!shortExc.empty() || !longExc.empty())
			{
				char name[1024];
				GetModuleFileName (NULL, name, 1023);
				progname = CFile::getFilename(name);
				progname += " ";
			}

			subject = progname + shortExc + addr;

			if (_Reason.empty())
			{
				if (!shortExc.empty()) _Reason += shortExc + " exception generated" + addr + ext + ".\n";
				if (!longExc.empty()) _Reason += longExc + ".\n";
			}

			// display the stack
			addStackAndLogToReason (skipNFirst);

			if(!shortExc.empty() || !longExc.empty())
			{
				bool i = false;
				report (progname+shortExc, "", subject, _Reason, true, 1, true, 1, true, i);
			}
		}
	}

	// display the callstack
	void addStackAndLogToReason (sint skipNFirst = 0)
	{
		DWORD symOptions = SymGetOptions();
		symOptions |= SYMOPT_LOAD_LINES;
		symOptions &= ~SYMOPT_UNDNAME;
		SymSetOptions (symOptions);
		
		nlverify (SymInitialize(getProcessHandle(), NULL, FALSE) == TRUE);

		STACKFRAME callStack;
		::ZeroMemory (&callStack, sizeof(callStack));
		callStack.AddrPC.Mode      = AddrModeFlat;
		callStack.AddrPC.Offset    = m_pexp->ContextRecord->Eip;
		callStack.AddrStack.Mode   = AddrModeFlat;
		callStack.AddrStack.Offset = m_pexp->ContextRecord->Esp;
		callStack.AddrFrame.Mode   = AddrModeFlat;
		callStack.AddrFrame.Offset = m_pexp->ContextRecord->Ebp;

		_Reason += "\nCallstack:\n";
		_Reason += "-------------------------------\n";
		for (sint32 i = 0; ; i++)
		{
			SetLastError(0);
			BOOL res = StackWalk (IMAGE_FILE_MACHINE_I386, getProcessHandle(), GetCurrentThread(), &callStack,
				m_pexp->ContextRecord, NULL, SymFunctionTableAccess, GetModuleBase/*SymGetModuleBase*/, NULL);

			if (res == FALSE || callStack.AddrFrame.Offset == 0)
				break;
		
			string symInfo, srcInfo;

			if (i >= skipNFirst)
			{
				srcInfo = getSourceInfo (callStack.AddrPC.Offset);
				symInfo = getFuncInfo (callStack.AddrPC.Offset, callStack.AddrFrame.Offset);
				_Reason += srcInfo + ": " + symInfo + "\n";
			}
		}
		_Reason += "-------------------------------\n";
		_Reason += "\n";
		if(DefaultMemDisplayer)
		{
			_Reason += "Log with no filter:\n";
			_Reason += "-------------------------------\n";
			DefaultMemDisplayer->write (_Reason);
		}
		else
		{
			_Reason += "No log\n";
		}
		_Reason += "-------------------------------\n";

		nlverify (SymCleanup(getProcessHandle()) == TRUE);
	}

	string getSourceInfo (DWORD addr)
	{
		string str;

		IMAGEHLP_LINE  line;
		::ZeroMemory (&line, sizeof (line));
		line.SizeOfStruct = sizeof(line);

		DWORD disp;
		if (SymGetLineFromAddr (getProcessHandle(), addr, &disp, &line))
		{
			str = line.FileName;
			str += "(" + toString (line.LineNumber) + ")";
		}
		else
		{
			HRESULT hr = GetLastError();
			IMAGEHLP_MODULE module;
			::ZeroMemory (&module, sizeof(module));
			module.SizeOfStruct = sizeof(module);

			if (SymGetModuleInfo (getProcessHandle(), addr, &module))
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

	HANDLE getProcessHandle()
	{
		return CSystemInfo::isNT()?GetCurrentProcess():(HANDLE)GetCurrentProcessId();
	}

	// return true if found
	bool findAndErase(string &str, const char *token, const char *replace = NULL)
	{
		int pos;
		if ((pos = str.find(token)) != string::npos)
		{
			str.erase (pos,strlen(token));
			if (replace != NULL)
				str.insert (pos, replace);
			return true;
		}
		else
			return false;
	}

	// remove space and const stuffs
	// rawType contains the type without anything (to compare with known type)
	// displayType contains the type without std:: and stl ugly things
	void cleanType(string &rawType, string &displayType)
	{
		while (findAndErase(rawType, "std::")) ;
		while (findAndErase(displayType, "std::")) ;

		while (findAndErase(rawType, "const")) ;

		while (findAndErase(rawType, " ")) ;

		while (findAndErase(rawType, "&")) ;

		// rename ugly stl type

		while (findAndErase(rawType, "classbasic_string<char,classchar_traits<char>,classallocator<char>>", "string")) ;
		while (findAndErase(displayType, "class basic_string<char,class char_traits<char>,class allocator<char> >", "string")) ;
		while (findAndErase(rawType, "classvector<char,class char_traits<char>,class allocator<char> >", "string")) ;
	}

	string getFuncInfo (DWORD funcAddr, DWORD stackAddr)
	{
		string str ("NoSymbol");

		DWORD symSize = 10000;
		PIMAGEHLP_SYMBOL  sym = (PIMAGEHLP_SYMBOL) GlobalAlloc (GMEM_FIXED, symSize);
		::ZeroMemory (sym, symSize);
		sym->SizeOfStruct = symSize;
		sym->MaxNameLength = symSize - sizeof(IMAGEHLP_SYMBOL);

		DWORD disp = 0;
		if (SymGetSymFromAddr (getProcessHandle(), funcAddr, &disp, sym) == FALSE)
		{
			return str;
		}

		CHAR undecSymbol[1024];
		if (UnDecorateSymbolName (sym->Name, undecSymbol, 1024, UNDNAME_COMPLETE | UNDNAME_NO_THISTYPE | UNDNAME_NO_SPECIAL_SYMS | UNDNAME_NO_MEMBER_TYPE | UNDNAME_NO_MS_KEYWORDS | UNDNAME_NO_ACCESS_SPECIFIERS ) > 0)
		{
			str = undecSymbol;
		}
		else if (SymUnDName (sym, undecSymbol, 1024) == TRUE)
		{
			str = undecSymbol;
		}

		// replace param with the value of the stack for this param

		string parse = str;
		str = "";
		uint pos = 0;
		sint stop = 0;

		string type;

		uint i = parse.find ("(");

		// copy the function name
		str = parse.substr(0, i);

//		nlinfo ("not parsed '%s'", parse.c_str());

		// if there s parameter, parse them
		if(i!=string::npos)
		{
			// copy the '('
			str += parse[i];
			for (i++; i < parse.size (); i++)
			{
				if (parse[i] == '<')
					 stop++;
				if (parse[i] == '>')
					 stop--;

				if (stop==0 && (parse[i] == ',' || parse[i] == ')'))
				{
					ULONG *addr = (ULONG*)(stackAddr) + 2 + pos++;

					string displayType = type;
					cleanType (type, displayType);

					char tmp[1024];
					if(type == "void")
					{
						tmp[0]='\0';
					}
					else if(type == "int")
					{
						sprintf (tmp, "%d", *addr);
					}
					else if (type == "char")
					{
						if (isprint(*addr))
						{
							sprintf (tmp, "'%c'", *addr);
						}
						else
						{
							sprintf (tmp, "%d", *addr);
						}
					}
					else if (type == "char*" && *addr != NULL)
					{
						uint pos = 0;
						tmp[pos++] = '\"';
						for (uint i = 0; i < strlen((char*)*addr); i++)
						{
							if (pos == 1020)
								break;
							char c = ((char *)*addr)[i];
							if (c == '\n')
							{
								tmp[pos++] = '\\';
								tmp[pos++] = 'n';
							}
							else if (c == '\r')
							{
								tmp[pos++] = '\\';
								tmp[pos++] = 'r';
							}
							else if (c == '\t')
							{
								tmp[pos++] = '\\';
								tmp[pos++] = 't';
							}
							else
								tmp[pos++] = c;
						}
						tmp[pos++] = '\"';
						tmp[pos++] = '\0';
					}
					else if (type == "string" && *addr != NULL)
					{
						sprintf (tmp, "\"%s\"", ((string*)addr)->c_str());
					}
					else
					{
						if(*addr == NULL)
							sprintf (tmp, "<NULL>");
						else
							sprintf (tmp, "0x%X", *addr);
					}

					str += displayType;
					if(tmp[0]!='\0')
					{
						str += "=";
						str += tmp;
					}
					str += parse[i];
					type = "";
				}
				else
				{
					type += parse[i];
				}
			}
			GlobalFree (sym);
			if (disp != 0)
			{
				str += " + ";
				str += toString (disp);
				str += " bytes";
			}
		}

//		nlinfo ("after parsing '%s'", str.c_str());

		return str;
	}

private:
	EXCEPTION_POINTERS * m_pexp;
};


void getCallStackAndLog (string &result, sint skipNFirst)
{
#ifdef NL_OS_WINDOWS
	try
	{
		DWORD array[1];
		array[0] = skipNFirst;
		RaiseException (0xACE0ACE, 0, 1, array);
	}
	catch (Exception &e)
	{
		result += e.what();
	}
#else
	result += "No callstack available";
#endif
}


static void exceptionTranslator(unsigned, EXCEPTION_POINTERS *pexp)
{
	// ace desactive because we can't debug if activated
	//if (!TrapCrashInDebugger && IsDebuggerPresent ())
	{
		if (pexp->ExceptionRecord->ExceptionCode == 0xACE0ACE)
			throw EDebug (pexp);
		else
			return;
	}
	/*else
	{
		if (pexp->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT)
			return;
		else
			throw EDebug (pexp);
	}*/
}

#endif // NL_OS_WINDOWS

void createDebug (const char *logPath, bool logInFile)
{
	static bool alreadyCreate = false;
	if (!alreadyCreate)
	{
		// Debug Info for mutexes
#ifdef MUTEX_DEBUG
		initAcquireTimeMap();
#endif

#ifdef NL_OS_WINDOWS
		//if (!IsDebuggerPresent ())
		{
			_set_se_translator(exceptionTranslator);
		}
#endif // NL_OS_WINDOWS


		ErrorLog = new CLog (CLog::LOG_ERROR);
		WarningLog = new CLog (CLog::LOG_WARNING);
		InfoLog = new CLog (CLog::LOG_INFO);
		DebugLog = new CLog (CLog::LOG_DEBUG);
		AssertLog = new CLog (CLog::LOG_ASSERT);

		sd = new CStdDisplayer ("DEFAULT_SD");
		if (TrapCrashInDebugger || !IsDebuggerPresent ())
		{
			DefaultMsgBoxDisplayer = new CMsgBoxDisplayer ("DEFAULT_MBD");
		}
#if LOG_IN_FILE
		if (logInFile)
		{
			string fn;
			if (logPath != NULL)
			{
				fn += logPath;
			}
			else
			{
				// log.log must always be accessed in creation current path directory
				char	tmpPath[1024];
				fn += getcwd(tmpPath, 1024);
				fn += "/";
			}
			fn += "log.log";
			fd = new CFileDisplayer (fn, false, "DEFAULT_FD");
		}
#endif // LOG_IN_FILE
		DefaultMemDisplayer = new CMemDisplayer ("DEFAULT_MD");
		
		initDebug2(logInFile);

		alreadyCreate = true;
	}
}




//
// Commands
//

NLMISC_COMMAND (displayMemlog, "displays the last N line of the log in memory", "[<NbLines>]")
{
	uint nbLines;

	if (args.size() == 0) nbLines = 100;
	else if (args.size() == 1) nbLines = atoi(args[0].c_str());
	else return false;

	if (DefaultMemDisplayer == NULL) return false;

	deque<string>::const_iterator it;
	
	const deque<string> &str = DefaultMemDisplayer->lockStrings ();

	if (nbLines >= str.size())
		it = str.begin();
	else
		it = str.end() - nbLines;

	DefaultMemDisplayer->write (&log);

	DefaultMemDisplayer->unlockStrings ();

	return true;
}


NLMISC_COMMAND(resetFilters, "disable all filters on Nel loggers", "[debug|info|warning|error|assert]")
{
	if(args.size() == 0)
	{
		DebugLog->resetFilters();
		InfoLog->resetFilters();
		WarningLog->resetFilters();
		ErrorLog->resetFilters();
		AssertLog->resetFilters();
	}
	else if (args.size() == 1)
	{
		if (args[0] == "debug") DebugLog->resetFilters();
		else if (args[0] == "info") InfoLog->resetFilters();
		else if (args[0] == "warning") WarningLog->resetFilters();
		else if (args[0] == "error") ErrorLog->resetFilters();
		else if (args[0] == "assert") AssertLog->resetFilters();
	}
	else
	{
		return false;
	}

	return true;
}

NLMISC_COMMAND(addPositiveFilterDebug, "add a positive filter on DebugLog", "<filterstr>")
{
	if(args.size() != 1) return false;
	DebugLog->addPositiveFilter( args[0].c_str() );
	return true;
}

NLMISC_COMMAND(addNegativeFilterDebug, "add a negative filter on DebugLog", "<filterstr>")
{
	if(args.size() != 1) return false;
	DebugLog->addNegativeFilter( args[0].c_str() );
	return true;
}

NLMISC_COMMAND(removeFilterDebug, "remove a filter on DebugLog", "[<filterstr>]")
{
	if(args.size() == 0)
		DebugLog->removeFilter();
	else if(args.size() == 1)
		DebugLog->removeFilter( args[0].c_str() );
	else return false;
	return true;
}

NLMISC_COMMAND(displayFilterDebug, "display filter on DebugLog", "")
{
	if(args.size() != 0) return false;
	DebugLog->displayFilter(log);
	return true;
}

NLMISC_COMMAND(addPositiveFilterInfo, "add a positive filter on InfoLog", "<filterstr>")
{
	if(args.size() != 1) return false;
	InfoLog->addPositiveFilter( args[0].c_str() );
	return true;
}

NLMISC_COMMAND(addNegativeFilterInfo, "add a negative filter on InfoLog", "<filterstr>")
{
	if(args.size() != 1) return false;
	InfoLog->addNegativeFilter( args[0].c_str() );
	return true;
}

NLMISC_COMMAND(removeFilterInfo, "remove a filter on InfoLog", "[<filterstr>]")
{
	if(args.size() == 0)
		InfoLog->removeFilter();
	else if(args.size() == 1)
		InfoLog->removeFilter( args[0].c_str() );
	else return false;
	return true;
}

NLMISC_COMMAND(displayFilterInfo, "display filter on InfoLog", "[d|i|w|e]")
{
	if(args.size() > 1) return false;
	if ( args.size() == 1 )
	{
		if ( strcmp( args[0].c_str(), "d" ) == 0 )
			InfoLog->displayFilter(*DebugLog);
		else if ( strcmp( args[0].c_str(), "i" ) == 0 )
			InfoLog->displayFilter(*InfoLog);
		else if ( strcmp( args[0].c_str(), "w" ) == 0 )
			InfoLog->displayFilter(*WarningLog);
		else if ( strcmp( args[0].c_str(), "e" ) == 0 )
			InfoLog->displayFilter(*ErrorLog);
		else
			return false;
	}
	else
	{
		InfoLog->displayFilter(log);
	}
	return true;
}

NLMISC_COMMAND(addPositiveFilterWarning, "add a positive filter on WarningLog", "<filterstr>")
{
	if(args.size() != 1) return false;
	WarningLog->addPositiveFilter( args[0].c_str() );
	return true;
}

NLMISC_COMMAND(addNegativeFilterWarning, "add a negative filter on WarningLog", "<filterstr>")
{
	if(args.size() != 1) return false;
	WarningLog->addNegativeFilter( args[0].c_str() );
	return true;
}

NLMISC_COMMAND(removeFilterWarning, "remove a filter on WarningLog", "[<filterstr>]")
{
	if(args.size() == 0)
		WarningLog->removeFilter();
	else if(args.size() == 1)
		WarningLog->removeFilter( args[0].c_str() );
	else return false;
	return true;
}

NLMISC_COMMAND(displayFilterWarning, "display filter on WarningLog", "")
{
	if(args.size() != 0) return false;
	WarningLog->displayFilter(log);
	return true;
}

NLMISC_COMMAND(assert, "generate a failed assert", "")
{
	if(args.size() != 0) return false;
	nlassertex (false, ("Assert generated by the assert command"));
	return true;
}

NLMISC_COMMAND(stop, "generate a stop", "")
{
	if(args.size() != 0) return false;
	nlstopex (("Stop generated by the stop command"));
	return true;
}

} // NLMISC
