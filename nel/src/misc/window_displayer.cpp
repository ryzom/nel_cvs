/** \file window_displayer.cpp
 * Implementation of the CDisplayer (look at displayer.h) that display on a Windows.
 * It's the base class for win_displayer (win32 api) and gtk_displayer (gtk api)
 *
 * $Id: window_displayer.cpp,v 1.4 2002/05/27 16:48:42 lecroart Exp $
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
	sint X, Y, W, H, HS;
	bool Iconified;

public:
	CUpdateThread (CWindowDisplayer *disp, string windowNameEx, bool iconified, sint x, sint y, sint w, sint h, sint hs) :
	  Disp(disp), WindowNameEx(windowNameEx), X(x), Y(y), W(w), H(h), HS(hs), Iconified(iconified)
	{
	}

	void run()
	{
		Disp->open (WindowNameEx, Iconified, X, Y, W, H, HS);
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
		access.value()[label].Value = value;
	}
}

void CWindowDisplayer::create (string windowNameEx, bool iconified, sint x, sint y, sint w, sint h, sint hs)
{
	nlassert (_Thread == NULL);
	_Thread = IThread::create (new CUpdateThread(this, windowNameEx, iconified, x, y, w, h, hs));
	
	_Thread->start ();
}

void CWindowDisplayer::doDisplay (const NLMISC::TDisplayInfo &args, const char *message)
{
	bool needSpace = false;
	stringstream ss;

	if (args.LogType != CLog::LOG_NO)
	{
		ss << logTypeToString(args.LogType);
		needSpace = true;
	}

	// Write thread identifier
	if ( args.ThreadId != 0 )
	{
		ss << setw(5) << args.ThreadId;
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

	uint nbl = 1;

	char *npos, *pos = const_cast<char *>(message);
	while (npos = strchr (pos, '\n'))
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
		CSynchronized<std::string>::CAccessor access (&_Buffer);
		access.value() += ss.str();
	}
}

} // NLMISC
