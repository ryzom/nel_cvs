/** \file displayer.cpp
 * Little easy displayers implementation
 *
 * $Id: displayer.cpp,v 1.39 2002/08/23 12:27:13 lecroart Exp $
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

#include "stdmisc.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "nel/misc/path.h"
#include "nel/misc/mutex.h"
#include "nel/misc/report.h"

#include "nel/misc/debug.h"


#ifdef NL_OS_WINDOWS
// these defines is for IsDebuggerPresent(). it'll not compile on windows 95
// just comment this and the IsDebuggerPresent to compile on wiindows 95
#	define _WIN32_WINDOWS	0x0410
#	define WINVER			0x0400
#	include <windows.h>
#else
#	define IsDebuggerPresent() false
#endif

#include "nel/misc/displayer.h"

using namespace std;

namespace NLMISC
{

static char *LogTypeToString[][8] = {
	{ "", "ERR", "WRN", "INF", "DBG", "STT", "AST", "UKN" },
	{ "", "Error", "Warning", "Information", "Debug", "Statistic", "Assert", "Unknown" },
	{ "", "A fatal error occurs. The program must quit", "", "", "", "", "A failed assertion occurs", "" },
};

const char *IDisplayer::logTypeToString (CLog::TLogType logType, bool longFormat)
{
	if (logType < CLog::LOG_NO || logType > CLog::LOG_UNKNOWN)
		return "<NotDefined>";

	return LogTypeToString[longFormat?1:0][logType];
}

const char *IDisplayer::dateToHumanString ()
{
	time_t date;
	time (&date);
	return dateToHumanString (date);
}

const char *IDisplayer::dateToHumanString (time_t date)
{
	static char cstime[25];
	strftime (cstime, 25, "%y/%m/%d %H:%M:%S", localtime(&date));
	return cstime;
}

const char *IDisplayer::dateToComputerString (time_t date)
{
	static char cstime[25];
	smprintf (cstime, 25, "%ld", &date);
	return cstime;
}

const char *IDisplayer::HeaderString ()
{
	static char header[1024];
	smprintf(header, 1024, "\nLog Starting [%s]\n", dateToHumanString());
	return header;
}


IDisplayer::IDisplayer(const char *displayerName)
{
	_Mutex = new CMutex (string(displayerName)+"DISP");
	DisplayerName = displayerName;
}

IDisplayer::~IDisplayer()
{
	delete _Mutex;
}

/*
 * Display the string where it does.
 */
void IDisplayer::display ( const TDisplayInfo& args, const char *message )
{
	_Mutex->enter();
	try
	{
		doDisplay( args, message );
	}
	catch (Exception &)
	{
		// silence
	}
	_Mutex->leave();
}


// Log format : "<LogType> <ThreadNo> <FileName> <Line> <ProcessName> : <Msg>"
void CStdDisplayer::doDisplay ( const TDisplayInfo& args, const char *message )
{
	bool needSpace = false;
	stringstream ss;

	if (args.LogType != CLog::LOG_NO)
	{
		ss << logTypeToString(args.LogType);
		needSpace = true;
	}

	// Write thread identifier
	if ( args.ThreadId != 0 )
	{
		ss << setw(5) << args.ThreadId;
		needSpace = true;
	}

	if (args.Filename != NULL)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << CFile::getFilename(args.Filename);
		needSpace = true;
	}

	if (args.Line != -1)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << args.Line;
		needSpace = true;
	}
	
	if (!args.ProcessName.empty())
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << args.ProcessName;
		needSpace = true;
	}

	if (needSpace) { ss << " : "; needSpace = false; }

	ss << message;

	string s = ss.str();

	// we don't use cout because sometimes, it crashs because cout isn't already init, printf doesn t crash.
	printf ("%s", s.c_str()); 
	printf (args.CallstackAndLog.c_str());

	fflush(stdout);

