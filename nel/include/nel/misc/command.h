/** \file command.h
 * Management of runtime command line processing
 *
 * $Id: command.h,v 1.8 2001/06/21 12:35:16 lecroart Exp $
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

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <istream>

#include "nel/misc/types_nl.h"

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
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
#define NLMISC_COMMAND(__name,__help,__args) \
struct __name##class : public ICommand \
{ \
	__name##class() : NLMISC::ICommand(#__name,__help,__args) { } \
	virtual bool execute(const std::vector<std::string> &args, NLMISC::CLog &log); \
}; \
__name##class __name##instance; \
bool __name##class::execute(const std::vector<std::string> &args, NLMISC::CLog &log)



/**
 * Add a variable that can be modify in realtime. The variable must be global. If you must acces the variable with
 * function, use NLMISC_DYNVARIABLE
 *
 * Example:
 * \code
	// I want to look and change the variable 'foobar' in realtime, so, first i create it:
	uint8 foobar;
	// and then, I add it
	NLMISC_VARIABLE(uint8, foobar, "this is a dummy variable");
 * \endcode
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
#define NLMISC_VARIABLE(__type,__var,__help) \
CVariable<__type> __var##instance(#__var, __help " (" #__type ")", &__var)



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
	NLMISC_DYNVARIABLE(uint8, foobar, "this is a dummy variable")
	{
		// read or write the variable
		if (get)
			*pointer = getVar();
		else
			setVar(*pointer);
	}
 * \endcode
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
#define NLMISC_DYNVARIABLE(__type,__name,__help) \
class __name##class : public NLMISC::ICommand \
{ \
public: \
	__name##class () : NLMISC::ICommand(#__name, __help " (" #__type ")", "[<value>]") { } \
	virtual bool execute(const std::vector<std::string> &args, NLMISC::CLog &log) \
	{ \
		if (args.size() == 1) \
		{ \
			std::stringstream s (args[0]); \
			__type p; \
			s >> p; \
			pointer (&p, false, log); \
			log.display("Set "); \
		} \
		if (args.size() >= 0) \
		{ \
			std::stringstream s; \
			__type p; \
			pointer (&p, true, log); \
			s << _CommandName << " = " << p; \
			log.displayNL(s.str().c_str()); \
		} \
		if (args.size() > 1) \
			return false; \
		 \
		return true; \
	} \
 \
	void pointer(__type *pointer, bool get, NLMISC::CLog &log); \
}; \
__name##class __name##instance; \
void __name##class::pointer(__type *pointer, bool get, NLMISC::CLog &log)





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
	
	typedef std::map<std::string, ICommand *> TCommand;

	static TCommand *_Commands;
	static bool		 _CommandsInit;

	/// Executes the command and display output to the log
	static void execute (const std::string &commandWithArgs, NLMISC::CLog &log);

	static void	expand (std::string &commandName);

	/// Fills the vector with all command name
	static void getCommands (std::vector<std::string> &commands);

protected:

	std::string _CommandName;
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
	CVariable (const char *commandName, const char *commandHelp, T *pointer) : NLMISC::ICommand(commandName, commandHelp, "[<value>]"), _Pointer(pointer) { }
	virtual bool execute(const std::vector<std::string> &args, NLMISC::CLog &log)
	{
		if (args.size() == 1)
		{
			std::stringstream s2 (args[0]);
			s2 >> *_Pointer;
			log.display("Set ");
		}
		if (args.size() >= 0)
		{
			std::stringstream s;
			s << _CommandName << " = " << *_Pointer;
			log.displayNL(s.str().c_str());
		}
		if (args.size() > 1)
			return false;
		
		return true;
	}
private:
	T *_Pointer;
};




/* All the code above is used to add our types (uint8, ...) in the stringstream.
 * So we can use stringstream operator << and >> with all NeL simple types (except for ucchar and ucstring)
 */

#ifdef NL_OS_WINDOWS

#define NLMISC_ADD_BASIC_ISTREAM_OPERATOR(__type,__casttype) \
template <class _CharT, class _Traits> \
std::basic_istream<_CharT, _Traits>& __STL_CALL \
operator>>(std::basic_istream<_CharT, _Traits>& __is, __type& __z) \
{ \
	__casttype __z2 = (__casttype) __z; \
	__is.operator>>(__z2); \
	__z = (__type) __z2; \
	return __is; \
} \
 \
