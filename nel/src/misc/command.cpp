/** \file command.cpp
 * <File description>
 *
 * $Id: command.cpp,v 1.5 2001/06/07 16:17:53 lecroart Exp $
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

#include <map>
#include <string>

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"

#include "nel/misc/command.h"

using namespace std;
using namespace NLMISC;

namespace NLMISC {

ICommand::TCommand *ICommand::_Commands;
bool ICommand::_CommandsInit;

ICommand::ICommand(const char *commandName, const char *commandHelp, const char *commandArgs)
{
	// self registration

	if (!_CommandsInit)
	{
		//printf("create map\n");
		_Commands = new TCommand;
		_CommandsInit = true;
	}

	TCommand::iterator comm = (*_Commands).find(commandName);

	if (comm != (*_Commands).end ())
	{
		// 2 commands have the same name
		NLMISC_BREAKPOINT;
	}
	else
	{
		// insert the new command in the map
		//printf("add command\n");
		HelpString = commandHelp;
		CommandArgs = commandArgs;
		_CommandName = commandName;
		(*_Commands)[commandName] = this;
	}
}

ICommand::~ICommand()
{
	// self deregistration

	if (!_CommandsInit)
	{
		// should never happen
		NLMISC_BREAKPOINT;
		return;
	}

	// find the command

	for (TCommand::iterator comm = (*_Commands).begin(); comm != (*_Commands).end(); comm++)
	{
		if ((*comm).second == this)
		{
			//printf("remove command\n");
			(*_Commands).erase (comm);

			if ((*_Commands).size() == 0)
			{
				// if the commands map is empty, destroy it
				//printf("delete map\n");
				delete _Commands;
				_CommandsInit = false;
			}
			
			return;
		}
	}
	// commands is not found
	NLMISC_BREAKPOINT;
}


void ICommand::execute (const std::string &commandWithArgs, CLog &log)
{
	nlinfo ("Executing command : '%s'", commandWithArgs.c_str());

	// convert the buffer into string vector

	vector<string> args;
	string command;

	char seps[]   = " ,\t\n";
	char *token;
	token = strtok ((char *)commandWithArgs.c_str(), seps);
	if (token == NULL)
	{
		command = commandWithArgs;
	}
	else
	{
		command = token;

		token = strtok (NULL, seps);
		while (token != NULL)
		{
			args.push_back(token);
			token = strtok (NULL, seps);
		}
	}
	
	// find the command	
	
	TCommand::iterator comm = (*_Commands).find(commandWithArgs.c_str());
	if (comm == (*_Commands).end ())
	{
		// the command doesn't exist
		log.displayNL("Command '%s' not found, try 'help'", commandWithArgs.c_str());
	}
	else
	{
		//printf("execute command\n");
		if (!(*comm).second->execute (args, log))
		{
			log.displayNL("Bad command usage, try 'help %s'", commandWithArgs.c_str());
		}
	}
}


void ICommand::expand (std::string &commandName)
{
	for (TCommand::iterator comm = (*_Commands).begin(); comm != (*_Commands).end(); comm++)
	{
		if ((*comm).first.find(commandName) == 0)
		{
			commandName = (*comm).first;
		}
	}
}


void ICommand::getCommands (std::vector<std::string> &commands)
{
	commands.clear ();
	for (TCommand::iterator comm = (*_Commands).begin(); comm != (*_Commands).end(); comm++)
	{
		commands.push_back((*comm).first);
	}
}


NLMISC_COMMAND(help,"display help on a specific variable/commands or on all variables and commands", "[<variable>|<command>]")
{	
	if (args.size() == 0)
	{
		// display all commands
		for (TCommand::iterator comm = (*_Commands).begin(); comm != (*_Commands).end(); comm++)
		{
			log.displayNL("%-15s: %s", (*comm).first.c_str(), (*comm).second->HelpString.c_str());
		}
	}
	else if (args.size() == 1)
	{
		// display help of the command
		TCommand::iterator comm = (*_Commands).find(args[0].c_str());
		if (comm == (*_Commands).end ())
		{
			log.displayNL("command '%s' not found", args[0].c_str());
		}
		else
		{
			log.displayNL("%s", (*comm).second->HelpString.c_str());
			log.displayNL("usage: %s %s", (*comm).first.c_str(), (*comm).second->CommandArgs.c_str(), (*comm).second->HelpString.c_str());
		}
	}
	else
	{
		return false;
	}
	return true;
}

} // NLMISC
