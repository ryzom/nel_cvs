/** \file displayer.h
 * Little easy displayers implementation
 *
 * $Id: displayer.h,v 1.6 2001/02/05 16:11:36 lecroart Exp $
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

#include <string>

#include "nel/misc/log.h"

namespace NLMISC
{

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
	IDisplayer() {}

	/// Destructor
	virtual ~IDisplayer() {}

	/// Display the string where it does.
	virtual void display (time_t date, CLog::TLogType logType, const std::string &processName, const char *fileName, sint line, const char *message) = 0;

protected:

	static const char *logTypeToString (CLog::TLogType logType, bool longFormat = false);

	/// Convert the current date to human string
	static const char *IDisplayer::dateToHumanString ();

	/// Convert date to "2000/01/14 10:05:17" string
	static const char *dateToHumanString (time_t date);

	/// Convert date to "784551148" string (time in second from 1975)
	static const char *dateToComputerString (time_t date);

	// Return the header string with date (for the first line of the log)
	static const char *HeaderString ();
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

	/// Display the string to stdout and OutputDebugString on Windows
	virtual void display (time_t date, CLog::TLogType logType, const std::string &processName, const char *fileName, sint line, const char *message);
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
	CFileDisplayer(const std::string& fileName, bool eraseLastLog = false);

	/// Put the string into the file.
    virtual void display (time_t date, CLog::TLogType logType, const std::string &processName, const char *fileName, sint line, const char *message);

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

	/// Constructor
	CMsgBoxDisplayer() {}

	/// Put the string into the file.
    virtual void display (time_t date, CLog::TLogType logType, const std::string &processName, const char *fileName, sint line, const char *message);
};


};

#endif // NL_DISPLAYER_H

/* End of displayer.h */
