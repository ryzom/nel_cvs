/** \file config_file.h
 * CConfigFile class
 *
 * $Id: config_file.h,v 1.10 2000/11/08 15:07:00 lecroart Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#ifndef NL_CONFIG_FILE_H
#define NL_CONFIG_FILE_H

#include "nel/misc/types_nl.h"

#include <vector>
#include <string>
#include <stdio.h>

namespace NLMISC
{

/**
 * CConfigFile class. Useful when you want to have a configuration file with variables.
 * It manages integers, real (double), and string basic types. A variable can be an array of
 * basic type. In this case, all elements of the array must be the same type.
 *
 * Example:
 *\code
 * try
 * {
 * 	CConfigFile cf;
 *
 * 	// Load and parse "test.txt" file
 *  cf.load ("test.txt");
 *
 *	// Attach a callback to the var1 variable. When the var1 will changed, this cvar1cb function will be called
 *	cf.setCallback ("var1", var1cb);
 * 
 *	// Get the foo variable (suppose it's a string variable)
 *	CConfigFile::CVar &foo = cf.getVar ("foo");
 * 
 *	// Display the content of the variable
 *	printf ("foo = %s\n", foo.asString ().c_str ());
 * 
 * 	// Get the bar variable (suppose it's an array of int)
 * 	CConfigFile::CVar &bar = cf.getVar ("bar");
 * 
 * 	// Display the content of the all elements of the bar variable
 * 	printf ("bar have %d elements : \n", bar.size ());
 * 	for (int i = 0; i < bar.size (); i++)
 * 		printf ("%d ", bar.asInt (i));
 * 	printf("\n");
 * }
 * catch (EConfigFile &e)
 * {
 *	// Something goes wrong... catch that
 * 	printf ("%s\n", e.what ());
 * }
 *\endcode
 *
 * Example of config file:
 *\code
 * // one line comment
 * / * big comment
 *     on more than one line * /
 * 
 * var1 = 123;                           // var1  type:int,         value:123
 * var2 = "456.25";                      // var2  type:string,      value:"456.25"
 * var3 = 123.123;                       // var3  type:real,        value:123.123
 * 
 * // the resulting type is type of the first left value
 * var4 = 123.123 + 2;                   // var4  type:real,        value:125.123
 * var5 = 123 + 2.1;                     // var5  type:int,         value:125
 * 
 * var6 = (-112+1) * 3 - 14;             // var6  type:int,         value:-347
 * 
 * var7 = var1 + 1;                      // var7  type:int,         value:124
 * 
 * var8 = var2 + 10;                     // var8  type:string,      value:456.2510 (convert 10 into a string and concat it)
 * var9 = 10.15 + var2;                  // var9  type:real,        value:466.4 (convert var2 into a real and add it)
 * 
 * var10 = { 10.0, 51.1 };               // var10 type:realarray,   value:{10.0,51.1}
 * var11 = { "str1", "str2", "str3" };   // var11 type:stringarray, value:{"str1", "str2", "str3"}
 * 
 * var12 = { 10+var1, var1-var7 };       // var12 type:intarray,    value:{133,-1}
 *\endcode
 *
 * Operators are '+', '-', '*', '/'.
 * You can't use operators on a array variable, for example, you can't do \cvar13=var12+1.
 * If you have 2 variables with the same name, the first value will be remplaced by the second one.
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CConfigFile
{
public:

	/**
	 * CVar class. Used by CConfigFile. A CVar is return when you want to have a variable.
	 *
	 * Example: see the CConfigFile example
	 *
	 * \author Vianney Lecroart
	 * \author Nevrax France
	 * \date 2000
	 */
	struct CVar
	{
	public:

		/// \name Access to the variable content.
		//@{
		/// Get the content of the variable as an integer
		int					 asInt		(int index=0) const;
		/// Get	the content of the variable as a double
		double				 asDouble	(int index=0) const;
		/// Get the content of the variable as a float
		float				 asFloat	(int index=0) const;
		/// Get the content of the variable as a STL string
		const std::string	&asString	(int index=0) const;
		//@}

		bool		operator==	(const CVar& var) const;
		bool		operator!=	(const CVar& var) const;
		
		// Get the size of the variable. It's the number of element of the array or 1 if it's not an array.
		int			 size ();

		/// \name Internal use
		//@{
		static char *TypeName[];

		enum TVarType { T_INT, T_STRING, T_REAL };

		std::string					Name;
		TVarType					Type;
		bool						Comp;
		std::vector<int>			IntValues;
		std::vector<double>			RealValues;
		std::vector<std::string>	StrValues;
		void						(*Callback)(CVar &var);
		//@}
	};

	virtual ~CConfigFile ();

	/// Get a variable with the variable name
	const CVar &getVar (const std::string &varName) const;

	/// load and parse the file
	void load (const std::string &fileName);

	/// reload and reparse the file
	void reparse ();

	/// display all variables with nlinfo (debug use)
	void print () const;

	/// set a callback function that is called when the config file is modified
	void setCallback (void (*cb)());

	/// set a callback function to a variable, it will be called when this variable is modified
	void setCallback (const std::string &VarName, void (*cb)(CConfigFile::CVar &var));

	/// set the time between 2 file checking (default value is 1 second)
	/// \param timeout time in millisecond, if timeout=0, the check will be made each "frame"
	static void CConfigFile::setTimeout (uint32 timeout);

	/// Internal use only
	static void checkConfigFiles ();