#ifdef NL_OS_WINDOWS
	// display the string in the debugger is the application is started with the debugger
	if (IsDebuggerPresent ())
	{
		stringstream ss2;
		needSpace = false;

		if (args.Filename != NULL) ss2 << args.Filename;

		if (args.Line != -1)
		{
			ss2 << '(' << args.Line << ')';
			needSpace = true;
		}

		if (needSpace) { ss2 << " : "; needSpace = false; }

		if (args.LogType != CLog::LOG_NO)
		{
			ss2 << logTypeToString(args.LogType);
			needSpace = true;
		}

		// Write thread identifier
		if ( args.ThreadId != 0 )
		{
			ss2 << setw(5) << args.ThreadId << ": ";
		}

		ss2 << message;

		const sint maxOutString = 2*1024;

		if(ss2.str().size() < maxOutString)
		{
			//////////////////////////////////////////////////////////////////
			// WARNING: READ THIS !!!!!!!!!!!!!!!! ///////////////////////////
			// If at the release time, it freezes here, it's a microsoft bug:
			// http://support.microsoft.com/support/kb/articles/q173/2/60.asp
			OutputDebugString(ss2.str().c_str());
		}
		else
		{
			/*OutputDebugString(ss2.str().c_str());
			OutputDebugString("\n\t\t\t");
			OutputDebugString("message end: ");
			OutputDebugString(&message[strlen(message) - 1024]);
			OutputDebugString("\n");*/

			sint count = 0;	
			uint n = strlen(message);
			std::string s(&ss2.str().c_str()[0], (ss2.str().size() - n));
			OutputDebugString(s.c_str());
			
			while(true)
			{												
				
				if((n - count) < maxOutString )
				{
					s = std::string(&message[count], (n - count));
					OutputDebugString(s.c_str());
					OutputDebugString("\n");
					break;
				}	
				else
				{
					s = std::string(&message[count] , count + maxOutString);
					OutputDebugString(s.c_str());
					OutputDebugString("\n\t\t\t");
					count += maxOutString;
				}
			}
		}

		// OutputDebugString is a big shit, we can't display big string in one time, we need to split
		uint32 pos = 0;
		string splited;
		while(true)
		{
			if (pos+1000 < args.CallstackAndLog.size ())
			{
				splited = args.CallstackAndLog.substr (pos, 1000);
				OutputDebugString(splited.c_str());
				pos += 1000;
			}
			else
			{
				splited = args.CallstackAndLog.substr (pos);
				OutputDebugString(splited.c_str());
				break;
			}
		}
	}
#endif
}

CFileDisplayer::CFileDisplayer (const std::string &filename, bool eraseLastLog, const char *displayerName) : IDisplayer (displayerName), _NeedHeader(true)
{
	setParam (filename, eraseLastLog);
}

CFileDisplayer::CFileDisplayer () : IDisplayer (""), _NeedHeader(true)
{
}

void CFileDisplayer::setParam (const std::string &filename, bool eraseLastLog)
{
	_FileName = filename;

	if (filename.empty())
	{
		nlwarning ("CFileDisplayer::setParam(): Can't create file with empty filename, don't log");
		return;
	}

	if (eraseLastLog)
	{
		ofstream ofs (filename.c_str(), ios::out | ios::trunc);
		if (!ofs.is_open())
		{
			nlwarning ("CFileDisplayer::setParam(): Can't open and clear the log file '%s', don't log", filename.c_str());
		}
	}
}


// Log format: "2000/01/15 12:05:30 <ProcessName> <LogType> <ThreadId> <Filename> <Line> : <Msg>"
void CFileDisplayer::doDisplay ( const TDisplayInfo& args, const char *message )
{
	bool needSpace = false;
	stringstream ss;

	// if the filename is not set, don't log
	if (_FileName.empty()) return;

	if (args.Date != 0)
	{
		ss << dateToHumanString(args.Date);
		needSpace = true;
	}

	if (!args.ProcessName.empty())
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << args.ProcessName;
		needSpace = true;
	}

	if (args.LogType != CLog::LOG_NO)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << logTypeToString(args.LogType);
		needSpace = true;
	}

	// Write thread identifier
	if ( args.ThreadId != 0 )
	{
		ss << setw(5) << args.ThreadId;
		needSpace = true;
	}

	if (args.Filename != NULL)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << CFile::getFilename(args.Filename);
		needSpace = true;
	}

	if (args.Line != -1)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << args.Line;
		needSpace = true;
	}
	
	if (needSpace) { ss << " : "; needSpace = false; }

	ss << message;

	// if the file is too big (>5mb), rename it and create another one
	if (CFile::getFileSize(_FileName) > 5000000)
	{
		string name = CFile::findNewFile (_FileName);
		rename (_FileName.c_str(), name.c_str());
	}

	ofstream ofs (_FileName.c_str (), ios::out | ios::app);
	if (ofs.is_open ())
	{
		if (_NeedHeader)
		{
			ofs << HeaderString();
			_NeedHeader = false;
		}
		
		ofs << ss.str();
		ofs << args.CallstackAndLog;
		ofs.close();
	}
}



// Log format in clipboard: "2000/01/15 12:05:30 <LogType> <ProcessName> <FileName> <Line>: <Msg>"
// Log format on the screen: in debug   "<ProcessName> <FileName> <Line>: <Msg>"
//                           in release "<Msg>"
void CMsgBoxDisplayer::doDisplay ( const TDisplayInfo& args, const char *message)
{
#ifdef NL_OS_WINDOWS

	bool needSpace = false;
	stringstream ss;

	// create the string for the clipboard

	if (args.Date != 0)
	{
		ss << dateToHumanString(args.Date);
		needSpace = true;
	}

	if (args.LogType != CLog::LOG_NO)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << logTypeToString(args.LogType);
		needSpace = true;
	}

	if (!args.ProcessName.empty())
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << args.ProcessName;
		needSpace = true;
	}
	
	if (args.Filename != NULL)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << CFile::getFilename(args.Filename);
		needSpace = true;
	}

	if (args.Line != -1)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << args.Line;
		needSpace = true;
	}

	if (needSpace) { ss << ": "; needSpace = false; }

	ss << message;

	if (OpenClipboard (NULL))
	{
		HGLOBAL mem = GlobalAlloc (GHND|GMEM_DDESHARE, ss.str().size()+1);
		if (mem)
		{
			char *pmem = (char *)GlobalLock (mem);
			strcpy (pmem, ss.str().c_str());
			GlobalUnlock (mem);
			EmptyClipboard ();
			SetClipboardData (CF_TEXT, mem);
		}
		CloseClipboard ();
	}
	
	// create the string on the screen
	needSpace = false;
	stringstream ss2;

