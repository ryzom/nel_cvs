/** \file config_file.cpp
 * CConfigFile class
 *
 * $Id: config_file.cpp,v 1.48 2003/09/01 15:15:28 lecroart Exp $
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

//#include "../stdmisc.h"

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "nel/misc/file.h"
#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/path.h"

using namespace std;
using namespace NLMISC;

extern void cfrestart (FILE *);	// used to reinit the file
extern int cfparse (void *);	// used to parse the file
//extern FILE *cfin;
extern int cf_CurrentLine;
extern bool cf_OverwriteExistingVariable;
extern CIFile cf_ifile;

// put true if you want that the config file class check type when you call asFunctions
// (for example, check when you call asInt() that the variable is an int).
// when it's false, the function will convert to the wanted type (if he can)
const bool CheckType = false;
bool LoadRoot = false;

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
	Root = false;
}

void CConfigFile::CVar::setAsDouble (double val, int index)
{
	if (Type != T_REAL) throw EBadType (Name, Type, T_REAL);
	else if (index > (int)RealValues.size () || index < 0) throw EBadSize (Name, RealValues.size (), index);
	else if (index == (int)RealValues.size ()) RealValues.push_back(val);
	else RealValues[index] = val;
	Root = false;
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
	Root = false;
}

void CConfigFile::CVar::setAsInt (std::vector<int> vals)
{
	if (Type != T_INT) throw EBadType (Name, Type, T_INT);
	else IntValues = vals;
	Root = false;
}

void CConfigFile::CVar::setAsDouble (std::vector<double> vals)
{
	if (Type != T_REAL) throw EBadType (Name, Type, T_REAL);
	else RealValues = vals;
	Root = false;
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
	Root = false;
}

void CConfigFile::CVar::setAsString (std::vector<std::string> vals)
{
	if (Type != T_STRING) throw EBadType (Name, Type, T_STRING);
	else StrValues = vals;
	Root = false;
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
		default: break;
		}
	}
	return false;
}

bool CConfigFile::CVar::operator!=	(const CVar& var) const
{
	return !(*this==var);
}

void CConfigFile::CVar::add (const CVar &var)
{
	if (Type == var.Type)
	{
		switch (Type)
		{
		case T_INT: IntValues.insert (IntValues.end(), var.IntValues.begin(), var.IntValues.end()); break;
		case T_REAL: RealValues.insert (RealValues.end(), var.RealValues.begin(), var.RealValues.end()); break;
		case T_STRING: StrValues.insert (StrValues.end(), var.StrValues.begin(), var.StrValues.end()); break;
		default: break;
		}
	}
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
	if(fileName.empty())
	{
		nlwarning ("Can't load a empty file name configfile");
		return;
	}

	FileNames.clear ();
	FileNames.push_back (fileName);

	if (_ConfigFiles == NULL)
	{
		_ConfigFiles = new std::vector<CConfigFile *>;
	}
	(*CConfigFile::_ConfigFiles).push_back (this);
	reparse ();

/* 	_FileName.clear ();
	_FileName.push_back (fileName);

	if (_ConfigFiles == NULL)
	{
		_ConfigFiles = new std::vector<CConfigFile *>;
	}
	(*CConfigFile::_ConfigFiles).push_back (this);
	reparse ();

	// If we find a linked config file, load it but don't overload already existant variable
	CVar *var = getVarPtr ("RootConfigFilename");
	if (var)
	{
		string RootConfigFilename = var->asString();
		nlinfo ("RootConfigFilename variable found in the '%s' config file, parse it (%s)", fileName.c_str(), RootConfigFilename.c_str());

		string path = CFile::getPath(fileName);

		if (!path.empty())
			path +=  "/";

		path += RootConfigFilename;

		reparse (path.c_str());
	}
*/
//	print ();
}

bool CConfigFile::loaded()
{
	return !CConfigFile::FileNames.empty();
}

