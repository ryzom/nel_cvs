/** \file log.cpp
 * CLog class
 *
 * $Id: log.cpp,v 1.47 2002/10/28 17:32:13 corvazier Exp $
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
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <time.h>
#include <stdarg.h>

#include "nel/misc/displayer.h"
#include "nel/misc/log.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"

using namespace std;


namespace NLMISC
{

string CLog::_ProcessName = "";

CLog::CLog( TLogType logType) : _LogType (logType), _Line(-1), _FileName(NULL), _Mutex("LOG"+toString((uint)logType)), _PosSet(false)
{
}

void CLog::setDefaultProcessName ()
{
#ifdef NL_OS_WINDOWS
	if (_ProcessName.empty())
	{
		char name[1024];
		GetModuleFileName (NULL, name, 1023);
		_ProcessName = CFile::getFilename(name);
	}
#endif
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

static string TempString;
static TDisplayInfo TempArgs;


void CLog::displayString (const char *str)
{
	const char *disp = NULL;
	TDisplayInfo localargs, *args = NULL;

	setDefaultProcessName ();

	if(strchr(str,'\n') == NULL)
	{
		if (TempString.empty())
		{
			time (&TempArgs.Date);
			TempArgs.LogType = _LogType;
			TempArgs.ProcessName = _ProcessName;
			TempArgs.ThreadId = getThreadId();
			TempArgs.Filename = _FileName;
			TempArgs.Line = _Line;
			TempArgs.CallstackAndLog = "";

			TempString = str;
		}
		else
		{
			TempString += str;
		}
		return;
	}
	else
	{
		if (TempString.empty())
		{
			time (&localargs.Date);
			localargs.LogType = _LogType;
			localargs.ProcessName = _ProcessName;
			localargs.ThreadId = getThreadId();
			localargs.Filename = _FileName;
			localargs.Line = _Line;
			localargs.CallstackAndLog = "";

			disp = str;
			args = &localargs;
		}
		else
		{
			TempString += str;
			disp = TempString.c_str();
			args = &TempArgs;
		}
	}

	// send to all bypass filter displayers
	for (CDisplayers::iterator idi=_BypassFilterDisplayers.begin(); idi!=_BypassFilterDisplayers.end(); idi++ )
	{
		(*idi)->display( *args, disp );
	}

	// get the log at the last minute to be sure to have everything
	if(args->LogType == LOG_ERROR || args->LogType == LOG_ASSERT)
	{
		getCallStackAndLog (args->CallstackAndLog, 4);
	}

	if (passFilter (disp))
	{
		// Send to the attached displayers
		for (CDisplayers::iterator idi=_Displayers.begin(); idi!=_Displayers.end(); idi++ )
		{
			(*idi)->display( *args, disp );
		}
	}
	TempString = "";
	unsetPosition();
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
	NLMISC_CONVERT_VARGS (str, format, 256/*NLMISC::MaxCStringSize*/);

	if (strlen(str)<256/*NLMISC::MaxCStringSize*/-1)
		strcat (str, "\n");
	else
		str[256/*NLMISC::MaxCStringSize*/-2] = '\n';

	displayString (str);
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
	NLMISC_CONVERT_VARGS (str, format, 256/*NLMISC::MaxCStringSize*/);

	displayString (str);
}


void CLog::displayRawString (const char *str)
{
	const char *disp = NULL;
	TDisplayInfo localargs, *args = NULL;

	setDefaultProcessName ();

	if(strchr(str,'\n') == NULL)
	{
		if (TempString.empty())
		{
			localargs.Date = 0;
			localargs.LogType = CLog::LOG_NO;
			localargs.ProcessName = "";
			localargs.ThreadId = 0;
			localargs.Filename = NULL;
			localargs.Line = -1;
			localargs.CallstackAndLog = "";

			TempString = str;
		}
		else
		{
			TempString += str;
		}
		return;
	}
	else
	{
		if (TempString.empty())
		{
			localargs.Date = 0;
			localargs.LogType = CLog::LOG_NO;
			localargs.ProcessName = "";
			localargs.ThreadId = 0;
			localargs.Filename = NULL;
			localargs.Line = -1;
			localargs.CallstackAndLog = "";

			disp = str;
			args = &localargs;
		}
		else
		{
			TempString += str;
			disp = TempString.c_str();
			args = &TempArgs;
		}
	}

	// send to all bypass filter displayers
	for (CDisplayers::iterator idi=_BypassFilterDisplayers.begin(); idi!=_BypassFilterDisplayers.end(); idi++ )
	{
		(*idi)->display( *args, disp );
	}

	// get the log at the last minute to be sure to have everything
	if(args->LogType == LOG_ERROR || args->LogType == LOG_ASSERT)
	{
		getCallStackAndLog (args->CallstackAndLog, 4);
	}

	if ( passFilter( disp ) )
	{
		// Send to the attached displayers
		for ( CDisplayers::iterator idi=_Displayers.begin(); idi!=_Displayers.end(); idi++ )
		{
			(*idi)->display( *args, disp );
		}
	}
	TempString = "";
	unsetPosition();
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
	NLMISC_CONVERT_VARGS (str, format, 256/*NLMISC::MaxCStringSize*/);

	if (strlen(str)<256/*NLMISC::MaxCStringSize*/-1)
		strcat (str, "\n");
	else
		str[256/*NLMISC::MaxCStringSize*/-2] = '\n';

	displayRawString(str);
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
	NLMISC_CONVERT_VARGS (str, format, 256/*NLMISC::MaxCStringSize*/);

	displayRawString(str);
}


void CLog::forceDisplayRaw (const char *format, ...)
{
	if ( noDisplayer() )
	{
		return;
	}

	char *str;
	NLMISC_CONVERT_VARGS (str, format, 256/*NLMISC::MaxCStringSize*/);

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
	if (filterstr == NULL)
	{
		_PositiveFilter.clear();
		_NegativeFilter.clear();
		//displayNL ("CLog::addNegativeFilter('%s')", filterstr);
	}
	else
	{
		_PositiveFilter.remove( filterstr );
		_NegativeFilter.remove( filterstr );
		//displayNL ("CLog::removeFilter('%s')", filterstr);
	}
}

void CLog::displayFilter( CLog &log )
{
	std::list<std::string>::iterator it;
	log.displayNL ("Positive Filter(s):");
	for (it = _PositiveFilter.begin (); it != _PositiveFilter.end (); it++)
	{
		log.displayNL ("'%s'", (*it).c_str());
	}
	log.displayNL ("Negative Filter(s):");
	for (it = _NegativeFilter.begin (); it != _NegativeFilter.end (); it++)
	{
		log.displayNL ("'%s'", (*it).c_str());
	}
}

void CLog::addPositiveFilter( const char *filterstr )
{
	//displayNL ("CLog::addPositiveFilter('%s')", filterstr);
	_PositiveFilter.push_back( filterstr );
}

void CLog::addNegativeFilter( const char *filterstr )
{
	//displayNL ("CLog::addNegativeFilter('%s')", filterstr);
	_NegativeFilter.push_back( filterstr );
}

void CLog::resetFilters()
{
	//displayNL ("CLog::resetFilter()");
	_PositiveFilter.clear();
	_NegativeFilter.clear();
}

} // NLMISC

