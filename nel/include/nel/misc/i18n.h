/** \file i18n.h
 * Internationalisation class for localisation of the system
 *
 * $Id: i18n.h,v 1.7 2002/06/21 13:02:15 miller Exp $
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

#ifndef NL_I18N_H
#define NL_I18N_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/file.h"

#include <string>
#include <map>

namespace NLMISC {


/**
 * Class for the internationalisation. It's a singleton pattern.
 * 
 * This class provide an easy way to localise all string. First you have to get all available language with \c getLanguageNames().
 * If you already know the number of the language (that is the index in the vector returns by \c getLanguagesNames()), you can
 * load the language file with \c load(). Now, you can get a localised string with his association with \c get().
 *
 *\code
	// get all language names (you must call this before calling load())
	CI18N::getLanguageNames ();
	// load the language 1 (french)
	CI18N::load (1);
	// display "Salut" that is the "hi" string in the selected language (french).
	nlinfo (CI18N::get("hi").c_str ());
	// display "rms est un master", the french version of the string
	nlinfo (CI18N::get("%s is a master").c_str (), "mrs");
 *\endcode
 *
 * If the string doesn't exist, it will be automatically added in all language files with a <Not Translated> mention.
 * If the language file doesn't exist, it'll be automatically create.
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CI18N
{
public:

	/// Return a vector with all language available. The vector contains the name of the language.
	/// The index in the vector is used in \c load() function
	/// \warning you *must* call this function before calling load()
	static const std::vector<ucstring> &getLanguageNames();

	/// Load a language file depending of the language
	static void load (uint32 lid);

	/// Find a string in the selected language and return his association.
	static const ucstring &get (const char *str);

	/// Temporary, we don't have file system for now, so we do a tricky cheat. there s not check so be careful!
	static void setPath (const char* str);

private:

	typedef std::map<std::string, ucstring>::iterator			 ItStrMap;
	typedef std::map<std::string, ucstring>::value_type			 ValueStrMap;

	static std::map<std::string, ucstring>						 _StrMap;
	static bool													 _StrMapLoaded;

	static std::string											 _Path;
	static std::string											 _FileName;
	static const char											*_LanguageFiles[];

	static std::vector<ucstring>								 _LanguageNames;
	static bool													 _LanguagesNamesLoaded;

	static ucchar	eatChar				(IStream &is);
	static void		checkASCII7B		(ucchar c);

	static void		createLanguageFile	(uint32 lid);
	static void		createLanguageEntry (const std::string &lval, const std::string &rval);

	static void		skipComment			(IStream &is, int &line);
	static ucchar	skipWS				(IStream &is, int &line);
};


} // NLMISC


#endif // NL_I18N_H

/* End of i18n.h */
