/** \file debug.cpp
 * This file contains all features that help us to debug applications
 *
 * $Id: debug.cpp,v 1.29 2001/02/16 11:35:54 lecroart Exp $
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


namespace NLMISC {

CLog ErrorLog (CLog::LOG_ERROR);
CLog WarningLog (CLog::LOG_WARNING);
CLog InfoLog (CLog::LOG_INFO);
CLog DebugLog (CLog::LOG_DEBUG);
CLog AssertLog (CLog::LOG_ASSERT);

CStdDisplayer sd;
CMsgBoxDisplayer mbd;


void nlFatalError (const char *format, ...)
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

	NLMISC::ErrorLog.displayNL (str);

#if defined(NL_OS_WINDOWS) && defined (NL_DEBUG)
	_asm int 3;
#endif

	throw EFatalError();
}

void nlError (const char *format, ...)
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

	NLMISC::ErrorLog.displayNL (str);

#if defined(NL_OS_WINDOWS) && defined (NL_DEBUG)
	_asm int 3;
#endif
}

void initDebug (bool setDisplayerInReleaseModeToo)
{
	static bool alreadyInit = false;

	if (!alreadyInit)
	{
#ifdef NL_DEBUG

		ErrorLog.addDisplayer (&sd);
		WarningLog.addDisplayer (&sd);
		InfoLog.addDisplayer (&sd);
		DebugLog.addDisplayer (&sd);
		AssertLog.addDisplayer (&sd);

#elif defined(NL_RELEASE)

		if (setDisplayerInReleaseModeToo)
		{
			InfoLog.addDisplayer (&sd);
			WarningLog.addDisplayer (&sd);
			ErrorLog.addDisplayer (&sd);
			ErrorLog.addDisplayer (&mbd);
		}

#endif // NL_RELEASE

		alreadyInit = true;
	}
	else
	{
		nlwarning ("NLMISC::initDebug() already called");
	}
}


} // NLMISC
