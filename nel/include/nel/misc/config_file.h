/* config_file.h
 *
 * Copyright, 2000 Nevrax Ltd.
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

/*
 * $Id: config_file.h,v 1.1 2000/10/04 09:38:41 lecroart Exp $
 *
 * Manage a configuration files
 */

#ifndef NL_CONFIG_FILE_H
#define NL_CONFIG_FILE_H

#include "nel/misc/types_nl.h"

#include <vector>
#include <string>

namespace NLMISC
{

/**
 * ConfigFile class. Useful when you want to have a configuration file with variables.
 * It manages integers, real (double), and string basic types. A variable can be an array of
 * basic type. In this case, all elements of the array must be the same type.
 *
 * Sample:
 *\code
	try
	{
		// Load and parse "test.txt" file
		CConfigFile cf ("test.txt");

		// Get the foo variable (suppose it's a string variable)
		CConfigFile::CVar &foo = cf.getVar ("foo");

		// Display the content of the variable
		printf ("foo = %s\n", foo.asString ().c_str ());

		// Get the bar variable (suppose it's an array of int)
		CConfigFile::CVar &bar = cf.getVar ("bar");

		// Display the content of the all elements of the bar variable
		printf ("bar have %d elements : \n", bar.size ());
		for (int i = 0; i < bar.size (); i++)
			printf ("%d ", bar.asInt (i));
		printf("\n");
	}
	catch (EConfigFile &e)
	{
		// Something goes wrong... catch that
		printf ("%s\n", e.what ());
	}
 *\endcode
 *
 * Sample of config file:
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

	struct CVar
	{
	public:

		int			 asInt		(int Index=0);

		double		 asDouble	(int Index=0);
		float		 asFloat	(int Index=0);

		std::string	&asString	(int Index=0);

		int			 size ();

		static char *TypeName[];

		enum TVarType { T_INT, T_STRING, T_REAL };

		std::string					Name;
		TVarType					Type;
		bool						Comp;
		std::vector<int>			IntValues;
		std::vector<double>			RealValues;
		std::vector<std::string>	StrValues;

	};

	std::vector<CVar>	Vars;

	bool	parsingOK;
	
	CConfigFile (const std::string FileName);

	CVar &getVar (const std::string VarName);

	void print ();
};

struct EConfigFile : public Exception
{
	virtual const char	*what () const throw () { static char str[1024]; sprintf (str, "Unknown Config File Exception"); return str; }
};

struct EBadType : public EConfigFile
{
	int varType;
	int wantedType;
	std::string varName;
	EBadType (std::string VarName, int VarType, int WantedType) : varName(VarName), varType (VarType), wantedType (WantedType) {}
	virtual const char	*what () const throw () { static char str[1024]; sprintf (str, "Bad variable type, variable \"%s\" is a %s and not a %s", varName.c_str (), CConfigFile::CVar::TypeName[varType], CConfigFile::CVar::TypeName[wantedType]); return str; }
};

struct EBadSize : public EConfigFile
{
	int varSize;
	int varIndex;
	std::string varName;
	EBadSize (std::string VarName, int VarSize, int VarIndex) : varName(VarName), varSize (VarSize), varIndex (VarIndex) {}
	virtual const char	*what () const throw () { static char str[1024]; sprintf (str, "Trying to access to the index %d but the variable \"%s\" size is %d", varIndex, varName.c_str (), varSize); return str; }
};

struct EUnknownVar : public EConfigFile
{
	std::string varName;
	EUnknownVar (std::string VarName) : varName(VarName) {}
	virtual const char	*what () const throw () { static char str[1024]; sprintf (str, "Unknown variable \"%s\"", varName.c_str ()); return str; }
};

struct EParseError : public EConfigFile
{
	std::string fileName;
	int currentLine;
	EParseError (std::string FileName, int CurrentLine) : fileName(FileName), currentLine (CurrentLine) {}
	virtual const char	*what () const throw () { static char str[1024]; sprintf (str, "Parse error on the \"%s\" file, line %d", fileName.c_str (), currentLine); return str; }
};


} // NLMISC

#endif // NL_CONFIG_FILE_H

/* End of config_file.h */