void CConfigFile::reparse (/*const char *filename, bool callingCallback*/)
{
	if (FileNames.empty())
	{
		nlwarning ("Can't reparse config file because file name is empty");
		return;
	}

	string fn = FileNames[0];

	FileNames.clear ();
	LastModified.clear ();

//	clearVars ();

	while (!fn.empty())
	{
		fn = NLMISC::CPath::getFullPath(fn, false);
		nldebug ("Adding config file '%s' in the config file", fn.c_str());
		FileNames.push_back (fn);
		LastModified.push_back (CFile::getFileModificationDate(fn));

		if (cf_ifile.open (fn))
		{
			cfrestart (NULL);
			cf_OverwriteExistingVariable = (FileNames.size()==1);
			LoadRoot = (FileNames.size()>1);
			bool parsingOK = (cfparse (&(_Vars)) == 0);
			cf_ifile.close();
			if (!parsingOK)
			{
				nlwarning ("Parsing error in file %s line %d", fn.c_str(), cf_CurrentLine);
				throw EParseError (fn, cf_CurrentLine);
			}
		}
		else
		{
			nlwarning ("Config file '%s' not found in the path '%s'", fn.c_str(), CPath::getCurrentPath().c_str());
			throw EFileNotFound (fn);
		}
		cf_ifile.close ();

		// If we find a linked config file, load it but don't overload already existant variable
		CVar *var = getVarPtr ("RootConfigFilename");
		if (var)
		{
			string RootConfigFilename = var->asString();

			if (!NLMISC::CFile::fileExists(RootConfigFilename))
			{
				// file is not found, try with the path of the master cfg
				string path = NLMISC::CPath::standardizePath (NLMISC::CFile::getPath(FileNames[0]));
				RootConfigFilename = path + RootConfigFilename;
			}

			if (RootConfigFilename != fn)
			{
				nlinfo ("RootConfigFilename variable found in the '%s' config file, parse the root config file '%s'", fn.c_str(), RootConfigFilename.c_str());
				fn = RootConfigFilename;
			}
			else
				fn.clear ();
		}
		else
			fn.clear ();	
	}

	if (_Callback != NULL) 
		_Callback();

/*	if (filename == NULL)
	{
		_LastModified = getLastModified ();

		nlassert (!_FileName.empty());

		if (cf_ifile.open (_FileName[0]))
		{
			// if we clear all the array, we'll lost the callback on variable and all information
			//		_Vars.clear();
			cfrestart (NULL);
			cf_OverwriteExistingVariable = true;
			LoadRoot = false;
			bool parsingOK = (cfparse (&(_Vars)) == 0);
			cf_ifile.close();
			if (!parsingOK)
			{
				nlwarning ("Parsing error in file %s line %d", _FileName.c_str(), cf_CurrentLine);
				throw EParseError (_FileName, cf_CurrentLine);
			}
		}
		else
		{
			nlwarning ("ConfigFile '%s' not found in the path '%s'", _FileName.c_str(), CPath::getCurrentPath().c_str());
			throw EFileNotFound (_FileName);
		}
	}
	else
	{
		nlassert (strlen(filename)>0);

		// load external config filename, don't overwrite existant variable
		if (cf_ifile.open (filename))
		{
			cfrestart (NULL);
			cf_OverwriteExistingVariable = false;
			LoadRoot = true;
			bool parsingOK = (cfparse (&(_Vars)) == 0);
			cf_ifile.close ();
			if (!parsingOK)
			{
				nlwarning ("Parsing error in file %s line %d", filename, cf_CurrentLine);
				throw EParseError (filename, cf_CurrentLine);
			}
		}
		else
		{
			nlwarning ("RootConfigFilename '%s' not found", _FileName.c_str());
		}
	}

	if (callingCallback)
	{
		if (_Callback != NULL) 
			_Callback();
	}
*/

}



CConfigFile::CVar &CConfigFile::getVar (const std::string &varName)
{
	uint i;
	for (i = 0; i < _Vars.size(); i++)
	{
		// the type could be T_UNKNOWN if we add a callback on this name but this var is not in the config file
		if (_Vars[i].Name == varName && (_Vars[i].Type != CVar::T_UNKNOWN || _Vars[i].Comp))
		{
			return _Vars[i];
			break;
		}
	}

	// if not found, add it in the array if necessary
	for (i = 0; i < UnknownVariables.size(); i++)
		if(UnknownVariables[i] == varName)
			break;
	if (i == UnknownVariables.size())
		UnknownVariables.push_back(varName);

	throw EUnknownVar (FileNames[0], varName);
}