private:

	/// Internal use only
	void (*_Callback)();

	/// Internal use only
	std::vector<CVar>	_Vars;

	/// Internal use only
	std::string _FileName;

	/// Internal use only
	uint32	getLastModified ();
	/// Internal use only
	uint32	_LastModified;

	static uint32	_Timeout;

	static std::vector<CConfigFile *> _ConfigFiles;
};

struct EConfigFile : public Exception
{
	virtual const char	*what () const throw () { static char str[1024]; sprintf (str, "Unknown Config File Exception"); return str; }
};

struct EBadType : public EConfigFile
{
	int VarType;
	int WantedType;
	std::string VarName;
	EBadType (const std::string &varName, int varType, int wantedType) : VarName(varName), VarType (varType), WantedType (wantedType) {}
	virtual const char	*what () const throw () { static char str[1024]; sprintf (str, "Bad variable type, variable \"%s\" is a %s and not a %s", VarName.c_str (), CConfigFile::CVar::TypeName[VarType], CConfigFile::CVar::TypeName[WantedType]); return str; }
};

struct EBadSize : public EConfigFile
{
	int VarSize;
	int VarIndex;
	std::string VarName;
	EBadSize (const std::string &varName, int varSize, int varIndex) : VarName(varName), VarSize (varSize), VarIndex (varIndex) {}
	virtual const char	*what () const throw () { static char str[1024]; sprintf (str, "Trying to access to the index %d but the variable \"%s\" size is %d", VarIndex, VarName.c_str (), VarSize); return str; }
};

struct EUnknownVar : public EConfigFile
{
	std::string VarName;
	EUnknownVar (const std::string &varName) : VarName(varName) {}
	virtual const char	*what () const throw () { static char str[1024]; sprintf (str, "Unknown variable \"%s\"", VarName.c_str ()); return str; }
};

struct EParseError : public EConfigFile
{
	std::string FileName;
	int CurrentLine;
	EParseError (const std::string &fileName, int currentLine) : FileName(fileName), CurrentLine (currentLine) {}
	virtual const char	*what () const throw () { static char str[1024]; sprintf (str, "Parse error on the \"%s\" file, line %d", FileName.c_str (), CurrentLine); return str; }
};

struct EFileNotFound : public EConfigFile
{
	std::string FileName;
	EFileNotFound (const std::string &fileName, int currentLine) : FileName(fileName) {}
	virtual const char	*what () const throw () { static char str[1024]; sprintf (str, "File \"%s\" not found", FileName.c_str ()); return str; }
};

} // NLMISC

#endif // NL_CONFIG_FILE_H

/* End of config_file.h */
