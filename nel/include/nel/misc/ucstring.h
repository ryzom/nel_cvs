/** \file ucstring.h
 * Unicode stringclass using 16bits per character
 *
 * $Id: ucstring.h,v 1.4 2003/02/14 14:15:11 lecroart Exp $
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
