/** \file i18n.cpp
 * Internationalisation
 *
 * $Id: i18n.cpp,v 1.21 2002/06/24 10:23:27 lecroart Exp $
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


#include "stdmisc.h"

#include "nel/misc/i18n.h"

using namespace std;

namespace NLMISC {

const char						*CI18N::_LanguageFiles[] = { "english", "french" };

map<string, ucstring>			 CI18N::_StrMap;
bool							 CI18N::_StrMapLoaded = false;
string							 CI18N::_Path = "";
string							 CI18N::_FileName = "";

vector<ucstring>				 CI18N::_LanguageNames;
bool							 CI18N::_LanguagesNamesLoaded = false;

ucchar CI18N::eatChar (IStream &is)
{
	uint8 c;
	ucchar code;
	sint iterations;

	is.serial (c);
	code = c;

	if ((code & 0xFE) == 0xFC)
	{
		code &= 0x01;
		iterations = 5;
	}
	else if ((code & 0xFC) == 0xF8)
	{
		code &= 0x03;
		iterations = 4;
	}
	else if ((code & 0xF8) == 0xF0)
	{
		code &= 0x07;
		iterations = 3;
	}
	else if ((code & 0xF0) == 0xE0)
	{
		code &= 0x0F;
		iterations = 2;
	}
	else if ((code & 0xE0) == 0xC0)
	{
		code &= 0x1F;
		iterations = 1;
	}
	else if ((code & 0x80) == 0x80)
	{
		nlerror ("CI18N::eatChar(): Invalid UTF-8 character");
	}
	else
	{
		return code;
	}

	for (sint i = 0; i < iterations; i++)
	{
		uint8 ch;
		is.serial (ch);

		if ((ch & 0xC0) != 0x80)
		{
			nlerror ("CI18N::eatChar(): Invalid UTF-8 character");
		}

		code <<= 6;
		code |= (ucchar)(ch & 0x3F);
	}
	return code;
}

void CI18N::checkASCII7B (ucchar c)
{
	if (c>0x7F)
	{
		nlerror ("CI18N::checkASCII7B: '%c' isn't ASCII 7bits", c);
	}
}


void CI18N::skipComment(IStream &is, int &line)
{
	// the first '/' is already eated
	ucchar c;
	bool longcomment;

	c = eatChar (is);
	if (c == '/') longcomment = false;
	else if (c == '*') longcomment = true;

	do
	{
		c = eatChar (is);
		if (!longcomment && c == '\n')
		{
			line++;
			return;
		}
		if (longcomment && c == '*')
		{
			c = eatChar (is);
			if (c == '/') return;
		}
	}
	while (true);
}

ucchar CI18N::skipWS(IStream &is, int &line)
{
	ucchar c;
	do
	{
		c = eatChar (is);
		if (c == '\n') line++;
		if (c == '/')
		{
			skipComment (is, line);
			c = eatChar (is);
		}
	}
	while (isspace (c));
	return c;
}


void CI18N::createLanguageFile (uint32 lid)
{
	nlassert (lid >= 0 && lid < sizeof (_LanguageFiles)/sizeof(_LanguageFiles[0]));
	
	// write the new string in the file
	COFile cof;
	nlverify (cof.open (_Path + _LanguageFiles[lid] + ".uxt", true, true));

	stringstream ss2;
	ss2 << "\"" << _LanguageFiles[lid] << "\"" << endl;
	cof.serialBuffer((uint8 *)(ss2.str().c_str()), ss2.str().size());
	cof.close ();
}

void CI18N::createLanguageEntry (const string &lval, const string &rval)
{
	sint i;
	for (i = 0; i < (sint)lval.size () ; i++)
	{
		unsigned char c = (unsigned char) lval[i];
		if (c>0x7F)
		{
			nlerror ("CI18N::createLanguageEntry(\"%s\"): your string must be ASCII 7bits ('%c' isn't ASCII 7bits)", lval.c_str(), c);
		}
	}
	for (i = 0; i < (sint)rval.size () ; i++)
	{
		unsigned char c = (unsigned char) rval[i];
		if (c>0x7F)
		{
			nlerror ("CI18N::createLanguageEntry(\"%s\"): your string must be ASCII 7bits ('%c' isn't ASCII 7bits)", rval.c_str(), c);
		}
	}

	for (i = 0; i < sizeof(_LanguageFiles)/sizeof(_LanguageFiles[0]); i++)
	{
		COFile cof;
		nlverify (cof.open (_Path + _LanguageFiles[i] + ".uxt", true, true));

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

void CI18N::setPath (const char* str)
{
	_Path = str;
}

void CI18N::load (uint32 lid)
{
	nlassert (lid >= 0 && lid < sizeof (_LanguageFiles)/sizeof(_LanguageFiles[0]));
	nlassert (_LanguagesNamesLoaded);

	_FileName  = _Path + _LanguageFiles[lid] + ".uxt";

	if (_StrMapLoaded)	_StrMap.clear ();
	else				_StrMapLoaded = true;

	CIFile cf;
	// if the file does not exist, it'll be create automatically
	if (!cf.open (_FileName, true))
	{
		nlwarning ("Could not open file \"%s\" (this file should contain the %s language (lid:%d))", _FileName.c_str (), _LanguageNames[lid].toString().c_str(), lid);
		createLanguageFile (lid);
		return;
	}
	nldebug ("Loading file \"%s\" (this file should contain the %s language (lid:%d))", _FileName.c_str (), _LanguageNames[lid].toString ().c_str(), lid);

	bool startstr = false, equal = false, second = false;
	int line = 1;
	try
	{
		ucchar c;
		// get the language name
		c = skipWS (cf, line);
		if (c != '"')
		{
			nlerror ("open '\"' missing in \"%s\" line %d", _FileName.c_str(), line);
		}
		do
		{
			c = eatChar (cf);
			if (c == '\\')
			{
				c = eatChar (cf);
			}
			else if (c == '"') break;
			else if (c == '\n') line++;
		}
		while (true);

		while (true)
		{
			string codstr;
			ucstring trsstr;
			ucchar c;

			codstr = "";
			trsstr = "";

			// get the coder string
			c = skipWS (cf, line);
			if (c != '"')
			{
				nlerror ("open '\"' missing in \"%s\" line %d", _FileName.c_str(), line);
			}
			startstr = true;
			do
			{
				c = eatChar (cf);
				if (c == '\\')
				{
					c = eatChar (cf);
				}
				else if (c == '"') break;
				else if (c == '\n') line++;
				checkASCII7B (c);
				codstr += (char) c;
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
				c = eatChar (cf);
				if (c == '\\')
				{
					c = eatChar (cf);
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
				nlwarning ("the string '%s' is duplicate in the langage file '%s' line %d, ignored the last one", codstr.c_str(), _FileName.c_str(), line);
			}
		}
	}
	catch (EReadError &)
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

const ucstring &CI18N::get (const char *str)
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

const vector<ucstring> &CI18N::getLanguageNames()
{
	CIFile cf;

	if (!_LanguagesNamesLoaded)
	{
		for (int i = 0; i < sizeof(_LanguageFiles)/sizeof(_LanguageFiles[0]); i++)
		{
			string fn = _Path + _LanguageFiles[i] + ".uxt";

			ucstring lg;

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
					ucchar c;
					// get the language name
					c = skipWS (cf, line);
					if (c != '"')
					{
						nlerror ("open '\"' missing in \"%s\" line %d", fn.c_str(), line);
					}
					do
					{
						c = eatChar (cf);
						if (c == '\\')
						{
							c = eatChar (cf);
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
			nldebug ("add %d '%s' '%s'", i, fn.c_str (), _LanguageFiles[i]);
		}
		_LanguagesNamesLoaded = true;
	}
	return _LanguageNames;
}


} // NLMISC
