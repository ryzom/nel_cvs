/* log.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: log.cpp,v 1.7 2000/10/11 08:31:07 lecroart Exp $
 *
 * Implementation for CLog
 */

#include <stdarg.h>

#include <algorithm>
#include <vector>

#include "nel/misc/displayer.h"
#include "nel/misc/log.h"
#include "nel/misc/debug.h"

#include <stdio.h>
#include <time.h>
#include <sstream>
using namespace std;

#include <string>

namespace NLMISC
{

string CLog::_LocalHostAndService = "<UnknownHost> <UnknownService> ";

CLog::CLog( TLogPriority priority, bool longinfo ) :
	_Priority( priority ),
	_Line( 0 ),
	_File( NULL ),
	_Long( longinfo )
{
}


void CLog::addDisplayer (IDisplayer *displayer)
{
	if (displayer == NULL) return;

	std::vector<IDisplayer *>::iterator idi = std::find (_Displayers.begin (), _Displayers.end (), displayer);
	if (idi == _Displayers.end ())
	{
		_Displayers.push_back (displayer);
	}
}

void CLog::removeDisplayer (IDisplayer *displayer)
{
	if (displayer == NULL) return;

	std::vector<IDisplayer *>::iterator idi = std::find (_Displayers.begin (), _Displayers.end (), displayer);
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
 * Returns a pointer the filename, skipping the directory
 */
char *getFilename( char *lfilename )
{
#ifdef NL_OS_WINDOWS
	char *slash = strrchr( lfilename, '\\' );
#else
	char *slash = sttrchr( lfilename, '/' );
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
	// Build the string
	char cstring [1024];

	va_list args;
	va_start( args, format );
	vsprintf( cstring, format, args );
	va_end( args );
	strcat( cstring, "\n" );

	display (cstring);
}

/*
 * Display the string with decoration to all attached displayers
 */
void CLog::display( const char *format, ... )
{
	// Build the string
	char cstring [1024];

	va_list args;
	va_start( args, format );
	vsprintf( cstring, format, args );
	va_end( args );

	time_t t;
	time( &t );
	
	char cstime[25];
	strftime( cstime, 25, "%y/%m/%d %H:%M:%S", localtime( &t ) );

	stringstream ss;
	if ( _Long )
	{
		ss << cstime << " ";
	}
	ss << priorityStr().c_str() << " ";
	if ( _Long )
	{
		ss << _LocalHostAndService.c_str();
	}
	if ( _File != NULL )
	{
		ss << getFilename(_File) << " " << _Line << " ";
	}
	ss << ": " << cstring ;//<< endl;
	string s = ss.str();

	// Send to the attached displayers

	for ( CDisplayers::iterator idi=_Displayers.begin(); idi<_Displayers.end(); idi++ )
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
	// Build the string
	char cstring [1024];
	va_list args;
	va_start( args, format );
	vsprintf( cstring, format, args );
	va_end( args );

	strcat ( cstring, "\n" );

	// Send to the attached displayers
	for ( CDisplayers::iterator idi=_Displayers.begin(); idi<_Displayers.end(); idi++ )
	{
		(*idi)->display( cstring );
	}
}

/*
 * Display a string (and nothing more) to all attached displayers
 */
void CLog::displayRaw( const char *format, ... )
{
	// Build the string
	char cstring [1024];
	va_list args;
	va_start( args, format );
	vsprintf( cstring, format, args );
	va_end( args );

	// Send to the attached displayers
	for ( CDisplayers::iterator idi=_Displayers.begin(); idi<_Displayers.end(); idi++ )
	{
		(*idi)->display( cstring );
	}
}


string CLog::priorityStr() const
{
	switch ( _Priority )
	{
		case LOG_DEBUG : return "DBG";
		case LOG_WARNING : return "WRN";
		case LOG_INFO : return "INF";
		case LOG_ERROR : return "ERR";
		case LOG_STAT : return "STT";
		case LOG_ASSERT : return "AST";
		default: nlstop; return "<Unknown>";
	}
}


} // NLMISC