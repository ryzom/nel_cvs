/* config_file.cpp
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
 * $Id: config_file.cpp,v 1.2 2000/10/04 13:59:03 lecroart Exp $
 *
 * Implementation of CConfigFile.
 */

/// \todo: docs

#include "nel/misc/types_nl.h"

#include <vector>
#include <string>
#include <algorithm>

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "nel/misc/config_file.h"

using namespace std;

extern int cfparse (void *);
extern FILE *cfin;
extern int cf_CurrentLine;

namespace NLMISC
{

char *CConfigFile::CVar::TypeName[] = { "Integer", "String", "Float" };

int CConfigFile::CVar::asInt (int index)
{
	if (Type != T_INT) throw EBadType (Name, Type, T_INT);
	else if (index >= IntValues.size () || index < 0) throw EBadSize (Name, IntValues.size (), index);
	else return IntValues[index];
}

double CConfigFile::CVar::asDouble (int index)
{
	if (Type != T_REAL) throw EBadType (Name, Type, T_REAL);
	else if (index >= RealValues.size () || index < 0) throw EBadSize (Name, RealValues.size (), index);
	else return RealValues[index];
}

float CConfigFile::CVar::asFloat (int index)
{
	return (float) asDouble (index);
}

std::string &CConfigFile::CVar::asString (int index)
{
	if (Type != T_STRING) throw EBadType (Name, Type, T_STRING);
	else if (index >= StrValues.size () || index < 0) throw EBadSize (Name, StrValues.size (), index);
	else return StrValues[index];
}

int CConfigFile::CVar::size ()
{
	switch (Type)
	{
	case T_INT: return IntValues.size ();
	case T_REAL: return RealValues.size ();
	case T_STRING: return StrValues.size ();
	default: return 0;
	}
}

CConfigFile::~CConfigFile ()
{
	vector<CConfigFile *>::iterator it = find (ConfigFiles.begin (), ConfigFiles.end (), this);
	if (it != ConfigFiles.end ())
	{
		ConfigFiles.erase (it);
		printf("rem %s\n", FileName.c_str ());
	}
}

void CConfigFile::parse (const string fileName)
{
	FileName = fileName;
	LastModified = getLastModified ();
	CConfigFile::ConfigFiles.push_back (this);
	printf("add %s\n", FileName.c_str ());
	reparse ();
}

void CConfigFile::reparse ()
{
	cfin = fopen (FileName.c_str (), "r");
	if (cfin != NULL)
	{
		Vars.clear ();
		bool parsingOK = (cfparse (&(Vars)) == 0);
		fclose (cfin);
		if (!parsingOK) throw EParseError (FileName, cf_CurrentLine);
	}
}


CConfigFile::CVar &CConfigFile::getVar (const std::string varName)
{
	for (int i = 0; i < Vars.size(); i++)
	{
		if (Vars[i].Name == varName)
		{
			return Vars[i];
			break;
		}
	}
	throw EUnknownVar (varName);
}

void CConfigFile::print ()
{
	printf("%d results:\n-------------------------------------\n", Vars.size());
	for(int i = 0; i < Vars.size(); i++)
	{
		if (Vars[i].Comp)
		{
			switch (Vars[i].Type)
			{
			case CConfigFile::CVar::T_INT:
			{
				printf("%-20s { ", Vars[i].Name.c_str());
				for (int it=0; it < Vars[i].IntValues.size(); it++)
				{
					printf("'%d' ", Vars[i].IntValues[it]);
				}
				printf ("}\n");
				break;
			}
			case CConfigFile::CVar::T_STRING:
			{
				printf("%-20s { ", Vars[i].Name.c_str());
				for (int st=0; st < Vars[i].StrValues.size(); st++)
				{
					printf("\"%s\" ", Vars[i].StrValues[st].c_str());
				}
				printf ("}\n");
				break;
			}
			case CConfigFile::CVar::T_REAL:
			{
				printf("%-20s { " , Vars[i].Name.c_str());
				for (int rt=0; rt < Vars[i].RealValues.size(); rt++)
				{
					printf("`%f` ", Vars[i].RealValues[rt]);
				}
				printf ("}\n");
				break;
			}
			}
		}
		else
		{
			switch (Vars[i].Type)
			{
			case CConfigFile::CVar::T_INT:
				printf("%-20s '%d'\n", Vars[i].Name.c_str(), Vars[i].IntValues[0]);
				break;
			case CConfigFile::CVar::T_STRING:
				printf("%-20s \"%s\"\n", Vars[i].Name.c_str(), Vars[i].StrValues[0].c_str());
				break;
			case CConfigFile::CVar::T_REAL:
				printf("%-20s `%f`\n", Vars[i].Name.c_str(), Vars[i].RealValues[0]);
				break;
			}
		}
	}
}

vector<CConfigFile *> CConfigFile::ConfigFiles;

uint32 CConfigFile::getLastModified ()
{
#if defined(WIN32)
	struct _stat buf;
#else
	struct stat buf;
#endif

	int result = _stat (FileName.c_str (), &buf);
	if (result != 0) return 0;
	else return buf.st_mtime;
}


void CConfigFile::checkConfigFiles ()
{
	static clock_t LastCheckClock = clock ();
	
	if ((float)(clock () - LastCheckClock)/(float)CLOCKS_PER_SEC < 1.0f) return;

	LastCheckClock = clock ();

	for (vector<CConfigFile *>::iterator it = ConfigFiles.begin (); it != ConfigFiles.end (); it++)
	{
		if ((*it)->LastModified != (*it)->getLastModified ())
		{
			(*it)->reparse ();
		}
	}
}


} // NLMISC