CConfigFile::CVar *CConfigFile::getVarPtr (const std::string &varName)
{
	uint i;
	for (i = 0; i < _Vars.size(); i++)
	{
		// the type could be T_UNKNOWN if we add a callback on this name but this var is not in the config file
		if (_Vars[i].Name == varName && (_Vars[i].Type != CVar::T_UNKNOWN || _Vars[i].Comp))
		{
			return &(_Vars[i]);
		}
	}

	// if not found, add it in the array if necessary
	for (i = 0; i < UnknownVariables.size(); i++)
		if(UnknownVariables[i] == varName)
			break;
	if (i == UnknownVariables.size())
		UnknownVariables.push_back(varName);

	return NULL;
}

bool CConfigFile::exists (const std::string &varName)
{
	for (uint i = 0; i < _Vars.size(); i++)
	{
		// the type could be T_UNKNOWN if we add a callback on this name but this var is not in the config file
		if (_Vars[i].Name == varName && (_Vars[i].Type != CVar::T_UNKNOWN || _Vars[i].Comp))
		{
			return true;
		}
	}
	return false;
}

void CConfigFile::save () const
{
	/*FILE *fp = fopen (_FileName.c_str (), "w");
	if (fp == NULL)
	{
		nlwarning ("Couldn't create %s file", _FileName.c_str ());
		return;
	}

	for(int i = 0; i < (int)_Vars.size(); i++)
	{
		// Not a root value 
		if (!_Vars[i].Root)
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
					break;
				}
				case CConfigFile::CVar::T_STRING:
				{
					for (int st=0; st < (int)_Vars[i].StrValues.size(); st++)
					{
						fprintf(fp, "\"%s\"%s", _Vars[i].StrValues[st].c_str(), st<(int)_Vars[i].StrValues.size()-1?", ":" ");
					}
					break;
				}
				case CConfigFile::CVar::T_REAL:
				{
					for (int rt=0; rt < (int)_Vars[i].RealValues.size(); rt++)
					{
						fprintf(fp, "%.10f%s", _Vars[i].RealValues[rt], rt<(int)_Vars[i].RealValues.size()-1?", ":" ");
					}
					break;
				}
				default: break;
				}
				fprintf(fp, "};\n");
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
				default: break;
				}
			}
		}
	}
	fclose (fp);*/
}

void CConfigFile::display () const
{
	display (InfoLog);
}

void CConfigFile::display (CLog *log) const
{
	createDebug ();

	log->displayRawNL ("Config file %s have %d variables and %d root config file:", FileNames[0].c_str(), _Vars.size(), FileNames.size()-1);
	log->displayRaw ("Root config files: ");
	for(int i = 1; i < (int)FileNames.size(); i++)
	{
		log->displayRaw (FileNames[i].c_str());
	}
	log->displayRawNL ("");
	log->displayRawNL ("------------------------------------------------------");
	for(int i = 0; i < (int)_Vars.size(); i++)
	{
		log->displayRaw ((_Vars[i].Callback==NULL)?"   ":"CB ");
		log->displayRaw ((_Vars[i].Root)?"Root ":"     ");
		if (_Vars[i].Comp)
		{
			switch (_Vars[i].Type)
			{
			case CConfigFile::CVar::T_INT:
			{
				log->displayRaw ("%-20s { ", _Vars[i].Name.c_str());
				for (int it=0; it < (int)_Vars[i].IntValues.size(); it++)
				{
					log->displayRaw ("'%d' ", _Vars[i].IntValues[it]);
				}
				log->displayRawNL ("}");
				break;
			}
			case CConfigFile::CVar::T_STRING:
			{
				log->displayRaw ("%-20s { ", _Vars[i].Name.c_str());
				for (int st=0; st < (int)_Vars[i].StrValues.size(); st++)
				{
					log->displayRaw ("\"%s\" ", _Vars[i].StrValues[st].c_str());
				}
				log->displayRawNL ("}");
				break;
			}
			case CConfigFile::CVar::T_REAL:
			{
				log->displayRaw ("%-20s { " , _Vars[i].Name.c_str());
				for (int rt=0; rt < (int)_Vars[i].RealValues.size(); rt++)
				{
					log->displayRaw ("`%f` ", _Vars[i].RealValues[rt]);
				}
				log->displayRawNL ("}");
				break;
			}
			case CConfigFile::CVar::T_UNKNOWN:
			{
				 log->displayRawNL ("%-20s { }" , _Vars[i].Name.c_str());
				break;
			}
			default:
			{
				log->displayRawNL ("%-20s <default case comp> (%d)" , _Vars[i].Name.c_str(), _Vars[i].Type);
				break;
			}
			}
		}
		else
		{
			switch (_Vars[i].Type)
			{
			case CConfigFile::CVar::T_INT:
				log->displayRawNL ("%-20s '%d'", _Vars[i].Name.c_str(), _Vars[i].IntValues[0]);
				break;
			case CConfigFile::CVar::T_STRING:
				log->displayRawNL ("%-20s \"%s\"", _Vars[i].Name.c_str(), _Vars[i].StrValues[0].c_str());
				break;
			case CConfigFile::CVar::T_REAL:
				log->displayRawNL ("%-20s `%f`", _Vars[i].Name.c_str(), _Vars[i].RealValues[0]);
				break;
			default:
			{
				log->displayRawNL ("%-20s <default case> (%d)" , _Vars[i].Name.c_str(), _Vars[i].Type);
				break;
			}
			}
		}
	}
}

