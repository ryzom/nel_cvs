/** \file command.cpp
 * <File description>
 *
 * $Id: command.cpp,v 1.9 2002/03/14 09:48:39 lecroart Exp $
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

#include "nel/misc/command.h"

using namespace std;
using namespace NLMISC;

namespace NLMISC {

ICommand::TCommand *ICommand::Commands;
bool ICommand::CommandsInit;

ICommand::ICommand(const char *commandName, const char *commandHelp, const char *commandArgs)
{
	// self registration

	if (!CommandsInit)
	{
		//printf("create map\n");
		Commands = new TCommand;
		CommandsInit = true;
	}

	TCommand::iterator comm = (*Commands).find(commandName);

	if (comm != (*Commands).end ())
	{
		// 2 commands have the same name
		nlstopex (("There are 2 commands that have the same name in the project (command name '%s')", commandName));
	}
	else
	{
		// insert the new command in the map
		//printf("add command\n");
		HelpString = commandHelp;
		CommandArgs = commandArgs;
		_CommandName = commandName;
		(*Commands)[commandName] = this;
	}

	Type = Command;
}

ICommand::~ICommand()
{
	// self deregistration

	if (!CommandsInit)
	{
		// should never happen
		nlstop;
		return;
	}

	// find the command

	for (TCommand::iterator comm = (*Commands).begin(); comm != (*Commands).end(); comm++)
	{
		if ((*comm).second == this)
		{
			//printf("remove command\n");
			(*Commands).erase (comm);

			if ((*Commands).size() == 0)
			{
				// if the commands map is empty, destroy it
				//printf("delete map\n");
				delete Commands;
				CommandsInit = false;
			}
			
			return;
		}
	}
	// commands is not found
	nlstop;
}

void ICommand::execute (const std::string &commandWithArgs, CLog &log)
{
	nlinfo ("Executing command : '%s'", commandWithArgs.c_str());

	// convert the buffer into string vector

	vector<string> args;
	string command;

	bool firstArg = true;
	uint i = 0;
	while (true)
	{
		// skip whitespace
		while (true)
		{
			if (i == commandWithArgs.size())
			{
				goto end;
				return;
			}
			if (commandWithArgs[i] != ' ' && commandWithArgs[i] != '\t' && commandWithArgs[i] != '\n' && commandWithArgs[i] != '\r')
			{
				break;
			}
			i++;
		}
		
		// get param
		string arg;
		if (commandWithArgs[i] == '\"')
		{
			// starting with a quote "
			i++;
			while (true)
			{
				if (i == commandWithArgs.size())
				{
					log.displayNL ("Missing end quote character \"");
					return;
				}
				if (commandWithArgs[i] == '"')
				{
					i++;
					break;
				}
				if (commandWithArgs[i] == '\\')
				{
					// manage escape char backslash
					i++;
					if (i == commandWithArgs.size())
					{
						log.displayNL ("Missing character after the backslash \\ character");
						return;
					}
					switch (commandWithArgs[i])
					{
						case '\\':	arg += '\\'; break; // double backslash
						case 'n':	arg += '\n'; break; // new line
						case '"':	arg += '"'; break; // "
						default:
							log.displayNL ("Unknown escape code '\\%c'", commandWithArgs[i]);
							return;
					}
					i++;
				}
				else
				{
					arg += commandWithArgs[i++];
				}
			}
		}
		else
		{
			// normal word
			while (true)
			{
				if (commandWithArgs[i] == '\\')
				{
					// manage escape char backslash
					i++;
					if (i == commandWithArgs.size())
					{
						log.displayNL ("Missing character after the backslash \\ character");
						return;
					}
					switch (commandWithArgs[i])
					{
						case '\\':	arg += '\\'; break; // double backslash
						case 'n':	arg += '\n'; break; // new line
						case '"':	arg += '"'; break; // "
						default:
							log.displayNL ("Unknown escape code '\\%c'", commandWithArgs[i]);
							return;
					}
					i++;
				}
				else
				{
					arg += commandWithArgs[i++];
				}
				if (i == commandWithArgs.size() || commandWithArgs[i] == ' ' || commandWithArgs[i] == '\t' || commandWithArgs[i] == '\n' || commandWithArgs[i] == '\r')
				{
					break;
				}
			}
		}
		if (firstArg)
		{
			command = arg;
			firstArg = false;
		}
		else
		{
			args.push_back (arg);
		}
	}
end:

/* displays args fordebug purpose
	nlinfo ("c '%s'", command.c_str());
	for (uint t = 0; t < args.size (); t++)
	{
		nlinfo ("p%d '%s'", t, args[t].c_str());
	}
*/
	
	// find the command	
	TCommand::iterator comm = (*Commands).find(commandWithArgs.c_str());
	if (comm == (*Commands).end ())
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
	for (TCommand::iterator comm = (*Commands).begin(); comm != (*Commands).end(); comm++)
	{
		if ((*comm).first.find(commandName) == 0)
		{
			commandName = (*comm).first;
		}
	}
}


void ICommand::serialCommands (IStream &f)
{
	vector<CSerialCommand> cmd;
	for (TCommand::iterator comm = (*Commands).begin(); comm != (*Commands).end(); comm++)
	{
		cmd.push_back (CSerialCommand ((*comm).first, (*comm).second->Type));
	}
	f.serialCont (cmd);
}


NLMISC_COMMAND(help,"display help on a specific variable/commands or on all variables and commands", "[<variable>|<command>]")
{	
	if (args.size() == 0)
	{
		// display all commands
		for (TCommand::iterator comm = (*Commands).begin(); comm != (*Commands).end(); comm++)
		{
			log.displayNL("%-15s: %s", (*comm).first.c_str(), (*comm).second->HelpString.c_str());
		}
	}
	else if (args.size() == 1)
	{
		// display help of the command
		TCommand::iterator comm = (*Commands).find(args[0].c_str());
		if (comm == (*Commands).end ())
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
