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
 * $Id: config_file.cpp,v 1.7 2000/10/09 10:37:12 lecroart Exp $
 *
 * Implementation of CConfigFile.
 */

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"

#include <vector>
#include <string>
#include <algorithm>

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "nel/misc/config_file.h"
#include "nel/misc/debug.h"

using namespace std;

extern int cfparse (void *);
extern FILE *cfin;
extern int cf_CurrentLine;

namespace NLMISC
{

char *CConfigFile::CVar::TypeName[] = { "Integer", "String", "Float" };

int CConfigFile::CVar::asInt (int index) const
{
	if (Type != T_INT) throw EBadType (Name, Type, T_INT);
	else if (index >= (int)IntValues.size () || index < 0) throw EBadSize (Name, IntValues.size (), index);
	else return IntValues[index];
}

double CConfigFile::CVar::asDouble (int index) const
{
	if (Type != T_REAL) throw EBadType (Name, Type, T_REAL);
	else if (index >= (int)RealValues.size () || index < 0) throw EBadSize (Name, RealValues.size (), index);
	else return RealValues[index];
}

float CConfigFile::CVar::asFloat (int index) const
{
	return (float) asDouble (index);
}

const std::string &CConfigFile::CVar::asString (int index) const
{
	if (Type != T_STRING) throw EBadType (Name, Type, T_STRING);
	else if (index >= (int)StrValues.size () || index < 0) throw EBadSize (Name, StrValues.size (), index);
	else return StrValues[index];
}

bool CConfigFile::CVar::operator==	(const CVar& var) const
{
	if (Type == var.Type)
	{
		switch (Type)
		{
		case T_INT: return IntValues == var.IntValues; break;
		case T_REAL: return RealValues == var.RealValues; break;
		case T_STRING: return StrValues == var.StrValues; break;
		}
	}
	return false;
}

bool CConfigFile::CVar::operator!=	(const CVar& var) const
{
	return !(*this==var);
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
	if (_ConfigFiles.size () == 0) return;
	vector<CConfigFile *>::iterator it = find (_ConfigFiles.begin (), _ConfigFiles.end (), this);
	if (it != _ConfigFiles.end ())
	{
		_ConfigFiles.erase (it);
	}
}

void CConfigFile::parse (const string &fileName)
{
	_FileName = fileName;
	CConfigFile::_ConfigFiles.push_back (this);
	reparse ();
}

void CConfigFile::reparse ()
{
	_LastModified = getLastModified ();
	cfin = fopen (_FileName.c_str (), "r");
	if (cfin != NULL)
	{
		bool parsingOK = (cfparse (&(_Vars)) == 0);
		fclose (cfin);
		if (!parsingOK) throw EParseError (_FileName, cf_CurrentLine);
	}
}


const CConfigFile::CVar &CConfigFile::getVar (const std::string &varName) const
{
	for (int i = 0; i < (int)_Vars.size(); i++)
	{
		if (_Vars[i].Name == varName)
		{
			return _Vars[i];
			break;
		}
	}
	throw EUnknownVar (varName);
}

void CConfigFile::print () const
{
	printf ("%d results:\n", _Vars.size());
	printf ("-------------------------------------\n");
	for(int i = 0; i < (int)_Vars.size(); i++)
	{
		printf ((_Vars[i].Callback==NULL)?"   ":"CB ");
		if (_Vars[i].Comp)
		{
			switch (_Vars[i].Type)
			{
			case CConfigFile::CVar::T_INT:
			{
				printf("%-20s { ", _Vars[i].Name.c_str());
				for (int it=0; it < (int)_Vars[i].IntValues.size(); it++)
				{
					printf("'%d' ", _Vars[i].IntValues[it]);
				}
				printf ("}\n");
				break;
			}
			case CConfigFile::CVar::T_STRING:
			{
				printf("%-20s { ", _Vars[i].Name.c_str());
				for (int st=0; st < (int)_Vars[i].StrValues.size(); st++)
				{
					printf("\"%s\" ", _Vars[i].StrValues[st].c_str());
				}
				printf ("}\n");
				break;
			}
			case CConfigFile::CVar::T_REAL:
			{
				printf("%-20s { " , _Vars[i].Name.c_str());
				for (int rt=0; rt < (int)_Vars[i].RealValues.size(); rt++)
				{
					printf("`%f` ", _Vars[i].RealValues[rt]);
				}
				printf ("}\n");
				break;
			}
			}
		}
		else
		{
			switch (_Vars[i].Type)
			{
			case CConfigFile::CVar::T_INT:
				printf("%-20s '%d'\n", _Vars[i].Name.c_str(), _Vars[i].IntValues[0]);
				break;
			case CConfigFile::CVar::T_STRING:
				printf("%-20s \"%s\"\n", _Vars[i].Name.c_str(), _Vars[i].StrValues[0].c_str());
				break;
			case CConfigFile::CVar::T_REAL:
				printf("%-20s `%f`\n", _Vars[i].Name.c_str(), _Vars[i].RealValues[0]);
				break;
			}
		}
	}
}

void CConfigFile::setCallback (void (*cb)())
{
	if (cb == NULL) return;

	_Callback = cb;
}

void CConfigFile::setCallback (const string &VarName, void (*cb)(CConfigFile::CVar &var))
{
	if (cb == NULL) return;

	for (vector<CVar>::iterator it = _Vars.begin (); it != _Vars.end (); it++)
	{
		if (VarName == (*it).Name)
		{
			(*it).Callback = cb;
			return;
		}
	}
	// VarName doesn't exist, add it now for the futur
	CVar Var;
	Var.Name = VarName;
	Var.Callback = cb;
	_Vars.push_back (Var);
}


// ***************************************************************************


vector<CConfigFile *> CConfigFile::_ConfigFiles;
uint32	CConfigFile::_Timeout = 1000;

uint32 CConfigFile::getLastModified ()
{
#if defined(WIN32)
	struct _stat buf;
#else
	struct stat buf;
#endif

	int result = _stat (_FileName.c_str (), &buf);
	if (result != 0) return 0;
	else return buf.st_mtime;
}


void CConfigFile::checkConfigFiles ()
{
	static clock_t LastCheckClock = clock ();
	
	if (_Timeout > 0 && (float)(clock () - LastCheckClock)/(float)CLOCKS_PER_SEC < (float)_Timeout) return;

	LastCheckClock = clock ();

	for (vector<CConfigFile *>::iterator it = _ConfigFiles.begin (); it != _ConfigFiles.end (); it++)
	{
		if ((*it)->_LastModified != (*it)->getLastModified ())
		{
			if ((*it)->_Callback != NULL) (*it)->_Callback();
			try
			{
				(*it)->reparse ();
			}
			catch (EConfigFile &ee)
			{
				nlwarning (ee.what ());
			}
		}
	}
}

void CConfigFile::setTimeout (uint32 timeout)
{
	nlassert (timeout>=0);
	_Timeout = timeout;
}

} // NLMISC
