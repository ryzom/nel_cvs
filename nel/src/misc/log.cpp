/** \file log.cpp
 * CLog class
 *
 * $Id: log.cpp,v 1.24 2001/03/07 14:56:23 cado Exp $
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

#include <stdarg.h>

#include <algorithm>
#include <vector>

#include "nel/misc/displayer.h"
#include "nel/misc/log.h"
#include "nel/misc/debug.h"

#include <stdio.h>
#include <time.h>
#include <string>
#include <sstream>

#ifdef NL_OS_WINDOWS
#include <process.h>
#else
#include <unistd.h>
#endif

using namespace std;


namespace NLMISC
{

string CLog::_ProcessName = "";

CLog::CLog( TLogType logType) : _LogType (logType), _Line(-1), _FileName(NULL)
{
}

void CLog::setProcessName (const std::string &processName)
{
	_ProcessName = processName;
}

void CLog::setPosition (sint line, char *filename)
{
    _Line = line;
	_FileName = filename;
}

void CLog::addDisplayer (IDisplayer *displayer)
{
	if (displayer == NULL)
	{
		nlwarning ("Trying to add a NULL displayer");
		return;
	}

	CDisplayers::iterator idi = std::find (_Displayers.begin (), _Displayers.end (), displayer);
	if (idi == _Displayers.end ())
	{
		_Displayers.push_back (displayer);
	}
	else
	{
		nlwarning ("Couldn't add the displayer, it was already added");
	}
}

void CLog::removeDisplayer (IDisplayer *displayer)
{
	if (displayer == NULL)
	{
		nlwarning ("Trying to remove a NULL displayer");
		return;
	}

	CDisplayers::iterator idi = std::find (_Displayers.begin (), _Displayers.end (), displayer);
	if (idi == _Displayers.end ())
	{
		nlwarning ("Couldn't remove the displayer, it isn't in the list");
	}
	else
	{
		_Displayers.erase (idi);
	}
}


/*
 * Returns true if the specified displayer is attached to the log object
 */
bool CLog::attached(IDisplayer *displayer) const 
{
	return (find( _Displayers.begin(), _Displayers.end(), displayer ) != _Displayers.end());
}


/*
 * Display the string with decoration and final new line to all attached displayers
 */
void CLog::displayNL (const char *format, ...)
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

	if (strlen(str)<NLMISC::MaxCStringSize-1)
		strcat (str, "\n");
	else
		str[NLMISC::MaxCStringSize-2] = '\n';

	display (str);
}

/*
 * Display the string with decoration to all attached displayers
 */
void CLog::display (const char *format, ...)
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

	time_t date;
	time (&date);

	// Send to the attached displayers
	for (CDisplayers::iterator idi=_Displayers.begin(); idi!=_Displayers.end(); idi++ )
	{
		(*idi)->display (date, _LogType, _ProcessName, _FileName, _Line, str);
	}

	setPosition (-1, NULL);	

/*
	char cstime[25];
	strftime( cstime, 25, "%y/%m/%d %H:%M:%S", localtime( &t ) );

	stringstream ss;
	if ( _Long )
	{
		ss << cstime << " ";
	}
	ss << priorityStr().c_str();
	if ( _Long && _LocalHostAndService != NULL && _LocalHostAndService->size() != 0)
	{
		ss << " " << _LocalHostAndService->c_str();
	}
	if ( _File != NULL )
	{
		ss << " " << getFilename(_File) << " " << _Line;
	}
	ss << ": " << str ;//<< endl;

	// Send to the attached displayers
	for ( CDisplayers::iterator idi=_Displayers.begin(); idi!=_Displayers.end(); idi++ )
	{
		(*idi)->display (t, _LogType, _ProcessName, _FileName, _Line, str);
	}

	_File = NULL;
	_Line = 0;
*/}


/*
 * Display a string (and nothing more) to all attached displayers
 */
void CLog::displayRawNL( const char *format, ... )
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

	if (strlen(str)<NLMISC::MaxCStringSize-1)
		strcat (str, "\n");
	else
		str[NLMISC::MaxCStringSize-2] = '\n';

	displayRaw(str);
}

/*
 * Display a string (and nothing more) to all attached displayers
 */
void CLog::displayRaw( const char *format, ... )
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

	// Send to the attached displayers
	for ( CDisplayers::iterator idi=_Displayers.begin(); idi<_Displayers.end(); idi++ )
	{
		(*idi)->display (0, LOG_NO, "", NULL, -1, str);
	}
}





