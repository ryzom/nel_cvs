/** \file window_displayer.cpp
 * Implementation of the CDisplayer (look at displayer.h) that display on a Windows.
 * It's the base class for win_displayer (win32 api) and gtk_displayer (gtk api)
 *
 * $Id: window_displayer.cpp,v 1.11 2003/02/03 15:53:56 coutelas Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#include <iostream>
#include <fstream>
#include <iomanip>
#include <signal.h>

#include "nel/misc/path.h"
#include "nel/misc/command.h"
#include "nel/misc/thread.h"

#include "nel/misc/window_displayer.h"

using namespace std;

namespace NLMISC {

class CUpdateThread : public IRunnable
{
	CWindowDisplayer *Disp;
	string WindowNameEx;
	bool Iconified;
	sint X, Y, W, H, HS;
	uint32 FS;
	string FN;
	bool WW;

public:
	CUpdateThread (CWindowDisplayer *disp, string windowNameEx, bool iconified, sint x, sint y, sint w, sint h, sint hs, sint fs, const std::string &fn, bool ww) :
	  Disp(disp), WindowNameEx(windowNameEx), X(x), Y(y), W(w), H(h), HS(hs), Iconified(iconified), FS(fs), FN(fn), WW(ww)
	{
	}

	void run()
	{
		Disp->open (WindowNameEx, Iconified, X, Y, W, H, HS, FS, FN, WW);
		Disp->display_main ();
	}
};

CWindowDisplayer::~CWindowDisplayer ()
{
	// we have to wait the exit of the thread
	_Continue = false;
	nlassert (_Thread != NULL);
	_Thread->wait();
	delete _Thread;
}

bool CWindowDisplayer::update ()
{
	vector<string> copy;
	{
		CSynchronized<std::vector<std::string> >::CAccessor access (&_CommandsToExecute);
		copy = access.value();
		access.value().clear ();
	}

	// execute all commands in the main thread
	for (uint i = 0; i < copy.size(); i++)
	{
		ICommand::execute (copy[i], *InfoLog);
	}

	return _Continue;
}

uint CWindowDisplayer::createLabel (const char *value)
{
	int pos;
	{
		CSynchronized<std::vector<CLabelEntry> >::CAccessor access (&_Labels);
		access.value().push_back (CLabelEntry(value));
		pos = access.value().size()-1;
	}
	return pos;
}

void CWindowDisplayer::setLabel (uint label, const string &value)
{
	{
		CSynchronized<std::vector<CLabelEntry> >::CAccessor access (&_Labels);
		nlassert (label < access.value().size());
		if (access.value()[label].Value != value)
		{
			access.value()[label].Value = value;
			access.value()[label].NeedUpdate = true;
		}
	}
}

void CWindowDisplayer::create (string windowNameEx, bool iconified, sint x, sint y, sint w, sint h, sint hs, sint fs, const std::string &fn, bool ww)
{
	nlassert (_Thread == NULL);
	_Thread = IThread::create (new CUpdateThread(this, windowNameEx, iconified, x, y, w, h, hs, fs, fn, ww));
	
	_Thread->start ();
}

void CWindowDisplayer::doDisplay (const NLMISC::TDisplayInfo &args, const char *message)
{
	bool needSpace = false;
	stringstream ss;

	uint32 color = 0xFF000000;

	if (args.LogType != CLog::LOG_NO)
	{
		ss << logTypeToString(args.LogType);
		if (args.LogType == CLog::LOG_ERROR || args.LogType == CLog::LOG_ASSERT) color = 0x00FF0000;
		else if (args.LogType == CLog::LOG_WARNING) color = 0x00800000;
		else if (args.LogType == CLog::LOG_DEBUG) color = 0x00808080;
		else color = 0;
		needSpace = true;
	}

	// Write thread identifier
	if ( args.ThreadId != 0 )
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << setw(4) << args.ThreadId;
		needSpace = true;
	}

	if (args.Filename != NULL)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << setw(20) << CFile::getFilename(args.Filename);
		needSpace = true;
	}

	if (args.Line != -1)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << setw(4) << args.Line;
		needSpace = true;
	}

	if (needSpace) { ss << ": "; needSpace = false; }

	uint nbl = 1;

	char *npos, *pos = const_cast<char *>(message);
	while ((npos = strchr (pos, '\n')))
	{
		*npos = '\0';
		ss << pos;
		if (needSlashR)
			ss << "\r";
		ss << "\n";
		*npos = '\n';
		pos = npos+1;
		nbl++;
	}
	ss << pos;

	pos = const_cast<char *>(args.CallstackAndLog.c_str());
	while ((npos = strchr (pos, '\n')))
	{
		*npos = '\0';
		ss << pos;
		if (needSlashR)
			ss << "\r";
		ss << "\n";
		*npos = '\n';
		pos = npos+1;
		nbl++;
	}
	ss << pos;

	{
		CSynchronized<std::list<std::pair<uint32, std::string> > >::CAccessor access (&_Buffer);
		if (_HistorySize > 0 && access.value().size() >= (uint)_HistorySize)
		{
			access.value().erase (access.value().begin());
		}
		access.value().push_back (make_pair (color, ss.str()));
	}
}

} // NLMISC
