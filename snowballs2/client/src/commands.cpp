/** \file commands.cpp
 * commands management with user interface
 *
 * $Id: commands.cpp,v 1.4 2001/07/11 17:39:40 lecroart Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "nel/misc/types_nl.h"

#include <list>

#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_3d_mouse_listener.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_landscape.h>

#include <nel/misc/event_listener.h>
#include <nel/misc/command.h>
#include <nel/misc/log.h>
#include <nel/misc/displayer.h>

#include "client.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

list <string> StoredLines;
uint32 NbStoredLines = 100, NbDisplayedLines = 5;

void addLine (const string &line)
{
	// add line

	StoredLines.push_back (line);

	// clear old lines

	while (StoredLines.size () > NbStoredLines)
	{
		StoredLines.pop_front ();
	}
}

/*
 * CChatDisplayer
 *
 * Log format : "<LogType>: <Msg>"
 */
class CCommandsDisplayer : public IDisplayer
{
	virtual void doDisplay (const TDisplayInfo &args, const char *message)
	{
		bool needSpace = false;
		stringstream ss;

		if (args.LogType != CLog::LOG_NO)
		{
			ss << logTypeToString(args.LogType);
			needSpace = true;
		}

		if (needSpace) { ss << ": "; needSpace = false; }

		ss << message;

		addLine (ss.str());
	}
};


CCommandsDisplayer CommandsDisplayer;

CLog CommandExecutionLog;


bool commandLine (const string &str)
{
	string command = "";

	if (str[0]=='/')
	{
		command = str.substr(1);

		// add the string in to the chat
		addLine (str);

		ICommand::execute (command, CommandExecutionLog);
	}
	else
		return false;

	return true;

}


class CCommandsListener : public IEventListener
{
	virtual void	operator() ( const CEvent& event )
	{
		CEventChar &ec = (CEventChar&)event;

		switch ( ec.Char )
		{
		case 13 : // RETURN : Send the chat message
			if ( _Line.size() == 0 )
				break;

			// if, it s a command, execute it and don't send the command
			if ( ! commandLine( _Line ) )
			{
				/// \todo ace: send the string to the server
				addLine (_Line);
			}
			_Line = "";
			_MaxWidthReached = false;
			break;

		case 8 : // BACKSPACE
			if ( _Line.size() != 0 )
			{
				_Line.erase( _Line.end()-1 );
				// _MaxWidthReached = false; // no need
			}
			break;
		case 9 : // TAB
			{
				if (!_Line.empty() && _Line[0] == '/')
				{
					string command = _Line.substr(1);
					ICommand::expand(command);
					_Line = '/' + command;
				}
			}
			break;
		case 27 : // ESCAPE
			break;

		default: 
			if ( ! _MaxWidthReached )
			{
				_Line += (char)ec.Char;
			}
		}
	}

public:
	CCommandsListener() : _MaxWidthReached( false )
	{}

	const string&	line() const
	{
		return _Line;
	}

	void			setMaxWidthReached( bool b )
	{
		_MaxWidthReached = b;
	}

private:
	string			_Line;
	bool			_MaxWidthReached;
};


CCommandsListener CommandsListener;

const float CommandsLineHeight = 0.025f;
const float CommandsLineY = 0.019f;


void	initCommands()
{
	Driver->EventServer.addListener (EventCharId, &CommandsListener);

	CommandExecutionLog.addDisplayer (&CommandsDisplayer);
}

void	updateCommands()
{
	// Display
	Driver->setMatrixMode2D11 ();
	Driver->drawQuad (0.01f, 0.01f, 0.99f, 0.3f, CRGBA (128, 255, 128, 128));

	// Output text
	TextContext->setHotSpot (UTextContext::BottomLeft);
	string line = string("> ")+CommandsListener.line() + string ("_");
	TextContext->printfAt (0.01f, CommandsLineY, line.c_str());
	CommandsListener.setMaxWidthReached (TextContext->getLastXBound() > 1.28); // max is 1.33=4/3

	float yPos = CommandsLineY;

	// display stored lines
	list<string>::reverse_iterator rit = StoredLines.rbegin();
	for (uint i = 0; i < NbDisplayedLines; i++)
	{
		yPos += CommandsLineHeight;
		if (rit == StoredLines.rend()) break;
		TextContext->printfAt (0.01f, yPos, (*rit).c_str());
		rit++;
	}
}

void	releaseCommands()
{
}
