/** \file i18n.cpp
 * Internationalisation
 *
 * $Id: i18n.cpp,v 1.26 2003/03/03 16:14:56 boucher Exp $
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

#include "nel/misc/path.h"
#include "nel/misc/i18n.h"

using namespace std;

namespace NLMISC {

/*const std::string		CI18N::_LanguageFiles[] = 
{
	std::string("english"),
	std::string("french")
};
*/
const std::string		CI18N::_LanguageCodes[] =
{
	std::string("en"),		// english
	std::string("fr"),		// french
	std::string("zh-TW"),	// traditionnal chinese
	std::string("zh-CN")	// simplified chinese
};

const uint				CI18N::_NbLanguages = sizeof(CI18N::_LanguageCodes) / sizeof(std::string);

CI18N::StrMapContainer	CI18N::_StrMap;
bool					CI18N::_StrMapLoaded = false;
//string					CI18N::_Path = "";
//string					CI18N::_FileName = "";

const ucstring			CI18N::_NotTranslatedValue("<Not Translated>");


bool					CI18N::_LanguagesNamesLoaded = false;
sint32					CI18N::_SelectedLanguage = -1;

/*ucchar CI18N::eatChar (IStream &is)
{
	uint8 c;
	ucchar code;
	sint iterations = 0;

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
*/
/*
void CI18N::checkASCII7B (ucchar c)
{
	if (c>0x7F)
	{
		nlerror ("CI18N::checkASCII7B: '%c' isn't ASCII 7bits", c);
	}
}
*/
/*
void CI18N::skipComment(IStream &is, int &line)
{
	// the first '/' is already eated
	ucchar c;
	bool longcomment = false;

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
*/
/*
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
*/
/*
void CI18N::createLanguageFile (uint32 lid)
{
	nlassert (lid < sizeof (_LanguageFiles)/sizeof(_LanguageFiles[0]));
	
	// write the new string in the file
	COFile cof;
	nlverify (cof.open (_Path + _LanguageFiles[lid] + ".uxt", true, true));

	stringstream ss2;
	ss2 << "\"" << _LanguageFiles[lid] << "\"" << endl;
	cof.serialBuffer((uint8 *)(ss2.str().c_str()), ss2.str().size());
	cof.close ();
}
*/
/*
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

	for (i = 0; i < (sint)(sizeof(_LanguageFiles)/sizeof(_LanguageFiles[0])); i++)
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
*/
/*
void CI18N::setPath (const char* str)
{
	_Path = str;
}
*/

void CI18N::load (uint32 lid)
{
	nlassert (lid < _NbLanguages);
//	nlassert (_LanguagesNamesLoaded);

	std::string fileName  = _LanguageCodes[lid] + ".uxt";

	_SelectedLanguage = lid;

	if (_StrMapLoaded)	_StrMap.clear ();
	else				_StrMapLoaded = true;

	ucstring text;
	// read in the text
	readTextFile(fileName, text);
	// remove any comment
	remove_C_Comment(text);

	ucstring::const_iterator first(text.begin()), last(text.end());
	std::string lastReadLabel("nothing");
	
	while (first != last)
	{
		skipWhiteSpace(first, last);
		std::string label;
		ucstring ucs;
		if (!parseLabel(first, last, label))
		{
			nlwarning("Error reading label field in %s. Stop reading after %s.", fileName.c_str(), lastReadLabel.c_str());
			return;
		}
		lastReadLabel = label;
		skipWhiteSpace(first, last);
		if (!parseMarkedString('[', ']', first, last, ucs))
		{
			nlwarning("Error reading text for label %s in %s. Stop reading.", label.c_str(), fileName.c_str());
			return;
		}

		// ok, a line read.
		std::pair<std::map<std::string, ucstring>::iterator, bool> ret;
		ret = _StrMap.insert(std::make_pair(label, text));
		if (!ret.second)
		{
			nlwarning("Error in %s, the label %s exist twice !", fileName.c_str(), label.c_str());
		}
	}

	// a little check to ensure that the lang name has been set.
	StrMapContainer::iterator it(_StrMap.find("LanguageName"));
	if (it == _StrMap.end())
	{
		nlwarning("In file %s, missing LanguageName translation (should be first in file)", fileName.c_str());
	}
}
/*
void CI18N::load (uint32 lid)
{
	nlassert (lid < _NbLanguages);
	nlassert (_LanguagesNamesLoaded);

	_FileName  = _Path + _LanguageFiles[lid] + ".uxt";

	_SelectedLanguage = lid;

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
*/
const ucstring &CI18N::get (const std::string &label)
{
	StrMapContainer::iterator it(_StrMap.find(label));

	if (it != _StrMap.end())
		return it->second;

	nlwarning("The string %s did not exist in language %s", label.c_str(), _LanguageCodes[_SelectedLanguage].c_str());

	return _NotTranslatedValue;
}
/*
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
*/

