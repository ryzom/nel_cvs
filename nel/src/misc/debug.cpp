/* debug.cpp
 *
 * Copyright, 2000 Nevrax Ltd.
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

/*
 * $Id: debug.cpp,v 1.10 2000/10/10 16:07:39 cado Exp $
 *
 * Debug
 */

#include "nel/misc/debug.h"
#include "nel/misc/log.h"
#include "nel/misc/displayer.h"

/**
 * \page log_howto How to log information ?
 * This document explains how to log some information (into the screen, into a file or to a logging server) , e.g. in order to debug your code.
 *
 * \subsection init_log Initialization
 * In the initialization of your service, attach some displayers to the global NLMISC::CLog objects
 * \e ErrorLog, \e WarningLog, \e InfoLog, \e DebugLog and \e AssertLog.
 * NLMISC::CStdDisplayer is for the screen (and VC++ debug window). It is attached by default to all of the five logger objects mentionned above.
 * NLMISC::CFileDisplayer is for a file.
 * NLMISC::CMsgBoxDisplayer is for a message box.
 * NLNET::CNetDisplayer is for a logging server (see CLogService in the server documentation)
 *
 * Example :
 * \code
 * NLNET::CNetDisplayer nd ( NLNET::CInetAddress( "log_service_machine", log_port ) )
 * NLMISC::DebugLog.addDisplayer( &nd );
 * \endcode
 *
 * \subsection use_log Logging information
 * In your code, use the macros :  \e nlerror, \e nlwarning, \e nlinfo, \e nldebug with a variable number of arguments.
 * You have to include the header "nel/misc/debug.h".
 *
 * Example :
 * \code
 * nldebug( "Toto is %d years old", age );
 * if ( age < 0 )
 * {
 *     nlerror( "Invalid age for toto : %d", age );
 *     return -1;
 * }
 * \endcode
 *
 * How to log string without repeating the header ?
 *
 * The macros nldebug and nlerror call a few methods of NLMISC::CLog, including displayNL().
 * It prints a string with an information header.
 * If you don't want to print the header, use the others methods of NLMISC::CLog.
 *
 * Example :
 * \code
 * NLMISC::DebugLog.displayNL ( "Dump of Values :" );
 * for ( int j=0; j!=height; ++j )
 * {
 *     for ( int i=0; i!=width; ++i )
 *     {
 *         NLMISC::DebugLog.displayRaw( "%d ", Values[j][i] );
 *     }
 *     NLMISC::DebugLog.displayRawNL( ": line %d", j );
 * }
 * \endcode
 */

 
#include "nel/misc/debug.h"


namespace NLMISC {

CLog ErrorLog( LOG_ERROR, true );
CLog WarningLog( LOG_WARNING, true );
CLog InfoLog( LOG_INFO, true );
CLog DebugLog( LOG_DEBUG, false );
CLog AssertLog( LOG_ASSERT, true );

CStdDisplayer sd;

void NLMISC_InitDebug ()
{
	static int alreadyInit = false;

	if (!alreadyInit)
	{
		ErrorLog.addDisplayer (&sd);
		WarningLog.addDisplayer (&sd);
		InfoLog.addDisplayer (&sd);
		DebugLog.addDisplayer (&sd);
		AssertLog.addDisplayer (&sd);
		alreadyInit = true;
	}
	else
	{
		nlwarning ("RMISC_InitDebug already called");
	}
}


} // NLMISC