void CConfigFile::setCallback (void (*cb)())
{
	_Callback = cb;
	nlinfo ("Setting callback when the file '%s' is modified externaly", FileNames.empty()?"":FileNames[0].c_str());
}

void CConfigFile::setCallback (const string &VarName, void (*cb)(CConfigFile::CVar &var))
{
	for (vector<CVar>::iterator it = _Vars.begin (); it != _Vars.end (); it++)
	{
		if (VarName == (*it).Name)
		{
			(*it).Callback = cb;
			nlinfo ("Setting callback when the variable '%s' on the file '%s' is modified externaly", VarName.c_str(), FileNames[0].c_str());
			return;
		}
	}
	// VarName doesn't exist, add it now for the future
	CVar Var;
	Var.Name = VarName;
	Var.Callback = cb;
	Var.Type = CVar::T_UNKNOWN;
	_Vars.push_back (Var);
	nlinfo ("Setting callback when the variable '%s' on the file '%s' is modified externaly (actually unknown)", VarName.c_str(), FileNames[0].c_str());
}

// ***************************************************************************


vector<CConfigFile *> *CConfigFile::_ConfigFiles = NULL;

uint32	CConfigFile::_Timeout = 1000;

void CConfigFile::checkConfigFiles ()
{
	if (_ConfigFiles == NULL) return;

	static time_t LastCheckTime = time (NULL);
	if (_Timeout > 0 && (float)(time (NULL) - LastCheckTime)*1000.0f < (float)_Timeout) return;

	LastCheckTime = time (NULL);

	bool needReparse;
	for (vector<CConfigFile *>::iterator it = (*_ConfigFiles).begin (); it != (*_ConfigFiles).end (); it++)
	{
		needReparse = false;
		nlassert ((*it)->FileNames.size() == (*it)->LastModified.size());
		for (uint i = 0; i < (*it)->FileNames.size(); i++)
		{
			if ((*it)->LastModified[i] != CFile::getFileModificationDate((*it)->FileNames[i]))
			{
				needReparse = true;
				(*it)->LastModified[i] = CFile::getFileModificationDate((*it)->FileNames[i]);
			}
		}
		if (needReparse)
		{
			try
			{
				(*it)->reparse ();
			}
			catch (EConfigFile &e)
			{
				nlwarning ("Exception will re-reading modified config file '%s': %s", e.what ());
			}
		}
	}
}

void CConfigFile::setTimeout (uint32 timeout)
{
	_Timeout = timeout;
}

void CConfigFile::clear()
{
	_Vars.clear ();
}

void CConfigFile::clearVars ()
{
	for (vector<CVar>::iterator it = _Vars.begin (); it != _Vars.end (); it++)
	{
		(*it).Type = CVar::T_UNKNOWN;
	}
}

uint CConfigFile::getNumVar () const
{
	return _Vars.size ();
}

CConfigFile::CVar *CConfigFile::getVar (uint varId)
{
	return &(_Vars[varId]);
}

CConfigFile::CVar *CConfigFile::insertVar (const std::string &varName, const CVar &varToCopy)
{
	// Get the var
	CVar *var = getVarPtr (varName);
	if (!var)
	{
		_Vars.push_back (varToCopy);
		var = &(_Vars.back ());
		var->Root = false;
		var->Name = varName;
	}
	return var;
}

} // NLMISC
