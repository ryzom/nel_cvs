/** \file debug.cpp
 * This file contains all features that help us to debug applications
 *
 * $Id: debug.cpp,v 1.26 2001/01/30 13:44:16 lecroart Exp $
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

CLog ErrorLog( LOG_ERROR, true );
CLog WarningLog( LOG_WARNING, true );
CLog InfoLog( LOG_INFO, true );
CLog DebugLog( LOG_DEBUG, false );
CLog AssertLog( LOG_ASSERT, true );

CStdDisplayer sd;


void nlFatalError (const char *format, ...)
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format);

	NLMISC::ErrorLog.displayNL (str);

#if defined(NL_OS_WINDOWS) && defined (NL_DEBUG)
	_asm int 3;
#endif

	throw EFatalError();
}

void nlError (const char *format, ...)
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format);

	NLMISC::ErrorLog.displayNL (str);

#if defined(NL_OS_WINDOWS) && defined (NL_DEBUG)
	_asm int 3;
#endif
}

void InitDebug ()
{
	static int alreadyInit = false;

	if (!alreadyInit)
	{
		InfoLog.setLongInfo(false);
#ifdef NL_DEBUG
		ErrorLog.addDisplayer (&sd);
		WarningLog.addDisplayer (&sd);
		InfoLog.addDisplayer (&sd);
		DebugLog.addDisplayer (&sd);
		AssertLog.addDisplayer (&sd);
#endif
		alreadyInit = true;
	}
	else
	{
		nlwarning ("RMISC_InitDebug already called");
	}
}


} // NLMISC
