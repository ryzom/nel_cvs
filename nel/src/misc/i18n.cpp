/** \file i18n.cpp
 * Internationalisation
 *
 * $Id: i18n.cpp,v 1.42 2003/10/20 16:10:17 lecroart Exp $
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
const ucstring			CI18N::_NotTranslatedValue("<Not Translated>");
bool					CI18N::_LanguagesNamesLoaded = false;
sint32					CI18N::_SelectedLanguage = -1;
CI18N::ILoadProxy		*CI18N::_LoadProxy = 0;


void CI18N::setLoadProxy(ILoadProxy *loadProxy)
{
	_LoadProxy = loadProxy;
}


void CI18N::load (const std::string &languageCode)
{
//	nlassert (lid < _NbLanguages);
//	nlassert (_LanguagesNamesLoaded);

  uint i;
	for (i=0; i<_NbLanguages; ++i)
	{
		if (_LanguageCodes[i] == languageCode)
			break;
	}

	if (i == _NbLanguages)
	{
		nlwarning("I18N: Unknow language code : %s, defaulting to %s", _LanguageCodes[0].c_str());
		i = 0;
	}

	std::string fileName  = _LanguageCodes[i] + ".uxt";

	_SelectedLanguage = i;

	if (_StrMapLoaded)	_StrMap.clear ();
	else				_StrMapLoaded = true;

	ucstring text;
	// read in the text
	if (_LoadProxy)
		_LoadProxy->loadStringFile(fileName, text);
	else
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
			nlwarning("I18N: Error reading label field in %s. Stop reading after %s.", fileName.c_str(), lastReadLabel.c_str());
			return;
		}
		lastReadLabel = label;
		skipWhiteSpace(first, last);
		if (!parseMarkedString('[', ']', first, last, ucs))
		{
			nlwarning("I18N: Error reading text for label %s in %s. Stop reading.", label.c_str(), fileName.c_str());
			return;
		}

		// ok, a line read.
		std::pair<std::map<std::string, ucstring>::iterator, bool> ret;
		ret = _StrMap.insert(std::make_pair(label, ucs));
		if (!ret.second)
		{
			nlwarning("I18N: Error in %s, the label %s exist twice !", fileName.c_str(), label.c_str());
		}
		skipWhiteSpace(first, last);
	}

	// a little check to ensure that the lang name has been set.
	StrMapContainer::iterator it(_StrMap.find("LanguageName"));
	if (it == _StrMap.end())
	{
		nlwarning("I18N: In file %s, missing LanguageName translation (should be first in file)", fileName.c_str());
	}
}

const ucstring &CI18N::get (const std::string &label)
{
	if (label.empty())
	{
		static ucstring	emptyString;
		return emptyString;
	}

	StrMapContainer::iterator it(_StrMap.find(label));

	if (it != _StrMap.end())
		return it->second;

	nlwarning("I18N: The string %s did not exist in language %s", label.c_str(), _LanguageCodes[_SelectedLanguage].c_str());

	static ucstring	badString;

	badString = ucstring(std::string("<NotExist:")+label+">");

	return badString;
}

ucstring CI18N::getCurrentLanguageName ()
{
	return get("LanguageName");
}


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


void	CI18N::skipWhiteSpace(ucstring::const_iterator &it, ucstring::const_iterator &last, ucstring *storeComments)
{
	while (it != last &&
			(
					*it == 0xa
				||	*it == 0xd
				||	*it == ' '
				||	*it == '\t'
				||	(storeComments && *it == '/' && it+1 != last && *(it+1) == '/')
				||	(storeComments && *it == '/' && it+1 != last && *(it+1) == '*')
			))
	{
		if (storeComments && *it == '/' && it+1 != last && *(it+1) == '/')
		{
			// found a one line C comment. Store it until end of line.
			while (it != last && *it != '\n')
				storeComments->push_back(*it++);
			// store the final '\n'
			if (it != last)
				storeComments->push_back(*it++);
		}
		else if (storeComments && *it == '/' && it+1 != last && *(it+1) == '*')
		{
			// found a multiline C++ comment. store until we found the closing '*/'
			while (it != last && !(*it == '*' && it+1 != last && *(it+1) == '/'))
				storeComments->push_back(*it++);
			// store the final '*'
			if (it != last)
				storeComments->push_back(*it++);
			// store the final '/'
			if (it != last)
				storeComments->push_back(*it++);
			// and a new line.
			storeComments->push_back('\r');
			storeComments->push_back('\n');
		}
		else
		{
			// just skip white space or don't store comments
			++it;
		}
	}
}

