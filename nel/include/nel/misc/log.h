/** \file log.h
 * Logging system providing multi displayer output and filtering processing
 *
 * $Id: log.h,v 1.33 2003/02/07 17:42:30 cado Exp $
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

#ifndef NL_LOG_H
#define NL_LOG_H

#include "nel/misc/types_nl.h"
#include "nel/misc/mutex.h"

#include <string>
#include <list>


namespace NLMISC
{

class IDisplayer;


/**
 * When display() is called, the logger builds a string a sends it to its attached displayers.
 * The positive filters, if any, are applied first, then the negative filters.
 * See the nldebug/nlinfo... macros in debug.h.
 *
 * \ref log_howto
 * \todo cado: display() and displayRaw() should save the string and send it only when displayRawNL()
 * (or a flush()-style method) is called.
 * \author Vianney Lecroart, Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CLog {
public:

	typedef enum { LOG_NO=0, LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG, LOG_STAT, LOG_ASSERT, LOG_UNKNOWN } TLogType;

	// Debug information
	struct TDisplayInfo
	{
		TDisplayInfo() : Date(0), LogType(CLog::LOG_NO), ThreadId(0), Filename(NULL), Line(-1) {}
		
		time_t				Date;
		TLogType		LogType;
		std::string			ProcessName;
		uint				ThreadId;
		const char			*Filename;
		sint				Line;

		std::string			CallstackAndLog;	// contains the callstack and a log with not filter of N last line (only in error/assert log type)
	};


	CLog(TLogType logType = LOG_NO);

	/// Add a new displayer in the log. You have to create the displayer, remove it and delete it when you have finish with it.
	/// For example, in a 3dDisplayer, you can add the displayer when you want, and the displayer displays the string if the 3d
	/// screen is available and do nothing otherwise. In this case, if you want, you could leave the displayer all the time.
	void addDisplayer (IDisplayer *displayer, bool bypassFilter = false);

	/// Return the first displayer selected by his name
	IDisplayer *getDisplayer (const char *displayerName);

	/// Remove a displayer. If the displayer doesn't work in a specific time, you could remove it.
	void removeDisplayer (IDisplayer *displayer);

	/// Remove a displayer using his name
	void removeDisplayer (const char *displayerName);

	/// Returns true if the specified displayer is attached to the log object
	bool attached(IDisplayer *displayer) const;
	
	/// Returns true if no displayer is attached
	bool noDisplayer() const { return _Displayers.empty() && _BypassFilterDisplayers.empty(); }


	/// Set the name of the process
	static void setProcessName (const std::string &processName);

	/// Find the process name if nobody call setProcessName before
	static void setDefaultProcessName ();

	/// If !noDisplayer(), sets line and file parameters, and enters the mutex. If !noDisplayer(), don't forget to call display...() after, to release the mutex.
	void setPosition (sint line, char *fileName);


	/// Display a string in decorated and final new line form to all attached displayers. Call setPosition() before. Releases the mutex.
	void displayNL (const char *format, ...);

	/// Display a string in decorated form to all attached displayers. Call setPosition() before. Releases the mutex.
	void display (const char *format, ...);

	/// Display a string with a final new line to all attached displayers. Call setPosition() before. Releases the mutex.
	void displayRawNL (const char *format, ...);

	/// Display a string (and nothing more) to all attached displayers. Call setPosition() before. Releases the mutex.
	void displayRaw (const char *format, ...);

	/// Display a raw text to the normal displayer but without filtering
	/// It's used by the Memdisplayer (little hack to work)
	void forceDisplayRaw (const char *format, ...);
	
	/// Adds a positive filter. Tells the logger to log only the lines that contain filterstr
	void addPositiveFilter( const char *filterstr );

	/// Adds a negative filter. Tells the logger to discard the lines that contain filterstr
	void addNegativeFilter( const char *filterstr );

	/// Reset both filters
	void resetFilters();

	/// Removes a filter by name (in both filters).
	void removeFilter( const char *filterstr = NULL);

	/// Displays the list of filter into a log
	void displayFilter( CLog &log );

protected:

	/// Symetric to setPosition(). Automatically called by display...(). Do not call if noDisplayer().
	void unsetPosition();

	/// Returns true if the string must be logged, according to the current filter
	bool passFilter( const char *filter );

	TLogType                          _LogType;
	static std::string		          *_ProcessName;

	sint                              _Line;
	char                             *_FileName;

	typedef std::list<IDisplayer *> CDisplayers;

	CDisplayers                       _Displayers;

	CDisplayers                       _BypassFilterDisplayers;	// these displayers always log info (by pass filter system)

	CMutex							  _Mutex;

	uint32							  _PosSet;

	/// "Discard" filter
	std::list<std::string>			  _NegativeFilter;

	/// "Crop" filter
	std::list<std::string>			  _PositiveFilter;

	/// Display a string in decorated form to all attached displayers.
	void displayString (const char *str);

	/// Display a Raw string to all attached displayers.
	void displayRawString (const char *str);

	std::string							TempString;
	TDisplayInfo						TempArgs;


};


} // NLMISC

#endif // NL_LOG_H

/* End of log.h */