#ifdef NL_DEBUG
	if (!args.ProcessName.empty())
	{
		if (needSpace) { ss2 << " "; needSpace = false; }
		ss2 << args.ProcessName;
		needSpace = true;
	}
	
	if (args.Filename != NULL)
	{
		if (needSpace) { ss2 << " "; needSpace = false; }
		ss2 << CFile::getFilename(args.Filename);
		needSpace = true;
	}

	if (args.Line != -1)
	{
		if (needSpace) { ss2 << " "; needSpace = false; }
		ss2 << args.Line;
		needSpace = true;
	}

	if (needSpace) { ss2 << ": "; needSpace = false; }
#endif // NL_DEBUG

	ss2 << message;
	ss2 << endl << endl << "(this message was copied in the clipboard)";

/*	if (IsDebuggerPresent ())
	{
		// Must break in assert call
		DebugNeedAssert = true;
	}
	else
*/	{

		// Display the report

		string body;

		body += toString(LogTypeToString[2][args.LogType]) + "\n";
		body += "Date: " + string(dateToHumanString(args.Date)) + "\n";
		if(args.Filename == NULL)
			body += "File: <???>\n";
		else
			body += "File: " + string(args.Filename) + "\n";
		body += "Line: " + toString(args.Line) + "\n";
		body += "Reason: " + toString(message);

		body += args.CallstackAndLog;

		string subject;

		subject += args.ProcessName + " NeL " + toString(LogTypeToString[0][args.LogType]) + " " + string(args.Filename) + " " + toString(args.Line);

		// Check the envvar NEL_IGNORE_ASSERT
		if (getenv ("NEL_IGNORE_ASSERT") == NULL)
		{
			if  (ReportDebug == report (args.ProcessName + " NeL " + toString(logTypeToString(args.LogType, true)), "", subject, body, true, 2, true, 1, true, IgnoreNextTime))
			{
				DebugNeedAssert = true;
			}
		}

/*		// Check the envvar NEL_IGNORE_ASSERT
		if (getenv ("NEL_IGNORE_ASSERT") == NULL)
		{
			// Ask the user to continue, debug or ignore
			int result = MessageBox (NULL, ss2.str().c_str (), logTypeToString(args.LogType, true), MB_ABORTRETRYIGNORE | MB_ICONSTOP);
			if (result == IDABORT)
			{
				// Exit the program now
				exit (EXIT_FAILURE);
			}
			else if (result == IDRETRY)
			{
				// Give the debugger a try
				DebugNeedAssert = true;
 			}
			else if (result == IDIGNORE)
			{
				// Continue, do nothing
			}
		}
*/	}

#endif
}



/***************************************************************/
/******************* THE FOLLOWING CODE IS COMMENTED OUT *******/
/***************************************************************
void CStdDisplayer::display (const std::string& str)
{
//	printf("%s", str.c_str ());
	cout << str;

#ifdef NL_OS_WINDOWS
	// display the string in the debugger is the application is started with the debugger
	if (IsDebuggerPresent ())
		OutputDebugString(str.c_str ());
#endif
}

//****************************************************************************

void CFileDisplayer::display (const std::string& str)
{
	if (_FileName.size () == 0) return;

	ofstream ofs (_FileName.c_str (), ios::out | ios::app);
	if (ofs.is_open ())
	{
		ofs << str;
		ofs.close();
	}


//	FILE *fp = fopen (_FileName.c_str (), "a");
//	if (fp == NULL) return;

//	fprintf (fp, "%s", str.c_str ());
	
//	fclose (fp);
}


//****************************************************************************

void CMsgBoxDisplayer::display (const std::string& str)
{
#ifdef NL_OS_WINDOWS

	if (OpenClipboard (NULL))
	{
		HGLOBAL mem = GlobalAlloc (GHND|GMEM_DDESHARE, str.size()+1);
		if (mem)
		{
			char *pmem = (char *)GlobalLock (mem);
			strcpy (pmem, str.c_str());
			GlobalUnlock (mem);
			EmptyClipboard ();
			SetClipboardData (CF_TEXT, mem);
		}
		CloseClipboard ();
	}
	
	string strf = str;
	strf += "\n\n(this message was copied in the clipboard)";
	MessageBox (NULL, strf.c_str (), "", MB_OK | MB_ICONEXCLAMATION);
#endif
}
**************************************************************************/


} // NLMISC
