/** \file variable.h
 * Management of runtime variable
 *
 * $Id: variable.h,v 1.2.2.1 2003/06/11 15:21:59 lecroart Exp $
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
#include "nel/misc/value_smoother.h"


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
NLMISC::CVariablePtr<__type> __var##Instance(#__var, __help " (" #__type ")", &__var)



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
class __name##Class : public NLMISC::IVariable \
{ \
public: \
	__name##Class () : IVariable(#__name, __help) { } \
	 \
	virtual void fromString(const std::string &val, bool human) \
	{ \
		std::stringstream ss (val); \
		__type p; \
		ss >> p; \
		pointer (&p, false, human); \
	} \
	 \
	virtual std::string toString(bool human) const \
	{ \
		__type p; \
		pointer (&p, true, human); \
		std::stringstream ss; \
		ss << p; \
		return ss.str(); \
	} \
	\
	void pointer(__type *pointer, bool get, bool human) const; \
}; \
__name##Class __name##Instance; \
void __name##Class::pointer(__type *pointer, bool get, bool human) const


/**
 * Create a variable that can be modify in realtime. Don't use this class directly but use the macro NLMISC_VARIABLE
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
/*template <class T>
class CVariable : public ICommand
{
public:
	CVariable (const char *commandName, const char *commandHelp, uint nbMeanValue = 0, bool useConfigFile = false) :
	  NLMISC::ICommand(commandName, commandHelp, "[<value>|stat]"),
	  _Mean(nbMeanValue), UseConfigFile(useConfigFile)
	{
		Type = Variable;
	}
	  
	void set(const T &val)
	{
		_Value = val;
		_Mean.addValue(val);
	}

	const T &get() const
	{
		return _Value;
	}

	string getStat() const
	{
		std::stringstream s;
		s << _CommandName << "=" << _Value;
		if (_Mean.getNumFrame()>0)
		{
			s << " Mean=" << _Mean.getSmoothValue();
			s << " LastValues=";
			for (uint i = 0; i < _Mean.getNumFrame(); i++)
			{
				s << _Mean.getLastFrames()[i];
				if (i < _Mean.getNumFrame()-1)
					s << ",";
			}
		}
		return s.str();
	}

	bool UseConfigFile;

	  CVariable (const char *commandName, const char *commandHelp, T *pointer) : NLMISC::ICommand(commandName, commandHelp, "[<value>]"), _Pointer(pointer) {	Type = Variable; }
	virtual bool execute(const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet)
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
*/

//
//
//
//
//
//
//

class IVariable : public ICommand
{
public:

	IVariable(const char *commandName, const char *commandHelp, const char *commandArgs = "[<value>]", bool useConfigFile = false) :
		ICommand(commandName, commandHelp, commandArgs), _UseConfigFile(useConfigFile)
	{
		Type = Variable;
	}
		  
	virtual void fromString(const std::string &val, bool human) = 0;
	
	virtual std::string toString(bool human) const = 0;

	virtual bool execute(const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human)
	{
		if (args.size() > 1)
			return false;
		
		if (args.size() == 1)
		{
			// set the value
			fromString (args[0], human);
		}
		
		// display the value
		if (quiet)
		{
			log.displayNL(toString(human).c_str());
		}
		else
		{
			log.displayNL("Variable %s = %s", _CommandName.c_str(), toString(human).c_str());
		}
		return true;
	}
	

private:

	bool _UseConfigFile;
};




template <class T>
class CVariablePtr : public IVariable
{
public:

	CVariablePtr (const char *commandName, const char *commandHelp, T *valueptr, bool useConfigFile = false) :
		IVariable(commandName, commandHelp, "[<value>]", useConfigFile), _ValuePtr(valueptr)
	{
	}
	  
	virtual void fromString(const std::string &val, bool human)
	{
		std::stringstream ss (val);
		ss >> *_ValuePtr;
	}
	
	virtual std::string toString(bool human) const
	{
		std::stringstream ss;
		ss << *_ValuePtr;
		return ss.str();
	}
		
private:

	T *_ValuePtr;
};


template <class T>
class CVariable : public IVariable
{
public:

	CVariable(const char *commandName, const char *commandHelp, uint nbMeanValue = 0, bool useConfigFile = false) :
		IVariable(commandName, commandHelp, "[<value>|stat]", useConfigFile), _Mean(nbMeanValue)
	{
	}

	virtual void fromString(const std::string &val, bool human)
	{
		std::stringstream ss (val);
		ss >> _Value;
		_Mean.addValue(val);
	}
	
	virtual std::string toString(bool human) const
	{
		std::stringstream ss;
		ss << _Value;
		return ss.str();
	}
	
	void set(const T &val)
	{
		_Value = val;
		_Mean.addValue(val);
	}
	
	const T &get() const
	{
		return _Value;
	}

	std::string getStat() const
	{
		std::stringstream s;
		s << _CommandName << "=" << _Value;
		if (_Mean.getNumFrame()>0)
		{
			s << " Mean=" << _Mean.getSmoothValue();
			s << " LastValues=";
			for (uint i = 0; i < _Mean.getNumFrame(); i++)
			{
				s << _Mean.getLastFrames()[i];
				if (i < _Mean.getNumFrame()-1)
					s << ",";
			}
		}
		return s.str();
	}
	
	virtual bool execute(const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human)
	{
		if (args.size() > 1)
			return false;
		
		if (args.size() == 1)
		{
			if (args[0] == "stat")
			{
				// display the stat value
				log.displayNL(getStat().c_str());
				return true;
			}
			else
			{
				// set the value
				fromString (args[0], human);
			}
		}

		// display the value
		if (quiet)
		{
			log.displayNL(toString(human).c_str());
		}
		else
		{
			log.displayNL("Variable %s = %s", _CommandName.c_str(), toString(human).c_str());
		}
		return true;
	}
	
private:

	T _Value;
	CValueSmootherTemplate<T> _Mean;
};






} // NLMISC


#endif // NL_VARIABLE_H

/* End of variable.h */
