/** \file varpath.cpp
 * use to manage variable path (ie: [serv1,serv2].*.*.var)
 *
 * $Id: varpath.cpp,v 1.2 2002/11/08 13:28:21 lecroart Exp $
 *
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NeL Network Services.
 * NEVRAX NeL Network Services is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX NeL Network Services is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "stdnet.h"

#include "nel/misc/types_nl.h"

#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <vector>
#include <map>

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/misc/log.h"

#include "nel/net/varpath.h"


//
// Namespaces
//

using namespace std;
using namespace NLMISC;


//
// Variables
//



//
// Functions
//

/**
 *
 * VarPath ::= [bloc '.']* bloc
 * bloc    ::= '[' [VarPath ',']* VarPath ']'  |  name
 * name    ::= [ascii]* ['*']
 *
 *
 */
/*bool CVarPath::getDest (uint level, vector<string> &dest)
{
	return true;
}*/

string CVarPath::getToken ()
{
	string res;
	
	if (TokenPos >= RawVarPath.size())
		return res;

	res += RawVarPath[TokenPos];

	switch (RawVarPath[TokenPos++])
	{
	case '.': case '*': case '[': case ']': case ',': case '=': break;
	default :
		{
			while (TokenPos < RawVarPath.size() && RawVarPath[TokenPos] != '.' && RawVarPath[TokenPos] != '*' && RawVarPath[TokenPos] != '[' && RawVarPath[TokenPos] != ']' && RawVarPath[TokenPos] != ',' && RawVarPath[TokenPos] != '=')
			{
				res += RawVarPath[TokenPos++];
			}
			break;
		}
	}
	return res;
}


void CVarPath::decode ()
{
	vector<string> dest;
	TokenPos = 0;
	Destination.clear ();

	string val = getToken ();

	if (val == "[" )
	{
		do
		{
			uint osbnb = 0;
			string d;
			do
			{
				val = getToken ();
				if (val == "[")
					osbnb++;

				if (osbnb == 0 && (val == "," || val == "]"))
					break;

				if (val == "]")
					osbnb--;

				d += val;
			}
			while (true);
			dest.push_back (d);
			if (val == "]")
				break;
		}
		while (true);
	}
	else if (val != "." && val != "," && val != "]")
	{
		dest.push_back (val);
	}
	else
	{
		nlwarning ("Malformated VarPath '%s' before position %d", RawVarPath.c_str (), TokenPos);
		return;
	}

	// must the a . or end of string
	val = getToken ();
	if (val != "." && val != "" && val != "=")
	{
		nlwarning ("Malformated VarPath '%s' before position %d", RawVarPath.c_str (), TokenPos);
		return;
	}

	for (uint i = 0; i < dest.size(); ++i)
	{
		string srv, var;
		uint pos;
		
		if ((pos = dest[i].find ('.')) != string::npos)
		{
			srv = dest[i].substr(0, pos);
			var = dest[i].substr(pos+1);
			if (TokenPos < RawVarPath.size())
				var += val + RawVarPath.substr (TokenPos);
		}
		else
		{
			srv = dest[i];
			if (val == "=")
			{
				srv += val + RawVarPath.substr (TokenPos);
				var = "";
			}
			else
				var = RawVarPath.substr (TokenPos);
		}

		Destination.push_back (make_pair(srv, var));
	}

	display ();
}

void CVarPath::display ()
{
	nlinfo ("VarPath dest = %d", Destination.size ());
	for (uint i = 0; i < Destination.size (); i++)
	{
		nlinfo (" > '%s' '%s'", Destination[i].first.c_str(), Destination[i].second.c_str());
	}
}

NLMISC_COMMAND (varPath, "Test a varpath (for debug purpose)", "<rawvarpath>")
{
	if(args.size() != 1) return false;

	CVarPath vp (args[0]);

	log.displayNL ("VarPath contains %d destination", vp.Destination.size ());
	for (uint i = 0; i < vp.Destination.size (); i++)
	{
		log.displayNL ("Dest '%s' value '%s'", vp.Destination[i].first.c_str(), vp.Destination[i].second.c_str());
	}
	log.displayNL ("End of varpath");
	
	return true;
}
