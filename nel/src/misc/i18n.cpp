/** \file i18n.cpp
 * Internationalisation
 *
 * $Id: i18n.cpp,v 1.5 2000/11/23 14:32:39 coutelas Exp $
 *
 * \todo ace: manage unicode format
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

#include "nel/misc/i18n.h"

#include <sstream>

using namespace std;

namespace NLMISC {

const char						*CI18N::_LanguageFiles[] = { "english", "french", "german" };

map<string, string>				 CI18N::_StrMap;
bool							 CI18N::_StrMapLoaded = false;
string							 CI18N::_FileName;

vector<string>					 CI18N::_LanguageNames;
bool							 CI18N::_LanguagesNamesLoaded = false;

void CI18N::skipComment(IStream &is, int &line)
{
	// the first '/' is already eated
	char c;
	bool longcomment;

	is.serial (c);
	if (c == '/') longcomment = false;
	else if (c == '*') longcomment = true;

	do
	{
		is.serial (c);
		if (!longcomment && c == '\n')
		{
			line++;
			return;
		}
		if (longcomment && c == '*')
		{
			is.serial (c);
			if (c == '/') return;
		}
	}
	while (true);
}

char CI18N::skipWS(IStream &is, int &line)
{
	char c;
	do
	{
		is.serial (c);
		if (c == '\n') line++;
		if (c == '/')
		{
			skipComment (is, line);
			is.serial (c);
		}
	}
	while (isspace (c));
	return c;
}


void CI18N::createLanguageFile (uint32 lid)
{
	nlassert (lid >= 0 && lid < sizeof (_LanguageFiles)/sizeof(_LanguageFiles[0]));
	
	string fn = _LanguageFiles[lid];
	fn += ".txt";

	// write the new string in the file
	COFile cof;
	nlverify (cof.open (fn, true, true));

	stringstream ss2;
	ss2 << "\"" << _LanguageFiles[lid] << "\"" << endl;
	cof.serialBuffer((uint8 *)(ss2.str().c_str()), ss2.str().size());
	cof.close ();
}

void CI18N::createLanguageEntry (const string &lval, const string &rval)
{
	for (int i = 0; i < sizeof(_LanguageFiles)/sizeof(_LanguageFiles[0]); i++)
	{
		COFile cof;
		string fn = _LanguageFiles[i];
		fn += ".txt";

		nlverify (cof.open (fn, true, true));

		stringstream ss2;
		ss2 << "\"";
		for (sint i = 0; i < (sint) lval.size (); i++)
		{
			if (lval[i] == '"')
				ss2 << '\\';
			ss2 << lval[i];
		}
		ss2 << "\" = \"";
		for (sint i2 = 0; i2 < (sint) rval.size(); i2++)
		{
			if (rval[i2] == '"')
				ss2 << '\\';
			ss2 << rval[i2];
		}
		ss2 << "\"" << endl;
		cof.serialBuffer((uint8 *)(ss2.str().c_str()), ss2.str().size());
		cof.close ();
	}
}

void CI18N::load (uint32 lid)
{
	nlassert (lid >= 0 && lid < sizeof (_LanguageFiles)/sizeof(_LanguageFiles[0]));

	_FileName = _LanguageFiles[lid];
	_FileName += ".txt";

	if (_StrMapLoaded)	_StrMap.clear ();
	else				_StrMapLoaded = true;
	
	CIFile cf;
	// if the file does not exist, it'll be create automatically
	if (!cf.open (_FileName, true))
	{
		nlwarning ("Could not open file \"%s\" (this file should contain the %s language (lid:%d))", _FileName.c_str (), _LanguageNames[lid].c_str (), lid);
		createLanguageFile (lid);
		return;
	}
	nldebug ("Loading file \"%s\" (this file should contain the %s language (lid:%d))", _FileName.c_str (), _LanguageNames[lid].c_str (), lid);

	bool startstr = false, equal = false, second = false;
	int line = 1;
	try
	{
		char c;
		// get the language name
		c = skipWS (cf, line);
		if (c != '"')
		{
			nlerror ("open '\"' missing in \"%s\" line %d", _FileName.c_str(), line);
		}
		do
		{
			cf.serial (c);
			if (c == '\\')
			{
				cf.serial (c);
			}
			else if (c == '"') break;
			else if (c == '\n') line++;
		}
		while (true);

		while (true)
		{
			string codstr, trsstr;
			char c;

			codstr = trsstr = "";

			// get the coder string
			c = skipWS (cf, line);
			if (c != '"')
			{
				nlerror ("open '\"' missing in \"%s\" line %d", _FileName.c_str(), line);
			}
			startstr = true;
			do
			{
				cf.serial (c);
				if (c == '\\')
				{
					cf.serial (c);
				}
				else if (c == '"') break;
				else if (c == '\n') line++;
				codstr += c;
			}
			while (true);
			startstr = false;

			equal = true;
			// get the '='
			c = skipWS (cf, line);
			if (c != '=')
			{
				nlerror ("'=' missing in \"%s\" line %d", _FileName.c_str(), line);
			}
			equal = false;
			
			second = true;
			// get the translated string
			c = skipWS (cf, line);
			if (c != '"')
			{
				nlerror ("open '\"' missing in \"%s\" line %d", _FileName.c_str(), line);
			}
			startstr = true;
			do
			{
				cf.serial (c);
				if (c == '\\')
				{
					cf.serial (c);
				}
				else if (c == '"') break;
				else if (c == '\n') line++;
				trsstr += c;
			}
			while (true);
			startstr = false;
			second = false;

			// Insert the node.
			pair<ItStrMap, bool> pr;
			pr = _StrMap.insert (ValueStrMap (codstr, trsstr));
			if (!pr.second)
			{
				nlwarning ("already exists, ignored");
			}
		}
	}
	catch (EReadError)
	{
		// always comes here when it's the end of file
		if (startstr)
		{
			nlerror ("a string didn't have the close '\"' in \"%s\" line %d", _FileName.c_str(), line);
		}
		if (equal)
		{
			nlerror ("'=' missing in \"%s\" line %d", _FileName.c_str(), line);
		}
		if (second)
		{
			nlerror ("open '\"' missing in \"%s\" line %d", _FileName.c_str(), line);
		}
		cf.close ();
	}
}

const string &CI18N::get (const char *str)
{
	nlassert (_StrMapLoaded);

	ItStrMap it = _StrMap.find (str);

	if (it == _StrMap.end ())
	{
		// str not found, add it in the map and in the file
		stringstream ss;
		ss << "<Not Translated>:" << str;

		pair<ItStrMap, bool> pr;
		pr = _StrMap.insert (ValueStrMap (str, ss.str()));
		nlassert (pr.second);
		it = pr.first;

		// write the new string in all files
		createLanguageEntry (str, ss.str());

		// warn the user
		nlwarning ("\"%s\" is not in the \"%s\" language file, I add in all languages files.", str, _FileName.c_str());
	}

	return it->second;
}

const vector<string> &CI18N::getLanguageNames()
{
	CIFile cf;

	if (!_LanguagesNamesLoaded)
	{
		for (int i = 0; i < sizeof(_LanguageFiles)/sizeof(_LanguageFiles[0]); i++)
		{
			string fn = _LanguageFiles[i];
			fn += ".txt";

			string lg;

			if (!cf.open (fn, true))
			{
				nlwarning ("Could not open file \"%s\" (lid:%d)", fn.c_str(), i);
				createLanguageFile (i);
				lg = _LanguageFiles[i];
			}
			else
			{
				int line = 1;
				try
				{
					char c;
					// get the language name
					c = skipWS (cf, line);
					if (c != '"')
					{
						nlerror ("open '\"' missing in \"%s\" line %d", fn.c_str(), line);
					}
					do
					{
						cf.serial (c);
						if (c == '\\')
						{
							cf.serial (c);
						}
						else if (c == '"') break;
						else if (c == '\n') line++;
						lg += c;
					}
					while (true);
				}
				catch (EReadError)
				{
					nlerror ("Missing the language name in the beginning of the file %s", fn.c_str());
				}
				cf.close ();
			}
			// add the language name
			_LanguageNames.push_back (lg);
			nldebug ("add %d %s %s -> %s", i, fn.c_str (), _LanguageFiles[i], lg.c_str ());
		}
		_LanguagesNamesLoaded = true;
	}
	return _LanguageNames;
}


} // NLMISC