////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
#if 0
string *CLog::_LocalHostAndService = NULL;

/*
 * Sets the local host name, with has to be determined outside
 */
void CLog::setLocalHostAndService( const std::string& hostname, const std::string& servicename )
{
	stringstream ss;
	ss << servicename.c_str() << "/" << hostname.c_str() << "/" << getpid();
	if (_LocalHostAndService == NULL)
		_LocalHostAndService = new string;

	if (hostname.size()==0 && servicename.size() == 0)
		*_LocalHostAndService = "";
	else
		*_LocalHostAndService = ss.str();
}


CLog::CLog( TLogPriority priority, bool longinfo ) :
	_Priority( priority ),
	_Line( 0 ),
	_File( NULL ),
	_Long( longinfo )
{
	// by default service and host or NULL so they ll not display
}


void CLog::addDisplayer (IDisplayer *displayer)
{
	if (displayer == NULL) return;

	CDisplayers::iterator idi = std::find (_Displayers.begin (), _Displayers.end (), displayer);
	if (idi == _Displayers.end ())
	{
		_Displayers.push_back (displayer);
	}
}

void CLog::removeDisplayer (IDisplayer *displayer)
{
	if (displayer == NULL) return;

	CDisplayers::iterator idi = std::find (_Displayers.begin (), _Displayers.end (), displayer);
	if (idi == _Displayers.end ())
	{
		/// Displayer not found
	}
	else
	{
		_Displayers.erase (idi);
	}
}


/*
 * Returns true if the specified displayer is attached to the log object
 */
bool CLog::attached(IDisplayer *displayer) const 
{
	return ( find( _Displayers.begin(), _Displayers.end(), displayer ) != _Displayers.end() );
}


/*
 * Returns a pointer the filename, skipping the directory
 */
char *getFilename( char *lfilename )
{
#ifdef NL_OS_WINDOWS
	char *slash = strrchr( lfilename, '\\' );
#else
	char *slash = strrchr( lfilename, '/' );
#endif
	if ( slash == NULL )
	{
		return lfilename;
	}
	else
	{
		return slash+1;
	}
}

/*
 * Display the string with decoration and final new line to all attached displayers
 */
void CLog::displayNL( const char *format, ... )
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format);

	strcat (str, "\n");

	display (str);
}

/*
 * Display the string with decoration to all attached displayers
 */
void CLog::display( const char *format, ... )
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format);

	time_t t;
	time( &t );
	
	char cstime[25];
	strftime( cstime, 25, "%y/%m/%d %H:%M:%S", localtime( &t ) );

	stringstream ss;
	if ( _Long )
	{
		ss << cstime << " ";
	}
	ss << priorityStr().c_str();
	if ( _Long && _LocalHostAndService != NULL && _LocalHostAndService->size() != 0)
	{
		ss << " " << _LocalHostAndService->c_str();
	}
	if ( _File != NULL )
	{
		ss << " " << getFilename(_File) << " " << _Line;
	}
	ss << ": " << str ;//<< endl;

	// Send to the attached displayers

	string s = ss.str();
	for ( CDisplayers::iterator idi=_Displayers.begin(); idi!=_Displayers.end(); idi++ )
	{
		(*idi)->display( s );
	}

	_File = NULL;
	_Line = 0;
}


/*
 * Display a string (and nothing more) to all attached displayers
 */
void CLog::displayRawNL( const char *format, ... )
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format);

	strcat (str, "\n");

	// Send to the attached displayers
	for ( CDisplayers::iterator idi=_Displayers.begin(); idi<_Displayers.end(); idi++ )
	{
		(*idi)->display( str );
	}
}

/*
 * Display a string (and nothing more) to all attached displayers
 */
void CLog::displayRaw( const char *format, ... )
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format);

	// Send to the attached displayers
	for ( CDisplayers::iterator idi=_Displayers.begin(); idi<_Displayers.end(); idi++ )
	{
		(*idi)->display( str );
	}
}


string CLog::priorityStr() const
{
	switch ( _Priority )
	{
	case LOG_NO : return "";
	case LOG_DEBUG : return "DBG";
	case LOG_WARNING : return "WRN";
	case LOG_INFO : return "INF";
	case LOG_ERROR : return "ERR";
	case LOG_STAT : return "STT";
	case LOG_ASSERT : return "AST";
	default: nlstop; return "<Unknown>";
	}
}
#endif

} // NLMISC

