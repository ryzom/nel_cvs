/** \file ucstring.h
 * Unicode stringclass using 16bits per character
 *
 * $Id: ucstring.h,v 1.9 2003/10/08 08:20:58 boucher Exp $
 *
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

#ifndef NL_UCSTRING_H
#define NL_UCSTRING_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"

#include <string>

/**
 * \typedef ucstring
 * An unicode string class (16 bits per character).
 * Add features to convert and assign \c ucstring to \c string and \c string to \c ucstring.
 */
typedef std::basic_string<ucchar> ucstringbase;

class ucstring : public ucstringbase
{
public:

	ucstring () {}
	
	ucstring (const ucstringbase &str) : ucstringbase (str) {}

	ucstring (const std::string &str) : ucstringbase ()
	{
		*this=str;
	}

	virtual ~ucstring () {}
	
	ucstring &operator= (ucchar c)
	{
		resize (1);
		operator[](0) = c;
		return *this;
	}

	ucstring &operator= (const char *str)
	{
		resize (strlen (str));
		for (sint i = 0; i < (sint) strlen (str); i++)
		{
			operator[](i) = str[i];
		}
		return *this;
	}

	ucstring &operator= (const std::string &str)
	{
		resize (str.size ());
		for (sint i = 0; i < (sint) str.size (); i++)
		{
			operator[](i) = str[i];
		}
		return *this;
	}

	ucstring &operator= (const ucstringbase &str)
	{
		ucstringbase::operator =(str);
		return *this;
	}

	ucstring &operator+= (ucchar c)
	{
		resize (size() + 1);
		operator[](size()-1) = c;
		return *this;
	}

	ucstring &operator+= (const char *str)
	{
		sint s = size();
		resize (s + strlen(str));
		for (sint i = 0; i < (sint) strlen(str); i++)
		{
			operator[](s+i) = str[i];
		}
		return *this;
	}

	ucstring &operator+= (const std::string &str)
	{
		sint s = size();
		resize (s + str.size());
		for (sint i = 0; i < (sint) str.size(); i++)
		{
			operator[](s+i) = str[i];
		}
		return *this;
	}

	ucstring &operator+= (const ucstringbase &str)
	{
		ucstringbase::operator +=(str);
		return *this;
	}


	/// Converts the controlled ucstring to a string str
	void toString (std::string &str) const
	{
		str.resize (size ());
		for (sint i = 0; i < (sint) str.size (); i++)
		{
			str[i] = (char) operator[](i);
		}
	}

	/// Converts the controlled ucstring and returns the resulting string
	std::string toString () const
	{
		std::string str;
		toString(str);
		return str;
	}

	/// Convert this ucstring (16bits char) into a utf8 string
	std::string toUtf8() const
	{
		std::string	res;
		ucstring::const_iterator first(begin()), last(end());
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
			else /*if (*first < 0x10000)*/
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
	}


	/// Convert the utf8 string into this ucstring (16 bits char)
	void fromUtf8(const std::string &stringUtf8)
	{
		// clear the string
		erase();

		uint8 c;
		ucchar code;
		sint iterations = 0;

		std::string::const_iterator first(stringUtf8.begin()), last(stringUtf8.end());
		for (; first != last; )
		{
			c = *first++;
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
				nlwarning ("ucstring::fromUtf8(): Invalid UTF-8 character");
			}
			else
			{
				push_back(code);
				iterations = 0;
			}

			if (iterations)
			{
				for (sint i = 0; i < iterations; i++)
				{
					if (first == last)
					{
						nlwarning ("ucstring::fromUtf8(): Invalid UTF-8 character");
						return;
					}

					uint8 ch;
					ch = *first ++;

					if ((ch & 0xC0) != 0x80)
					{
						nlwarning ("ucstring::fromUtf8(): Invalid UTF-8 character");
						code = '?';
						break;
					}

					code <<= 6;
					code |= (ucchar)(ch & 0x3F);
				}
				push_back(code);
			}
		}
	}
};

inline ucstring operator+(const ucstringbase &ucstr, ucchar c)
{
	ucstring	ret;
	ret= ucstr;
	ret+= c;
	return ret;
}

inline ucstring operator+(const ucstringbase &ucstr, const char *c)
{
	ucstring	ret;
	ret= ucstr;
	ret+= c;
	return ret;
}

inline ucstring operator+(const ucstringbase &ucstr, const std::string &c)
{
	ucstring	ret;
	ret= ucstr;
	ret+= c;
	return ret;
}

inline ucstring operator+(ucchar c, const ucstringbase &ucstr)
{
	ucstring	ret;
	ret= c;
	ret += ucstr;
	return ret;
}

inline ucstring operator+(const char *c, const ucstringbase &ucstr)
{
	ucstring	ret;
	ret= c;
	ret += ucstr;
	return ret;
}

inline ucstring operator+(const std::string &c, const ucstringbase &ucstr)
{
	ucstring	ret;
	ret= c;
	ret += ucstr;
	return ret;
}

#endif // NL_UCSTRING_H
