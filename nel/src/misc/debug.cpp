/** \file debug.cpp
 * This file contains all features that help us to debug applications
 *
 * $Id: debug.cpp,v 1.40 2001/09/10 16:09:19 lecroart Exp $
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

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/log.h"
#include "nel/misc/displayer.h"


#ifdef NL_OS_WINDOWS
#include <windows.h>
#endif

#include <stdarg.h>
#include <stdio.h>
 
#include "nel/misc/debug.h"

// If you don't want to add default displayer, put 0 instead of 1. In this case, you
// have to manage yourself displayer (in final release for example, we have to put 0)
#define DEFAULT_DISPLAYER 1

// Put 0 if you don't want to display in file "log.log"
#define LOG_IN_FILE 1

namespace NLMISC {

CLog *ErrorLog = NULL;
CLog *WarningLog = NULL;
CLog *InfoLog = NULL;
CLog *DebugLog = NULL;
CLog *AssertLog = NULL;

static CStdDisplayer *sd = NULL;
static CFileDisplayer *fd = NULL;
static CMsgBoxDisplayer *mbd = NULL;

void nlFatalError (const char *format, ...)
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

	NLMISC::ErrorLog->displayNL (str);

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
		InfoLog->addDisplayer (sd);
		WarningLog->addDisplayer (sd);
		ErrorLog->addDisplayer (sd);
#if LOG_IN_FILE
		InfoLog->addDisplayer (fd);
		WarningLog->addDisplayer (fd);
		ErrorLog->addDisplayer (fd);
#endif // LOG_IN_FILE

#ifdef NL_DEBUG
		DebugLog->addDisplayer (sd);
#endif // NL_DEBUG
		AssertLog->addDisplayer (sd);

#if LOG_IN_FILE

#ifdef NL_DEBUG
		DebugLog->addDisplayer (fd);
#endif // NL_DEBUG
		AssertLog->addDisplayer (fd);

#endif // LOG_IN_FILE

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

void createDebug ()
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
		fd = new CFileDisplayer ("log.log", false, "DEFAULT_FD");
		
		initDebug2();

		alreadyCreate = true;
	}
}

} // NLMISC
