/** \file log.cpp
 * CLog class
 *
 * $Id: log.cpp,v 1.36 2002/03/14 13:49:27 lecroart Exp $
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

#ifdef NL_OS_WINDOWS
#include <process.h>
#else
#include <unistd.h>
#endif

#include <time.h>
#include <stdarg.h>

#include "nel/misc/displayer.h"
#include "nel/misc/log.h"

using namespace std;


namespace NLMISC
{

string CLog::_ProcessName = "";

CLog::CLog( TLogType logType) : _LogType (logType), _Line(-1), _FileName(NULL), _PosSet(false), _Mutex("LOG"+toString((uint)logType))
{
}

void CLog::setProcessName (const std::string &processName)
{
	_ProcessName = processName;
}

void CLog::setPosition (sint line, char *filename)
{
	if ( !noDisplayer() )
	{
		_Mutex.enter();
		_PosSet++;
	    _Line = line;
		_FileName = filename;
	}
}

/// Symetric to setPosition(). Automatically called by display...(). Do not call if noDisplayer().
void CLog::unsetPosition()
{
	nlassert( !noDisplayer() );

	if ( _PosSet > 0 )
	{
		_FileName = NULL;
		_Line = -1;
		_PosSet--;
		_Mutex.leave(); // needs setPosition() to have been called
	}
}


void CLog::addDisplayer (IDisplayer *displayer, bool bypassFilter)
{
	if (displayer == NULL)
	{
		nlwarning ("Trying to add a NULL displayer");
		return;
	}

	if (bypassFilter)
	{
		CDisplayers::iterator idi = std::find (_BypassFilterDisplayers.begin (), _BypassFilterDisplayers.end (), displayer);
		if (idi == _BypassFilterDisplayers.end ())
		{
			_BypassFilterDisplayers.push_back (displayer);
		}
		else
		{
			nlwarning ("Couldn't add the displayer, it was already added");
		}
	}
	else
	{
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
}

void CLog::removeDisplayer (IDisplayer *displayer)
{
	if (displayer == NULL)
	{
		nlwarning ("Trying to remove a NULL displayer");
		return;
	}

	CDisplayers::iterator idi = std::find (_Displayers.begin (), _Displayers.end (), displayer);
	if (idi != _Displayers.end ())
	{
		_Displayers.erase (idi);
	}

	idi = std::find (_BypassFilterDisplayers.begin (), _BypassFilterDisplayers.end (), displayer);
	if (idi != _BypassFilterDisplayers.end ())
	{
		_BypassFilterDisplayers.erase (idi);
	}

}

void CLog::removeDisplayer (const char *displayerName)
{
	if (displayerName == NULL || displayerName[0] == '\0')
	{
		nlwarning ("Trying to remove an empty displayer name");
		return;
	}

	CDisplayers::iterator idi;
	for (idi = _Displayers.begin (); idi != _Displayers.end ();)
	{
		if ((*idi)->DisplayerName == displayerName)
		{
			idi = _Displayers.erase (idi);
		}
		else
		{
			idi++;
		}
	}

	for (idi = _BypassFilterDisplayers.begin (); idi != _BypassFilterDisplayers.end ();)
	{
		if ((*idi)->DisplayerName == displayerName)
		{
			idi = _BypassFilterDisplayers.erase (idi);
		}
		else
		{
			idi++;
		}
	}
}

IDisplayer *CLog::getDisplayer (const char *displayerName)
{
	if (displayerName == NULL || displayerName[0] == '\0')
	{
		nlwarning ("Trying to get an empty displayer name");
		return NULL;
	}

	CDisplayers::iterator idi;
	for (idi = _Displayers.begin (); idi != _Displayers.end (); idi++)
	{
		if ((*idi)->DisplayerName == displayerName)
		{
			return *idi;
		}
	}
	for (idi = _BypassFilterDisplayers.begin (); idi != _BypassFilterDisplayers.end (); idi++)
	{
		if ((*idi)->DisplayerName == displayerName)
		{
			return *idi;
		}
	}
	return NULL;
}

/*
 * Returns true if the specified displayer is attached to the log object
 */
bool CLog::attached(IDisplayer *displayer) const 
{
	return (find( _Displayers.begin(), _Displayers.end(), displayer ) != _Displayers.end()) ||
			(find( _BypassFilterDisplayers.begin(), _BypassFilterDisplayers.end(), displayer ) != _BypassFilterDisplayers.end());
}


