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
 * $Id: debug.cpp,v 1.4 2000/10/04 15:27:53 cado Exp $
 *
 * <Replace this by a description of the file>
 */

#include "nel/misc/debug.h"

/**
 * \page log_howto How to log information ?
 * This document explains how to log some information (into the screen, into a file or to a logging server) , e.g. in order to debug your code.
 *
 * \subsection init_log Initialization
 * In the initialization of your service, attach some displayers to the global CLog objects
 * \e ErrorLog, \e WarningLog, \e InfoLog, \e DebugLog.
 * CStdDisplayer is for the screen (and VC++ debug window).
 * CFileDisplayer is for a file.
 * CMsgBoxDisplayer is for a message box.
 * CNetDisplayer is for a logging server (see CLogService)
 *
 * Example :
 * \code
 * CStdDisplayer sd;
 * CNetDisplayer nd ( CInetAddress( "log_service_machine", log_port ) )
 * ErrorLog.addDisplayer( &sd );
 * DebugLog.addDisplayer( &nd );
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
 */


namespace NLMISC {

CLog ErrorLog( LOG_ERROR, true );
CLog WarningLog( LOG_WARNING, true );
CLog InfoLog( LOG_INFO, true );
CLog DebugLog( LOG_DEBUG, false );
CLog StatLog( LOG_STAT, true );

} // NLMISC
