/** \file command.h
 * Management of runtime command line processing
 *
 * $Id: command.h,v 1.28 2004/01/15 17:27:03 lecroart Exp $
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

#ifndef NL_COMMAND_H
#define NL_COMMAND_H

#include "nel/misc/types_nl.h"

#include <string>
#include <map>
#include <vector>
//#include <sstream>
#include <istream>

#include "nel/misc/stream.h"
#include "nel/misc/config_file.h"
#include "nel/misc/log.h"


namespace NLMISC {

/** WARNING:
 *   This is standard Unix linker behavior: object files
 *   that are not referenced from outside are discarded. The
 *   file in which you run your constructor is thus simply
 *   thrown away by the linker, which explains why the constructor
 *   is not run.
 */

/**
 * Create a function that can be call in realtime.
 *
 * Example:
 * \code
	// I want to create a function that compute the square of the parameter and display the result
	NLMISC_COMMAND(square,"display the square of the parameter","<value>")
	{
		// check args, if there s not the right number of parameter, return bad
		if(args.size() != 1) return false;
		// get the value
		uint32 val = atoi(args[0].c_str());
		// display the result on the displayer
		log.displayNL("The square of %d is %d", val, val*val);
		return true;
	}
 * \endcode
 *
 * Please use the same casing than for the function (first letter in lower case and after each word first letter in upper case)
 * ie: myFunction, step, orderByName, lookAtThis
 *
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
#define NLMISC_COMMAND(__name,__help,__args) \
struct __name##Class: public NLMISC::ICommand \
{ \
	__name##Class() : NLMISC::ICommand(#__name,__help,__args) { } \
	virtual bool execute(const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human); \
}; \
__name##Class __name##Instance; \
bool __name##Class::execute(const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human)


/**
 * Create a function that can be call in realtime. Don't use this class directly but use the macro NLMISC_COMMAND
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class ICommand
{
public:

	/// Constructor
	ICommand(const char *commandName, const char *commandHelp, const char *commandArgs);

	virtual ~ICommand();

	// quiet means that we don't display anything else than the value
	// human means that we want the value in a human readable if possible
	virtual bool execute(const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human = true) = 0;

	std::string HelpString;
	std::string CommandArgs;
	
	// is it a variable or a classic command?
	enum TType { Unknown, Command, Variable };
	TType Type;

	// static members

	typedef std::map<std::string, ICommand *> TCommand;

	static TCommand *Commands;
	static bool		 CommandsInit;

	/// Executes the command and display output to the log
	/// \param quiet true if you don't want to display the "executing the command ..."
	static void execute (const std::string &commandWithArgs, NLMISC::CLog &log, bool quiet = false, bool human = true);

	/** Command name completion.
	 * Case-sensitive. Displays the list after two calls with the same non-unique completion.
     * Completes commands used with prefixes (such as "help " for example) as well.
	 */
	static void	expand (std::string &commandName, NLMISC::CLog &log=*InfoLog);

	static void serialCommands (IStream &f);

	/// returns true if the command exists
	static bool exists (std::string &commandName);

	/// if the string begin with an upper case, it s a variable, otherwise, it s a command
	static bool isCommand (const std::string &str)
	{
		if (str.empty())
			return false;
		
		return isupper(str[0]) == 0;
	}
	
	const std::string &getName () const { return _CommandName; }

protected:

	std::string _CommandName;

	friend void cbVarChanged (CConfigFile::CVar &var);

};

/** This class is only used to serialize easily a command for the admin service for example */
struct CSerialCommand
{
	CSerialCommand () : Name ("<Unknown>"), Type(ICommand::Unknown) { }
	CSerialCommand (std::string n, ICommand::TType t) : Name (n), Type(t) { }

	std::string Name;
	ICommand::TType Type;

	void serial (IStream &f)
	{
		f.serial (Name);
		f.serialEnum (Type);
	}
};


} // NLMISC


#endif // NL_COMMAND_H

/* End of command.h */
