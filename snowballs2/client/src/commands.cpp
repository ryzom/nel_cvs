/** \file commands.cpp
 * commands management with user interface
 *
 * $Id: commands.cpp,v 1.8 2001/07/12 17:39:12 lecroart Exp $
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

#include <list>

#include <nel/misc/types_nl.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/command.h>
#include <nel/misc/log.h>
#include <nel/misc/displayer.h>

#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_3d_mouse_listener.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_landscape.h>

#include "network.h"
#include "client.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

list <string> StoredLines;
uint32 NbStoredLines = 100;

CLog CommandsLog;



// these variables are set with the config file

float CommandsBoxX, CommandsBoxY, CommandsBoxWidth;
float CommandsBoxBorder;
int CommandsNbLines;
float CommandsLineHeight;
int CommandsFontSize;
CRGBA CommandsBackColor, CommandsFrontColor;

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

bool commandLine (const string &str)
{
	string command = "";

	if (str[0]=='/')
	{
		command = str.substr(1);

		// add the string in to the chat
		addLine (string ("command> ") + str);

		ICommand::execute (command, CommandsLog);
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
				// it s a chat line, send  it to the server
				if (isOnline ())
					sendChatLine (_Line);
				else
					addLine (string ("you said> ") + _Line);
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

void cbUpdateCommands (CConfigFile::CVar &var)
{
	if (var.Name == "CommandsBoxX") CommandsBoxX = var.asFloat ();
	else if (var.Name == "CommandsBoxY") CommandsBoxY = var.asFloat ();
	else if (var.Name == "CommandsBoxWidth") CommandsBoxWidth = var.asFloat ();
	else if (var.Name == "CommandsBoxBorder") CommandsBoxBorder = var.asFloat ();
	else if (var.Name == "CommandsNbLines") CommandsNbLines = var.asInt ();
	else if (var.Name == "CommandsLineHeight") CommandsLineHeight = var.asFloat ();
	else if (var.Name == "CommandsBackColor") CommandsBackColor.set (var.asInt(0), var.asInt(1), var.asInt(2), var.asInt(3));
	else if (var.Name == "CommandsFrontColor") CommandsFrontColor.set (var.asInt(0), var.asInt(1), var.asInt(2), var.asInt(3));
	else if (var.Name == "CommandsFontSize") CommandsFontSize = var.asInt ();
	else nlwarning ("Unknown variable update %s", var.Name.c_str());
}

void	initCommands()
{
	Driver->EventServer.addListener (EventCharId, &CommandsListener);

	CommandsLog.addDisplayer (&CommandsDisplayer);
	InfoLog->addDisplayer (&CommandsDisplayer);
	WarningLog->addDisplayer (&CommandsDisplayer);
	AssertLog->addDisplayer (&CommandsDisplayer);
	ErrorLog->addDisplayer (&CommandsDisplayer);

	ConfigFile.setCallback ("CommandsBoxX", cbUpdateCommands);
	ConfigFile.setCallback ("CommandsBoxY", cbUpdateCommands);
	ConfigFile.setCallback ("CommandsBoxWidth", cbUpdateCommands);
	ConfigFile.setCallback ("CommandsBoxBorder", cbUpdateCommands);
	ConfigFile.setCallback ("CommandsNbLines", cbUpdateCommands);
	ConfigFile.setCallback ("CommandsLineHeight", cbUpdateCommands);
	ConfigFile.setCallback ("CommandsBackColor", cbUpdateCommands);
	ConfigFile.setCallback ("CommandsFrontColor", cbUpdateCommands);
	ConfigFile.setCallback ("CommandsFontSize", cbUpdateCommands);
  
	cbUpdateCommands (ConfigFile.getVar ("CommandsBoxX"));
	cbUpdateCommands (ConfigFile.getVar ("CommandsBoxY"));
	cbUpdateCommands (ConfigFile.getVar ("CommandsBoxWidth"));
	cbUpdateCommands (ConfigFile.getVar ("CommandsBoxBorder"));
	cbUpdateCommands (ConfigFile.getVar ("CommandsNbLines"));
	cbUpdateCommands (ConfigFile.getVar ("CommandsLineHeight"));
	cbUpdateCommands (ConfigFile.getVar ("CommandsBackColor"));
	cbUpdateCommands (ConfigFile.getVar ("CommandsFrontColor"));
	cbUpdateCommands (ConfigFile.getVar ("CommandsFontSize"));
}

void	updateCommands()
{
	// Display
	Driver->setMatrixMode2D11 ();
	Driver->drawQuad (CommandsBoxX-CommandsBoxBorder, CommandsBoxY-CommandsBoxBorder, CommandsBoxX+CommandsBoxWidth+CommandsBoxBorder, CommandsBoxY + (CommandsNbLines+1) * CommandsLineHeight + CommandsBoxBorder, CommandsBackColor);

	// Set the text context
	TextContext->setHotSpot (UTextContext::BottomLeft);
	TextContext->setColor (CommandsFrontColor);
	TextContext->setFontSize (CommandsFontSize);

	// Output text
	string line = string("> ")+CommandsListener.line() + string ("_");
	TextContext->printfAt (CommandsBoxX, CommandsBoxY + CommandsBoxBorder, line.c_str());
	CommandsListener.setMaxWidthReached (TextContext->getLastXBound() > CommandsBoxWidth*1.33f); // max is 1.33=4/3

	// display stored lines
	float yPos = CommandsBoxY + CommandsBoxBorder;
	list<string>::reverse_iterator rit = StoredLines.rbegin();
	for (sint i = 0; i < CommandsNbLines; i++)
	{
		yPos += CommandsLineHeight;
		if (rit == StoredLines.rend()) break;
		TextContext->printfAt (CommandsBoxX, yPos, (*rit).c_str());
		rit++;
	}
}

void	clearCommands ()
{
	StoredLines.clear ();
}

void	releaseCommands()
{
}
