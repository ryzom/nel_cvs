/** \file debug.cpp
 * This file contains all features that help us to debug applications
 *
 * $Id: debug.cpp,v 1.47 2002/03/28 17:44:38 lecroart Exp $
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

#ifdef HAVE_CONFIG_H
#include "nelconfig.h"
#endif // HAVE_CONFIG_H

#include "stdmisc.h"
#include "nel/misc/log.h"
#include "nel/misc/displayer.h"
#include "nel/misc/mem_displayer.h"
#include "nel/misc/command.h"

#ifdef NL_OS_WINDOWS
#include <windows.h>
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

namespace NLMISC {

CLog *ErrorLog = NULL;
CLog *WarningLog = NULL;
CLog *InfoLog = NULL;
CLog *DebugLog = NULL;
CLog *AssertLog = NULL;

CMemDisplayer *DefaultMemDisplayer = NULL;

static CStdDisplayer *sd = NULL;
static CFileDisplayer *fd = NULL;
static CMsgBoxDisplayer *mbd = NULL;

void nlFatalError (const char *format, ...)
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

	NLMISC::ErrorLog->displayNL (str);

	// write the memory displayer into file
	DefaultMemDisplayer->write ();

#if defined(NL_OS_WINDOWS) && defined (NL_DEBUG)
	_asm int 3;
#endif

	throw EFatalError();
}

void nlError (const char *format, ...)
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

	NLMISC::ErrorLog->displayNL (str);

	// write the memory displayer into file
	DefaultMemDisplayer->write ();

#if defined(NL_OS_WINDOWS) && defined (NL_DEBUG)
	_asm int 3;
#endif
}

// the default behavior is to display all in standard output and to a file named "log.log";

void initDebug2 ()
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
#ifdef NL_DEBUG
		DebugLog->addDisplayer (fd);
#endif // NL_DEBUG
		InfoLog->addDisplayer (fd);
		WarningLog->addDisplayer (fd);
		AssertLog->addDisplayer (fd);
		ErrorLog->addDisplayer (fd);
#endif // LOG_IN_FILE

		// put the message box only in release for error

#ifdef NL_RELEASE
		ErrorLog->addDisplayer (mbd);
#endif // NL_RELEASE

#endif // DEFAULT_DISPLAYER
		alreadyInit = true;
	}
	else
	{
		nlwarning ("NLMISC::initDebug2() already called");
	}
}

void createDebug (const char *logPath)
{
	static bool alreadyCreate = false;
	if (!alreadyCreate)
	{
		// Debug Info for mutexes
#ifdef MUTEX_DEBUG
		initAcquireTimeMap();
#endif

		ErrorLog = new CLog (CLog::LOG_ERROR);
		WarningLog = new CLog (CLog::LOG_WARNING);
		InfoLog = new CLog (CLog::LOG_INFO);
		DebugLog = new CLog (CLog::LOG_DEBUG);
		AssertLog = new CLog (CLog::LOG_ASSERT);

		sd = new CStdDisplayer ("DEFAULT_SD");
		mbd = new CMsgBoxDisplayer ("DEFAULT_MBD");
#if LOG_IN_FILE
		string fn;
		if (logPath != NULL)
		{
			fn += logPath;
		}
		fn += "log.log";
		fd = new CFileDisplayer (fn, false, "DEFAULT_FD");
#endif // LOG_IN_FILE
		DefaultMemDisplayer = new CMemDisplayer ("DEFAULT_MD");
		
		initDebug2();

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

NLMISC_COMMAND(displayFilterInfo, "display filter on InfoLog", "")
{
	if(args.size() != 0) return false;
	InfoLog->displayFilter(log);
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
