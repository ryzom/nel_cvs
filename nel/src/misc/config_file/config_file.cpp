/** \file config_file.cpp
 * CConfigFile class
 *
 * $Id: config_file.cpp,v 1.28 2002/02/28 15:16:40 lecroart Exp $
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

#include "../stdmisc.h"

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "nel/misc/config_file.h"

using namespace std;

extern void cfrestart (FILE *);	// used to reinit the file
extern int cfparse (void *);	// used to parse the file
extern FILE *cfin;
extern int cf_CurrentLine;


// put true if you want that the config file class check type when you call asFunctions
// (for example, check when you call asInt() that the variable is an int).
// when it's false, the function will convert to the wanted type (if he can)
const bool CheckType = false;

namespace NLMISC
{

char *CConfigFile::CVar::TypeName[] = { "Integer", "String", "Float" };

int CConfigFile::CVar::asInt (int index) const
{
	if (CheckType && Type != T_INT) throw EBadType (Name, Type, T_INT);
	switch (Type)
	{
	case T_STRING:
		if (index >= (int)StrValues.size () || index < 0) throw EBadSize (Name, StrValues.size (), index);
		return atoi(StrValues[index].c_str());
	case T_REAL:
		if (index >= (int)RealValues.size () || index < 0) throw EBadSize (Name, RealValues.size (), index);
		return (int)RealValues[index];
	default:
		if (index >= (int)IntValues.size () || index < 0) throw EBadSize (Name, IntValues.size (), index);
		return IntValues[index];
	}
}

double CConfigFile::CVar::asDouble (int index) const
{
	if (CheckType && Type != T_REAL) throw EBadType (Name, Type, T_REAL);
	switch (Type)
	{
	case T_INT:
		if (index >= (int)IntValues.size () || index < 0) throw EBadSize (Name, IntValues.size (), index);
		return (double)IntValues[index];
	case T_STRING:
		if (index >= (int)StrValues.size () || index < 0) throw EBadSize (Name, StrValues.size (), index);
		return atof(StrValues[index].c_str());
	default:
		if (index >= (int)RealValues.size () || index < 0) throw EBadSize (Name, RealValues.size (), index);
		return RealValues[index];
	}
}

float CConfigFile::CVar::asFloat (int index) const
{
	return (float) asDouble (index);
}

const std::string &CConfigFile::CVar::asString (int index) const
{
	if (Type != T_STRING) throw EBadType (Name, Type, T_STRING);
	if (index >= (int)StrValues.size () || index < 0) throw EBadSize (Name, StrValues.size (), index);
	return StrValues[index];
}



void CConfigFile::CVar::setAsInt (int val, int index)
{
	if (Type != T_INT) throw EBadType (Name, Type, T_INT);
	else if (index > (int)IntValues.size () || index < 0) throw EBadSize (Name, IntValues.size (), index);
	else if (index == (int)IntValues.size ()) IntValues.push_back(val);
	else IntValues[index] = val;
}

void CConfigFile::CVar::setAsDouble (double val, int index)
{
	if (Type != T_REAL) throw EBadType (Name, Type, T_REAL);
	else if (index > (int)RealValues.size () || index < 0) throw EBadSize (Name, RealValues.size (), index);
	else if (index == (int)RealValues.size ()) RealValues.push_back(val);
	else RealValues[index] = val;
}

void CConfigFile::CVar::setAsFloat (float val, int index)
{
	setAsDouble (val, index);
}

void CConfigFile::CVar::setAsString (std::string val, int index)
{
	if (Type != T_STRING) throw EBadType (Name, Type, T_STRING);
	else if (index > (int)StrValues.size () || index < 0) throw EBadSize (Name, StrValues.size (), index);
	else if (index == (int)StrValues.size ()) StrValues.push_back(val);
	else StrValues[index] = val;
}

void CConfigFile::CVar::setAsInt (std::vector<int> vals)
{
	if (Type != T_INT) throw EBadType (Name, Type, T_INT);
	else IntValues = vals;
}

void CConfigFile::CVar::setAsDouble (std::vector<double> vals)
{
	if (Type != T_REAL) throw EBadType (Name, Type, T_REAL);
	else RealValues = vals;
}

void CConfigFile::CVar::setAsFloat (std::vector<float> vals)
{
	if (Type != T_REAL) throw EBadType (Name, Type, T_REAL);
	else
	{
		RealValues.clear ();
		RealValues.resize (vals.size ());
		for (uint i = 0; i < vals.size (); i++)
			RealValues[i] = (double)vals[i];
	}
}

void CConfigFile::CVar::setAsString (std::vector<std::string> vals)
{
	if (Type != T_STRING) throw EBadType (Name, Type, T_STRING);
	else StrValues = vals;
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

int CConfigFile::CVar::size () const
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
	if (_ConfigFiles == NULL || (*_ConfigFiles).empty ()) return;

	vector<CConfigFile *>::iterator it = find ((*_ConfigFiles).begin (), (*_ConfigFiles).end (), this);
	if (it != (*_ConfigFiles).end ())
	{
		(*_ConfigFiles).erase (it);
	}

	if ((*_ConfigFiles).empty())
	{
		delete _ConfigFiles;
		_ConfigFiles = NULL;
	}
}

void CConfigFile::load (const string &fileName)
{
 	_FileName = fileName;
	_Callback = NULL;

	if (_ConfigFiles == NULL)
	{
		_ConfigFiles = new std::vector<CConfigFile *>;
	}
	(*CConfigFile::_ConfigFiles).push_back (this);
	reparse ();
}


bool CConfigFile::loaded()
{
	return ( CConfigFile::_FileName != "" );
}


void CConfigFile::reparse ()
{
	_LastModified = getLastModified ();
	cfin = fopen (_FileName.c_str (), "r");
	if (cfin != NULL)
	{
// if we clear all the array, we'll lost the callback on variable and all information
//		_Vars.clear();
		cfrestart (cfin);
		bool parsingOK = (cfparse (&(_Vars)) == 0);
		fclose (cfin);
		if (!parsingOK) throw EParseError (_FileName, cf_CurrentLine);
	}
	else
	{
		nlwarning ("ConfigFile '%s' not found", _FileName.c_str());
	}
}


CConfigFile::CVar &CConfigFile::getVar (const std::string &varName)
{
	for (int i = 0; i < (int)_Vars.size(); i++)
	{
		// the type could be T_UNKNOWN if we add a callback on this name but this var is not in the config file
		if (_Vars[i].Name == varName && (_Vars[i].Type != CVar::T_UNKNOWN || _Vars[i].Comp))
		{
			return _Vars[i];
			break;
		}
	}
	throw EUnknownVar (_FileName, varName);
}


void CConfigFile::save () const
{
	FILE *fp = fopen (_FileName.c_str (), "w");
	if (fp == NULL)
	{
		nlwarning ("Couldn't create %s file", _FileName.c_str ());
		return;
	}

	for(int i = 0; i < (int)_Vars.size(); i++)
	{
		if (_Vars[i].Comp)
		{
			fprintf(fp, "%-20s = { ", _Vars[i].Name.c_str());
			switch (_Vars[i].Type)
			{
			case CConfigFile::CVar::T_INT:
			{
				for (int it=0; it < (int)_Vars[i].IntValues.size(); it++)
				{
					fprintf(fp, "%d%s", _Vars[i].IntValues[it], it<(int)_Vars[i].IntValues.size()-1?", ":" ");
				}
				fprintf(fp, "};\n");
				break;
			}
			case CConfigFile::CVar::T_STRING:
			{
				for (int st=0; st < (int)_Vars[i].StrValues.size(); st++)
				{
					fprintf(fp, "\"%s\"%s", _Vars[i].StrValues[st].c_str(), st<(int)_Vars[i].StrValues.size()-1?", ":" ");
				}
				fprintf(fp, "};\n");
				break;
			}
			case CConfigFile::CVar::T_REAL:
			{
				for (int rt=0; rt < (int)_Vars[i].RealValues.size(); rt++)
				{
					fprintf(fp, "%.10f%s", _Vars[i].RealValues[rt], rt<(int)_Vars[i].RealValues.size()-1?", ":" ");
				}
				fprintf(fp, "};\n");
				break;
			}
			}
		}
		else
		{
			switch (_Vars[i].Type)
			{
			case CConfigFile::CVar::T_INT:
				fprintf(fp, "%-20s = %d;\n", _Vars[i].Name.c_str(), _Vars[i].IntValues[0]);
				break;
			case CConfigFile::CVar::T_STRING:
				fprintf(fp, "%-20s = \"%s\";\n", _Vars[i].Name.c_str(), _Vars[i].StrValues[0].c_str());
				break;
			case CConfigFile::CVar::T_REAL:
				fprintf(fp, "%-20s = %.10f;\n", _Vars[i].Name.c_str(), _Vars[i].RealValues[0]);
				break;
			}
		}
	}
	fclose (fp);
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
			case CConfigFile::CVar::T_UNKNOWN:
			{
				printf("%-20s { }\n" , _Vars[i].Name.c_str());
				break;
			}
			default:
			{
				printf("%-20s <default case>\n" , _Vars[i].Name.c_str());
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
			default:
			{
				printf("%-20s <default case>\n" , _Vars[i].Name.c_str());
				break;
			}
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
	Var.Type = CVar::T_UNKNOWN;
	_Vars.push_back (Var);
}

void CConfigFile::setLastModifiedNow ()
{
	_LastModified = getLastModified ();
}


// ***************************************************************************


vector<CConfigFile *> *CConfigFile::_ConfigFiles = NULL;

uint32	CConfigFile::_Timeout = 1000;

uint32 CConfigFile::getLastModified ()
{
#if defined (NL_OS_WINDOWS)
	struct _stat buf;
	int result = _stat (_FileName.c_str (), &buf);
#elif defined (NL_OS_UNIX)
	struct stat buf;
	int result = stat (_FileName.c_str (), &buf);
#endif

	if (result != 0) return 0;
	else return buf.st_mtime;
}


void CConfigFile::checkConfigFiles ()
{
	if (_ConfigFiles == NULL) return;

	static time_t LastCheckTime = time (NULL);
	if (_Timeout > 0 && (float)(time (NULL) - LastCheckTime)*1000.0f < (float)_Timeout) return;

	LastCheckTime = time (NULL);

	for (vector<CConfigFile *>::iterator it = (*_ConfigFiles).begin (); it != (*_ConfigFiles).end (); it++)
	{
		if ((*it)->_LastModified != (*it)->getLastModified ())
		{
			try
			{
				(*it)->reparse ();
				if ((*it)->_Callback != NULL) (*it)->_Callback();
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
