/** \file command.h
 * Management of runtime command line processing
 *
 * $Id: command.h,v 1.17 2002/11/29 10:07:54 lecroart Exp $
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
#include <sstream>
#include <istream>

#include "nel/misc/stream.h"

#include "nel/misc/log.h"


namespace NLMISC {


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
	virtual bool execute(const std::vector<std::string> &args, NLMISC::CLog &log); \
}; \
__name##Class __name##Instance; \
bool __name##Class::execute(const std::vector<std::string> &args, NLMISC::CLog &log)



/**
 * Add a variable that can be modify in realtime. The variable must be global. If you must acces the variable with
 * function, use NLMISC_DYNVARIABLE
 *
 * Example:
 * \code
	// I want to look and change the variable 'foobar' in realtime, so, first i create it:
	uint8 foobar;
	// and then, I add it
	NLMISC_VARIABLE(uint8, FooBar, "this is a dummy variable");
 * \endcode
 *
 * Please use the same casing than for the variable (first letter of each word in upper case)
 * ie: MyVariable, NetSpeedLoop, Time
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
#define NLMISC_VARIABLE(__type,__var,__help) \
NLMISC::CVariable<__type> __var##Instance(#__var, __help " (" #__type ")", &__var)



/**
 * Add a variable that can be modify in realtime. The code profide the way to access to the variable in the read
 * and write access (depending of the \c get boolean value)
 *
 * Example:
 * \code
	// a function to read the variable
	uint8 getVar() { return ...; }

	// a function to write the variable
	void setVar(uint8 val) { ...=val; }

	// I want to look and change the variable in realtime:
	NLMISC_DYNVARIABLE(uint8, FooBar, "this is a dummy variable")
	{
		// read or write the variable
		if (get)
			*pointer = getVar();
		else
			setVar(*pointer);
	}
 * \endcode
 *
 * Please use the same casing than for the variable (first letter of each word in upper case)
 * ie: MyVariable, NetSpeedLoop, Time
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
#define NLMISC_DYNVARIABLE(__type,__name,__help) \
class __name##Class : public NLMISC::ICommand \
{ \
public: \
	__name##Class () : NLMISC::ICommand(#__name, __help " (" #__type ")", "[<value>]") { Type = Variable; } \
	virtual bool execute(const std::vector<std::string> &args, NLMISC::CLog &log) \
	{ \
		if (args.size() == 1) \
		{ \
			std::stringstream ls (args[0]); \
			__type p; \
			ls >> p; \
			pointer (&p, false, log); \
		} \
		if (args.size() >= 0) \
		{ \
			__type p; \
			pointer (&p, true, log); \
			std::stringstream ls; \
			ls << "Variable " << _CommandName << " = " << p; \
			log.displayNL(ls.str().c_str()); \
		} \
		if (args.size() > 1) \
			return false; \
		 \
		return true; \
	} \
 \
	void pointer(__type *pointer, bool get, NLMISC::CLog &log); \
}; \
__name##Class __name##Instance; \
void __name##Class::pointer(__type *pointer, bool get, NLMISC::CLog &log)


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

	virtual ICommand::~ICommand();

	virtual bool execute(const std::vector<std::string> &args, NLMISC::CLog &log) = 0;

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
	static void execute (const std::string &commandWithArgs, NLMISC::CLog &log, bool quiet = false);

	/** Command name completion.
	 * Case-sensitive. Displays the list after two calls with the same non-unique completion.
     * Completes commands used with prefixes (such as "help " for example) as well.
	 */
	static void	expand (std::string &commandName, NLMISC::CLog &log=*InfoLog);

	static void serialCommands (IStream &f);

protected:

	std::string _CommandName;
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





/**
 * Create a variable that can be modify in realtime. Don't use this class directly but use the macro NLMISC_VARIABLE
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
template <class T>
class CVariable : public ICommand
{
public:
	CVariable (const char *commandName, const char *commandHelp, T *pointer) : NLMISC::ICommand(commandName, commandHelp, "[<value>]"), _Pointer(pointer) {	Type = Variable; }
	virtual bool execute(const std::vector<std::string> &args, NLMISC::CLog &log)
	{
		if (args.size() == 1)
		{
			std::stringstream s2 (args[0]);
			s2 >> *_Pointer;
		}
		if (args.size() >= 0)
		{
			std::stringstream s;
			s << "Variable " << _CommandName << " = " << *_Pointer;
			log.displayNL(s.str().c_str());
		}
		if (args.size() > 1)
			return false;
		
		return true;
	}
private:
	T *_Pointer;
};


} // NLMISC


#endif // NL_COMMAND_H

/* End of command.h */