template <class _CharT, class _Traits> \
std::basic_ostream<_CharT, _Traits>& __STL_CALL \
operator<<(std::basic_ostream<_CharT, _Traits>& __os, const __type& __z) \
{ \
	std::basic_ostringstream<_CharT, _Traits, std::allocator<_CharT> > __tmp; \
	__tmp << (__casttype) __z; \
	return __os << __tmp.str(); \
}

NLMISC_ADD_BASIC_ISTREAM_OPERATOR(uint8, unsigned int);
NLMISC_ADD_BASIC_ISTREAM_OPERATOR(sint8, signed int);
NLMISC_ADD_BASIC_ISTREAM_OPERATOR(uint16, unsigned int);
NLMISC_ADD_BASIC_ISTREAM_OPERATOR(sint16, signed int);
NLMISC_ADD_BASIC_ISTREAM_OPERATOR(uint32, unsigned int);
NLMISC_ADD_BASIC_ISTREAM_OPERATOR(sint32, signed int);


template <class _CharT, class _Traits>
std::basic_istream<_CharT, _Traits>& __STL_CALL
operator>>(std::basic_istream<_CharT, _Traits>& __is, uint64& __z)
{
	__z = 0;
	bool neg = false;
	char c;
	do
	{
		__is >> c;
	}
	while (isspace(c));

	if (c == '-')
	{
		neg = true;
		__is >> c;
	}

	while (isdigit(c))
	{
		__z *= 10;
		__z += c-'0';
		__is >> c;
		if (__is.fail())
			break;
	}

	if (neg) __z = 0;

	return __is;
}

template <class _CharT, class _Traits>
std::basic_ostream<_CharT, _Traits>& __STL_CALL
operator<<(std::basic_ostream<_CharT, _Traits>& __os, const uint64& __z)
{
	std::basic_ostringstream<_CharT, _Traits, std::allocator<_CharT> > __tmp;

	uint64	__z2 = __z;
	uint64	__reverse = 0;
	uint	__digitnb = 0;

	if (__z == 0)
	{
		__tmp << '0';
	}
	else
	{
		// reverse digit
		while (__z2 != 0)
		{
			__reverse *= 10;
			__reverse += __z2%10;
			__z2 /= 10;
			__digitnb++;
		}

		// store the reverse number in the ostream
		while (__digitnb != 0)
		{
			__tmp << (char)((__reverse%10)+'0');
			__reverse /= 10;
			__digitnb--;
		}
	}
	return __os << __tmp.str();
}

template <class _CharT, class _Traits>
std::basic_istream<_CharT, _Traits>& __STL_CALL
operator>>(std::basic_istream<_CharT, _Traits>& __is, sint64& __z)
{
	__z = 0;
	bool neg = false;
	char c;
	do
	{
		__is >> c;
	}
	while (isspace(c));

	if (c == '-')
	{
		neg = true;
		__is >> c;
	}

	while (isdigit(c))
	{
		__z *= 10;
		__z += c-'0';
		__is >> c;
		if (__is.fail())
			break;
	}

	if (neg) __z = -__z;

	return __is;
}

template <class _CharT, class _Traits>
std::basic_ostream<_CharT, _Traits>& __STL_CALL
operator<<(std::basic_ostream<_CharT, _Traits>& __os, const sint64& __z)
{
	std::basic_ostringstream<_CharT, _Traits, std::allocator<_CharT> > __tmp;

	sint64 __z2 = __z;

	if (__z2 < 0)
	{
		__z2 = -__z2;
		__tmp << '-';
	}

	sint64	__reverse = 0;
	uint	__digitnb = 0;

	// reverse digit
	while (__z2 != 0)
	{
		__reverse *= 10;
		__reverse += __z2%10;
		__z2 /= 10;
		__digitnb++;
	}

	// store the reverse number in the ostream
	while (__digitnb != 0)
	{
		__tmp << (char)((__reverse%10)+'0');
		__reverse /= 10;
		__digitnb--;
	}

	return __os << __tmp.str();
}

#endif // NL_OS_WINDOWS

} // NLMISC


#endif // NL_COMMAND_H

/* End of command.h */
