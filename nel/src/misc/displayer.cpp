/** \file displayer.cpp
 * Little easy displayers implementation
 *
 * $Id: displayer.cpp,v 1.11 2001/03/09 14:56:47 cado Exp $
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

#include <time.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/mutex.h"


#ifdef NL_OS_WINDOWS
// these defines is for IsDebuggerPresent(). it'll not compile on windows 95
// just comment this and the IsDebuggerPresent to compile on wiindows 95
#define _WIN32_WINDOWS	0x0410
#define WINVER			0x0400
#include <windows.h>
#endif

#include "nel/misc/displayer.h"

using namespace std;

namespace NLMISC
{

static char *LogTypeToString[][8] = {
	{ "", "ERR", "WRN", "INF", "DBG", "STT", "AST", "UKN" },
	{ "", "Error", "Warning", "Information", "Debug", "Statistic", "Assert", "Unknown" }
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


IDisplayer::IDisplayer()
{
	_Mutex = new CMutex;
}

IDisplayer::~IDisplayer()
{
	delete _Mutex;
}

/*
 * Display the string where it does.
 */
void IDisplayer::display (time_t date, CLog::TLogType logType, const std::string &processName, const char *fileName, sint line, const char *message)
{
	_Mutex->enter();
	try
	{
		doDisplay( date, logType, processName, fileName, line, message );
	}
	catch ( ... )
	{}
	_Mutex->leave();
}


// Log format : "<LogType> <FileName> <Line>: <Msg>"
void CStdDisplayer::doDisplay (time_t date, CLog::TLogType logType, const string &processName, const char *filename, sint line, const char *message)
{
	bool needSpace = false;
	stringstream ss;

	if (logType != CLog::LOG_NO)
	{
		ss << logTypeToString(logType);
		needSpace = true;
	}

	// Write thread identifier
	ss << setw(5) << getThreadId();

	if (filename != NULL)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << CFile::getFilename(filename);
		needSpace = true;
	}

	if (line != -1)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << line;
		needSpace = true;
	}
	
	if (needSpace) { ss << ": "; needSpace = false; }

	ss << message;

	cout << ss.str();

	fflush(stdout);

#ifdef NL_OS_WINDOWS
	// display the string in the debugger is the application is started with the debugger
	if (IsDebuggerPresent ())
		OutputDebugString(ss.str().c_str());
#endif
}


CFileDisplayer::CFileDisplayer(const std::string& filename, bool eraseLastLog) : _FileName(filename), _NeedHeader(true)
{
	if (eraseLastLog && !filename.empty())
	{
		ofstream ofs (filename.c_str(), ios::out | ios::trunc);
		if (!ofs.is_open())
		{
			nlwarning ("Can't open and clear the log file '%s'", filename.c_str());
		}
	}
}


// Log format: "2000/01/15 12:05:30 <LogType>: <Msg>"
void CFileDisplayer::doDisplay (time_t date, CLog::TLogType logType, const string &processName, const char *filename, sint line, const char *message)
{
	bool needSpace = false;
	stringstream ss;

	if (_FileName.empty()) return;

	if (date != 0)
	{
		ss << dateToHumanString(date);
		needSpace = true;
	}

	if (logType != CLog::LOG_NO)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << logTypeToString(logType);
		needSpace = true;
	}

	if (needSpace) { ss << ": "; needSpace = false; }

	ss << message;

	ofstream ofs (_FileName.c_str (), ios::out | ios::app);
	if (ofs.is_open ())
	{
		if (_NeedHeader)
		{
			ofs << HeaderString();
			_NeedHeader = false;
		}
		
		ofs << ss.str();
		ofs.close();
	}
}



// Log format in clipboard: "2000/01/15 12:05:30 <LogType> <ProcessName> <FileName> <Line>: <Msg>"
// Log format on the screen: in debug   "<ProcessName> <FileName> <Line>: <Msg>"
//                           in release "<Msg>"
void CMsgBoxDisplayer::doDisplay (time_t date, CLog::TLogType logType, const string &processName, const char *filename, sint line, const char *message)
{
#ifdef NL_OS_WINDOWS

	bool needSpace = false;
	stringstream ss;

	// create the string for the clipboard

	if (date != 0)
	{
		ss << dateToHumanString(date);
		needSpace = true;
	}

	if (logType != CLog::LOG_NO)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << logTypeToString(logType);
		needSpace = true;
	}

	if (!processName.empty())
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << processName;
		needSpace = true;
	}
	
	if (filename != NULL)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << CFile::getFilename(filename);
		needSpace = true;
	}

	if (line != -1)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << line;
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
	if (!processName.empty())
	{
		if (needSpace) { ss2 << " "; needSpace = false; }
		ss2 << processName;
		needSpace = true;
	}
	
	if (filename != NULL)
	{
		if (needSpace) { ss2 << " "; needSpace = false; }
		ss2 << CFile::getFilename(filename);
		needSpace = true;
	}

	if (line != -1)
	{
		if (needSpace) { ss2 << " "; needSpace = false; }
		ss2 << line;
		needSpace = true;
	}

	if (needSpace) { ss2 << ": "; needSpace = false; }
#endif // NL_DEBUG

	ss2 << message;
	ss2 << endl << endl << "(this message was copied in the clipboard)";
	MessageBox (NULL, ss2.str().c_str (), logTypeToString(logType, true), MB_OK | MB_ICONEXCLAMATION);

#endif
}



/*
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
*/


} // MKMISC