/*
 * Display the string with decoration and final new line to all attached displayers
 */
void CLog::displayNL (const char *format, ...)
{
	if ( noDisplayer() )
	{
		return;
	}
	
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
	if ( noDisplayer() )
	{
		return;
	}

	char *str;
	NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

	TDisplayInfo args;
	time (&args.Date);
	args.LogType = _LogType;
	args.ProcessName = _ProcessName;
	args.ThreadId = getThreadId();
	args.Filename = _FileName;
	args.Line = _Line;

	IDisplayer *id = NULL;

	// send to all bypass filter displayers
	for (CDisplayers::iterator idi=_BypassFilterDisplayers.begin(); idi!=_BypassFilterDisplayers.end(); idi++ )
	{
		id = *idi;
		(*idi)->display( args, str );
	}

	if (passFilter (str))
	{
		// Send to the attached displayers
		for (CDisplayers::iterator idi=_Displayers.begin(); idi!=_Displayers.end(); idi++ )
		{
			id = *idi;
			(*idi)->display( args, str );
		}
	}

	unsetPosition();

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
*/
}


/*
 * Display a string (and nothing more) to all attached displayers
 */
void CLog::displayRawNL( const char *format, ... )
{
	if ( noDisplayer() )
	{
		return;
	}

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
	if ( noDisplayer() )
	{
		return;
	}

	char *str;
	NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

	TDisplayInfo args;
	args.Date = 0;
	args.LogType = LOG_NO;
	args.ProcessName = "";
	args.ThreadId = 0;
	args.Filename = NULL;
	args.Line = -1;

	// send to all bypass filter displayers
	for (CDisplayers::iterator idi=_BypassFilterDisplayers.begin(); idi!=_BypassFilterDisplayers.end(); idi++ )
	{
		(*idi)->display( args, str );
	}

	if ( passFilter( str ) )
	{
		// Send to the attached displayers
		for ( CDisplayers::iterator idi=_Displayers.begin(); idi!=_Displayers.end(); idi++ )
		{
			(*idi)->display( args, str );
		}
	}

	unsetPosition();
}


void CLog::forceDisplayRaw (const char *format, ...)
{
	if ( noDisplayer() )
	{
		return;
	}

	char *str;
	NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

	TDisplayInfo args;

	// Send to the attached displayers
	for ( CDisplayers::iterator idi=_Displayers.begin(); idi!=_Displayers.end(); idi++ )
	{
		(*idi)->display( args, str );
	}
}



/*
 * Returns true if the string must be logged, according to the current filter
 */
bool CLog::passFilter( const char *filter )
{
	bool yes = _PositiveFilter.empty();

	bool found;
	list<string>::iterator ilf;

	// 1. Positive filter
	for ( ilf=_PositiveFilter.begin(); ilf!=_PositiveFilter.end(); ++ilf )
	{
		found = ( strstr( filter, (*ilf).c_str() ) != NULL );
		if ( found )
		{
			yes = true; // positive filter passed (no need to check another one)
			break;
		}
		// else try the next one
	}
	if ( ! yes )
	{
		return false; // positive filter not passed
	}

	// 2. Negative filter
	for ( ilf=_NegativeFilter.begin(); ilf!=_NegativeFilter.end(); ++ilf )
	{
		found = ( strstr( filter, (*ilf).c_str() ) != NULL );
		if ( found )
		{
			return false; // negative filter not passed (no need to check another one)
		}
	}
	return true; // negative filter passed
}


/*
 * Removes a filter by name. Returns true if it was found.
 */
void CLog::removeFilter( const char *filterstr )
{
	_PositiveFilter.remove( filterstr );
	_NegativeFilter.remove( filterstr );
	displayNL ("CLog::removeFilter('%s')", filterstr);
}



void CLog::addPositiveFilter( const char *filterstr )
{
	_PositiveFilter.push_back( filterstr );
	displayNL ("CLog::addPositiveFilter('%s')", filterstr);
}

void CLog::addNegativeFilter( const char *filterstr )
{
	displayNL ("CLog::addNegativeFilter('%s')", filterstr);
	_NegativeFilter.push_back( filterstr );
}

void CLog::resetFilters()
{
	_PositiveFilter.clear(); _NegativeFilter.clear();
	displayNL ("CLog::resetFilter()");
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
#endif // 0

} // NLMISC

