/** \file variable.h
 * Management of runtime variable
 *
 * $Id: variable.h,v 1.1 2003/03/06 09:59:56 lecroart Exp $
 */

/* Copyright, 2003 Nevrax Ltd.
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

#ifndef NL_VARIABLE_H
#define NL_VARIABLE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/command.h"


namespace NLMISC {

/** WARNING:
 *   This is standard Unix linker behavior: object files
 *   that are not referenced from outside are discarded. The
 *   file in which you run your constructor is thus simply
 *   thrown away by the linker, which explains why the constructor
 *   is not run.
 */


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
			__type p2; \
			ls >> p2; \
			pointer (&p2, false, log); \
		} \
		__type p; \
		pointer (&p, true, log); \
		std::stringstream ls; \
		ls << "Variable " << _CommandName << " = " << p; \
		log.displayNL(ls.str().c_str()); \
		return (args.size() <= 1); \
	} \
 \
	void pointer(__type *pointer, bool get, NLMISC::CLog &log); \
}; \
__name##Class __name##Instance; \
void __name##Class::pointer(__type *pointer, bool get, NLMISC::CLog &log)


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
		{
			std::stringstream s;
			s << "Variable " << _CommandName << " = " << *_Pointer;
			log.displayNL(s.str().c_str());
		}
		return (args.size() <= 1);
	}
private:
	T *_Pointer;

	// some stat about the variable

};


} // NLMISC


#endif // NL_VARIABLE_H

/* End of variable.h */