ucstring CI18N::getCurrentLanguageName ()
{
	return get("LanguageName");
}

/*string CI18N::getCurrentLanguage ()
{
	if (_SelectedLanguage == -1)
		return "<NoLanguage>";
	else
		return _LanguageFiles[_SelectedLanguage];
}
*/

void CI18N::remove_C_Comment(ucstring &commentedString)
{
	{
		ucstring temp;
		temp.reserve(commentedString.size());
		ucstring::const_iterator first(commentedString.begin()), last(commentedString.end());
		for (;first != last; ++first)
		{
			temp.push_back(*first);
			if (*first == '[')
			{
				// no comment inside string literal
				while (++first != last)
				{
					temp.push_back(*first);
					if (*first == ']')
						break;
				}
			}
			else if (*first == '/')
			{
				// start of comment ?
				++first;
				if (first != last && *first == '/')
				{
					temp.pop_back();
					// one line comment, skip until end of line
					while (first != last && *first != '\n')
						++first;
				}
				else if (first != last && *first == '*')
				{
					temp.pop_back();
					// start of multiline comment, skip until we found '*/'
					while (first != last && !(*first == '*' && (first+1) != last && *(first+1) == '/'))
						++first;
					// skip the closing '/'
					if (first != last)
						++first;
				}
				else
				{
					temp.push_back(*first);
				}
			}
		}

		commentedString.swap(temp);
	}
}


void	CI18N::skipWhiteSpace(ucstring::const_iterator &it, ucstring::const_iterator &last)
{
	while (it != last &&
			(
					*it == 0xa
				||	*it == 0xd
				||	*it == ' '
				||	*it == '\t'
			))
	{
		++it;
	}
}

bool CI18N::parseLabel			(ucstring::const_iterator &it, ucstring::const_iterator &last, std::string &label)
{
	label.erase();

	// first char must be A-Za-z@_
	if (it != last && 
			(
				(*it >= 'A' && *it <= 'Z')
			||	(*it >= 'a' && *it <= 'z')
			||	(*it == '_')
			||	(*it == '@')
			)
		)
		label.push_back(char(*it++));
	else
		return false;

	// other char must be [0-9A-Za-z@_]*
	while (it != last && 
			(
				(*it >= '0' && *it <= '9')
			||	(*it >= 'A' && *it <= 'Z')
			||	(*it >= 'a' && *it <= 'z')
			||	(*it == '_')
			||	(*it == '@')
			)
		)
		label.push_back(char(*it++));

	return true;
}

