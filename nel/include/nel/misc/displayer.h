/** \file displayer.h
 * Displayer class interface and classic standard displayers
 *
 * $Id: displayer.h,v 1.15 2002/08/23 12:18:13 lecroart Exp $
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

#ifndef NL_DISPLAYER_H
#define NL_DISPLAYER_H

#include "nel/misc/types_nl.h"

#include <string>

#include "nel/misc/log.h"

namespace NLMISC
{


// Debug information
struct TDisplayInfo
{
	TDisplayInfo() : Date(0), LogType(CLog::LOG_NO), ThreadId(0), Filename(NULL), Line(-1) {}
	
	time_t				Date;
	CLog::TLogType		LogType;
	std::string			ProcessName;
	uint				ThreadId;
	const char			*Filename;
	sint				Line;

	std::string			CallstackAndLog;	// contains the callstack and a log with not filter of N last line (only in error/assert log type)
};


class CMutex;


/**
 * Displayer interface. Used to specialize a displayer to display a string.
 * \ref log_howto
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class IDisplayer
{
public:

	/// Constructor
	IDisplayer(const char *displayerName = "");

	/// Destructor
	virtual ~IDisplayer();

	/// Display the string where it does.
	void display( const TDisplayInfo& args, const char *message );

	/// This is the idenfiant for a displayer, it is used to fond or remove a displayer
	std::string DisplayerName;

protected:

	/// Method to implement in the deriver
	virtual void doDisplay( const TDisplayInfo& args, const char *message) = 0;

	
	/// Convert log type to string
	static const char *logTypeToString (CLog::TLogType logType, bool longFormat = false);

	/// Convert the current date to human string
	static const char *IDisplayer::dateToHumanString ();

	/// Convert date to "2000/01/14 10:05:17" string
	static const char *dateToHumanString (time_t date);

	/// Convert date to "784551148" string (time in second from 1975)
	static const char *dateToComputerString (time_t date);

	// Return the header string with date (for the first line of the log)
	static const char *HeaderString ();

private:

	CMutex	*_Mutex;
};



/**
 * Std displayer. Put string to stdout.
 * \ref log_howto
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CStdDisplayer : virtual public IDisplayer
{
public:
	CStdDisplayer (const char *displayerName = "") : IDisplayer (displayerName) {}

protected:

	/// Display the string to stdout and OutputDebugString on Windows
	virtual void doDisplay ( const TDisplayInfo& args, const char *message );
};


/**
 * File displayer. Put string into a file.
 * \ref log_howto
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CFileDisplayer : virtual public IDisplayer
{
public:

	/// Constructor
	CFileDisplayer (const std::string &filename, bool eraseLastLog = false, const char *displayerName = "");

	CFileDisplayer ();

	/// Set Parameter of the displayer if not set at the ctor time
	void setParam (const std::string &filename, bool eraseLastLog = false);

protected:
	/// Put the string into the file.
    virtual void doDisplay ( const TDisplayInfo& args, const char *message );

private:
	std::string _FileName;

	bool		_NeedHeader;
};

/**
 * Message Box displayer. Put string into a message box.
 * \ref log_howto
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CMsgBoxDisplayer : virtual public IDisplayer
{
public:
	CMsgBoxDisplayer (const char *displayerName = "") : IgnoreNextTime(false), IDisplayer (displayerName) {}

	bool IgnoreNextTime;

protected:
	/// Put the string into the file.
    virtual void doDisplay ( const TDisplayInfo& args, const char *message );
};


};

#endif // NL_DISPLAYER_H

/* End of displayer.h */
