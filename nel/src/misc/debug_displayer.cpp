/** \file debug_displayer.cpp
 * <File description>
 *
 * $Id: debug_displayer.cpp,v 1.1 2001/12/12 15:34:39 legros Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "nel/misc/debug_displayer.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

namespace NLMISC {


/*
 * Constructor
 */
CDebugDisplayer::CDebugDisplayer (const std::string &filename, bool eraseLastLog, const char *displayerName) : IDisplayer (displayerName), _NeedHeader(true)
{
	setParam (filename, eraseLastLog);
}

CDebugDisplayer::CDebugDisplayer (const char *displayerName) : IDisplayer (displayerName), _NeedHeader(true)
{
}

CDebugDisplayer::~CDebugDisplayer()
{
	flush();
}

void CDebugDisplayer::setParam (const std::string &filename, bool eraseLastLog)
{
	_FileName = filename;

	if (filename.empty())
	{
		nlwarning ("CDebugDisplayer::setParam(): Can't create file with empty filename, don't log");
		return;
	}

	if (eraseLastLog)
	{
		ofstream ofs (filename.c_str(), ios::out | ios::trunc);
		if (!ofs.is_open())
		{
			nlwarning ("CDebugDisplayer::setParam(): Can't open and clear the log file '%s', don't log", filename.c_str());
		}
	}
}


// Log format: "2000/01/15 12:05:30 <ProcessName> <LogType> <ThreadId> <Filename> <Line> : <Msg>"
void CDebugDisplayer::doDisplay ( const TDisplayInfo& args, const char *message )
{
	stringstream	ss;
	bool			needSpace = false;

	if (_NeedHeader)
	{
		ss << HeaderString();
		_NeedHeader = false;
	}

	if (args.Date != 0)
	{
		ss << dateToHumanString(args.Date);
		needSpace = true;
	}

	if (!args.ProcessName.empty())
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << args.ProcessName;
		needSpace = true;
	}

	if (args.LogType != CLog::LOG_NO)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << logTypeToString(args.LogType);
		needSpace = true;
	}

	// Write thread identifier
	if ( args.ThreadId != 0 )
	{
		ss << setw(5) << args.ThreadId;
		needSpace = true;
	}

	if (args.Filename != NULL)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << CFile::getFilename(args.Filename);
		needSpace = true;
	}

	if (args.Line != -1)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << args.Line;
		needSpace = true;
	}
	
	if (needSpace) { ss << " : "; needSpace = false; }

	ss << message;
	_String += ss.str();
}

void CDebugDisplayer::flush ()
{
	// if the filename is not set, don't log
	if (!_FileName.empty())
	{
		ofstream ofs (_FileName.c_str (), ios::out | ios::app);
		if (ofs.is_open ())
		{
			ofs << _String;
			ofs.close();
		}

		_String.clear();
	}
}

} // NLMISC