bool CI18N::parseMarkedString(ucchar openMark, ucchar closeMark, ucstring::const_iterator &it, ucstring::const_iterator &last, ucstring &result)
{
//		ucstring ret;
	result.erase();
	// parse a string delimited by the specified opening and closing mark

	if (it != last && *it == openMark)
	{
		++it;

		while (it != last && *it != closeMark)
		{
			// ignore tab, new lines and line feed
			if (*it == '\t' || *it == '\n' || *it == '\r')
				++it;
			else if (*it == '\\' && it+1 != last && *(it+1) != '\\')
			{
				++it;
				// this is an esace sequence !
				switch(*it)
				{
				case 't':
					result.push_back('\t');
					break;
				case 'n':
					result.push_back('\n');
					break;
				default:
					nlwarning("Ignoring unknown escape code \\%c", *it);
				}
				++it;
			}
			else if (*it == '\\' && it+1 != last && *(it+1) == '\\')
			{
				// escape the \ char
				++it;
				result.push_back(*it);
				++it;
			}
			else
				result.push_back(*it++);
		}

		if (it == last || *it != closeMark)
		{
			nlwarning("Missing end of delimited string (Delimiters : '%c' - '%c')", char(openMark), char(closeMark));
			return false;
		}
		else
			++it;
	}
	else
	{
		nlwarning("Malformed or non existent delimited string (Delimiters : '%c' - '%c')", char(openMark), char(closeMark));
		return false;
	}

	return true;
}
/*
const vector<ucstring> &CI18N::getLanguageNames()
{
	enumFiles();
	return _LanguageNames;
}
*/
/*
void CI18N::enumFiles()
{
	static bool done = false;

	if (done)
		return;

	done = true;

	CIFile cf;

	if (!_LanguagesNamesLoaded)
	{
		for (int i = 0; i < (int)(sizeof(_LanguageFiles)/sizeof(_LanguageFiles[0])); i++)
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
}
*/

void CI18N::readTextFile(const std::string &filename, ucstring &result, bool forceUtf8)
{
	std::string fullName = CPath::lookup(filename, false);

	if (fullName.empty())
		return;

	NLMISC::CIFile	file(fullName);

	std::string text;

	text.reserve(file.getFileSize());

	//while (!file.eof())
	for (uint i=0; i<file.getFileSize(); ++i)
	{
		uint8 c;
		file.serial(c);
		text.push_back(c);
	}

	static char utf16RevHeader[] = {char(0xff), char(0xfe), 0};
	static char utf16Header[] = {char(0xfe), char(0xff), 0};
	static char utf8Header[] = {char(0xef), char(0xbb), char(0xbf), 0};

	if (forceUtf8)
	{
		if (text.find(utf8Header) == 0)
			// remove utf8 header
			text = std::string(&(*(text.begin()+3)), text.size()-3);
		result.fromUtf8(text);
	}
	else if (text.find(utf8Header) == 0)
	{
		// remove utf8 header
		text = std::string(&(*(text.begin()+3)), text.size()-3);
		result.fromUtf8(text);
	}
	else if (text.find(utf16Header))
	{
		// remove utf16 header
		text = std::string(&(*(text.begin()+2)), text.size()-2);
		uint32 size = text.size();
		// check pair number of bytes
		nlassert((text.size() & 1) == 0);
		// and do manual conversion
		uint16 *src = (uint16*)(text.c_str());

		for (uint j=0; j<text.size()/2; j++)
			result.push_back(*src++);
	}
	else if (text.find(utf16RevHeader))
	{
		// remove utf16 header
		text = std::string(&(*(text.begin()+2)), text.size()-2);
		uint32 size = text.size();
		// check pair number of bytes
		nlassert((text.size() & 1) == 0);
		// and do manual conversion
		uint16 *src = (uint16*)(text.c_str());
		
		uint j;
		for (j=0; j<text.size()/2; j++)
			result.push_back(*src++);
		//  Reverse byte order
		for (j=0; j<text.size()/2; j++)
		{
			uint8 *pc = (uint8*) &result[j];
			std::swap(pc[0], pc[1]);
		}
	}
	else
	{
		// hum.. ascii read ?
		// so, just to a direct conversion
		result = text;
	}
}


} // NLMISC





