bool CI18N::parseLabel(ucstring::const_iterator &it, ucstring::const_iterator &last, std::string &label)
{
	label.erase();

	// first char must be A-Za-z@_
	if (it != last && 
			(
				(*it >= '0' && *it <= '9')
			||	(*it >= 'A' && *it <= 'Z')
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
			if (*it == openMark)
			{
				nlwarning("I18N: Found a non escaped openmark %c in a delimited string (Delimiters : '%c' - '%c')", char(openMark), char(openMark), char(closeMark));
				return false;
			}
			if (*it == '\t' || *it == '\n' || *it == '\r')
				++it;
			else if (*it == '\\' && it+1 != last && *(it+1) != '\\')
			{
				++it;
				// this is an escape sequence !
				switch(*it)
				{
				case 't':
					result.push_back('\t');
					break;
				case 'n':
					result.push_back('\n');
					break;
				case 'd':
					// insert a delete
					result.push_back(8);
					break;
				default:
					// escape the close mark ?
					if(*it == closeMark)
						result.push_back(closeMark);
					// escape the open mark ?
					else if(*it == openMark)
						result.push_back(openMark);
					else
						nlwarning("I18N: Ignoring unknown escape code \\%c (char value : %u)", char(*it), *it);
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
			nlwarning("I18N: Missing end of delimited string (Delimiters : '%c' - '%c')", char(openMark), char(closeMark));
			return false;
		}
		else
			++it;
	}
	else
	{
		nlwarning("I18N: Malformed or non existent delimited string (Delimiters : '%c' - '%c')", char(openMark), char(closeMark));
		return false;
	}

	return true;
}


void CI18N::readTextFile(const std::string &filename, ucstring &result, bool forceUtf8, bool fileLookup, bool preprocess)
{
	std::string fullName;
	if (fileLookup)
		fullName = CPath::lookup(filename, false);
	else
		fullName = filename;

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

	if (!text.empty())
		readTextBuffer((uint8*)&text[0], text.size(), result, forceUtf8);

	if (preprocess)
	{
		ucstring final;
		// parse the file, looking for preprocessor command.
		ucstring::size_type pos = 0;
		ucstring::size_type lastPos = 0;
		ucstring	includeCmd("#include");

		while ((pos = result.find(includeCmd, pos)) != ucstring::npos)
		{
			// copy the previous text
			final += result.substr(lastPos, pos - lastPos);

			// extract the inserted file name.
			ucstring::const_iterator first, last;
			first = result.begin()+pos+includeCmd.size();
			last = result.end();

			ucstring name;
			skipWhiteSpace(first, last);
			if (parseMarkedString('\"', '\"', first, last, name))
			{
				string subFilename = name.toString();
				nldebug("I18N: Including '%s' into '%s'",
					subFilename.c_str(),
					filename.c_str());
				ucstring inserted;

				{
					CIFile testFile;
					if (!testFile.open(subFilename))
					{
						// try to open the include file relative to current file
						subFilename = CFile::getPath(filename)+subFilename;
					}
				}
				readTextFile(subFilename, inserted, forceUtf8, fileLookup, preprocess);

				final += inserted;
			}
			else
			{
				nlwarning("I18N: Error parsing include file in line '%s' from file '%s'",
					result.substr(pos, result.find(ucstring("\n"), pos) - pos).c_str(),
					filename.c_str());
			}

			pos = lastPos = first - result.begin();
		}

		// copy the remaining chars
		ucstring temp = final;
		
		for (uint i=lastPos; i<result.size(); ++i)
		{
			temp += result[i];
		}

//		final = final + temp;

		result.swap(temp);
	}


}

void CI18N::readTextBuffer(uint8 *buffer, uint size, ucstring &result, bool forceUtf8)
{
	std::string text;

	text.append((char*)buffer, size);

	static char utf16Header[] = {char(0xff), char(0xfe), 0};
	static char utf16RevHeader[] = {char(0xfe), char(0xff), 0};
	static char utf8Header[] = {char(0xef), char(0xbb), char(0xbf), 0};

	if (forceUtf8)
	{
		if (text.find(utf8Header) == 0)
			// remove utf8 header
			text = std::string(text, 3);
		result.fromUtf8(text);
	}
	else if (text.find(utf8Header) == 0)
	{
		// remove utf8 header
//		text = std::string(&(*(text.begin()+3)), text.size()-3);
		text = text.substr(3);
		result.fromUtf8(text);
	}
	else if (text.find(utf16Header) == 0)
	{
		// remove utf16 header
		text = std::string(text, 2);
//		uint32 size = text.size();
		// check pair number of bytes
		nlassert((text.size() & 1) == 0);
		// and do manual conversion
		uint16 *src = (uint16*)(text.c_str());

		for (uint j=0; j<text.size()/2; j++)
			result.push_back(*src++);
	}
	else if (text.find(utf16RevHeader) == 0)
	{
		// remove utf16 header
		text = std::string(text, 2);
//		uint32 size = text.size();
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


void CI18N::writeTextFile(const std::string filename, const ucstring &content, bool utf8)
{
	COFile file(filename);

	if (!utf8)
	{
		// write the unicode 16 bits tag
		uint16 unicodeTag = 0xfeff;
		file.serial(unicodeTag);

		uint i;
		for (i=0; i<content.size(); ++i)
		{
			uint16 c = content[i];
			file.serial(c);
		}
	}
	else
	{
		static char utf8Header[] = {char(0xef), char(0xbb), char(0xbf), 0};

		std::string str = encodeUTF8(content);
		// add the UTF-8 'not official' header
		str = utf8Header + str;

		uint i;
		for (i=0; i<str.size(); ++i)
		{
			file.serial(str[i]);
		}
	}
}

ucstring CI18N::makeMarkedString(ucchar openMark, ucchar closeMark, const ucstring &text)
{
	ucstring ret;

	ret.push_back(openMark);

	ucstring::const_iterator first(text.begin()), last(text.end());
	for (; first != last; ++first)
	{
		if (*first == '\n')
		{
			ret += '\\';
			ret += 'n';
		}
		else if (*first == '\t')
		{
			ret += '\\';
			ret += 't';
		}
		else if (*first == closeMark)
		{
			// excape the embeded closing mark
			ret += '\\';
			ret += closeMark;
		}
		else
		{
			ret += *first;
		}
	}

	ret += closeMark;

	return ret;
}


string CI18N::encodeUTF8(const ucstring &str)
{
	return str.toUtf8();
	/*	
	string	res;
	ucstring::const_iterator first(str.begin()), last(str.end());
	for (; first != last; ++first)
	{
	  //ucchar	c = *first;
		uint nbLoop = 0;
		if (*first < 0x80)
			res += char(*first);
		else if (*first < 0x800)
		{
			ucchar c = *first;
			c = c >> 6;
			c = c & 0x1F;
			res += c | 0xC0;
			nbLoop = 1;
		}
		else if (*first < 0x10000)
		{
			ucchar c = *first;
			c = c >> 12;
			c = c & 0x0F;
			res += c | 0xE0;
			nbLoop = 2;
		}

		for (uint i=0; i<nbLoop; ++i)
		{
			ucchar	c = *first;
			c = c >> ((nbLoop - i - 1) * 6);
			c = c & 0x3F;
			res += char(c) | 0x80; 
		}
	}
	return res;
	*/
}

/* UTF-8 conversion table
U-00000000 - U-0000007F:  0xxxxxxx  
U-00000080 - U-000007FF:  110xxxxx 10xxxxxx  
U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx  
// not used as we convert from 16 bits unicode
U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx  
U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx  
U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx  
*/


uint64	CI18N::makeHash(const ucstring &str)
{
	// on passe au moins 8 fois sur chaque octet de resultat
	if (str.empty())
		return 0;
	const	uint32	MIN_TURN = 8*8;
	uint64	hash = 0;
	uint8	*ph = (uint8*)&hash;
	uint8	*pc = (uint8*)str.data();

	uint nbLoop = max(uint32(str.size()*2), MIN_TURN);
	uint roll = 0;

	for (uint i=0; i<nbLoop; ++i)
	{
		ph[(i/2) & 0x7] += pc[i%(str.size()*2)] << roll;
		ph[(i/2) & 0x7] += pc[i%(str.size()*2)] >> (8-roll);

		roll++;
		roll &= 0x7;
	}

	return hash;
}

// convert a hash value to a readable string 
string CI18N::hashToString(uint64 hash)
{
	uint32 *ph = (uint32*)&hash;

	char temp[] = "0011223344556677";
	sprintf(temp, "%08X%08X", ph[0], ph[1]);

	return string(temp);
}

// convert a readable string into a hash value.
uint64 CI18N::stringToHash(const std::string &str)
{
	nlassert(str.size() == 16);
	uint32	low, hight;

	string sl, sh;
	sh = str.substr(0, 8);
	sl = str.substr(8, 8);

	sscanf(sh.c_str(), "%08X", &hight);
	sscanf(sl.c_str(), "%08X", &low);

	uint64 hash;
	uint32 *ph = (uint32*)&hash;

	ph[0] = hight;
	ph[1] = low;

	return hash;
}



} // NLMISC





















