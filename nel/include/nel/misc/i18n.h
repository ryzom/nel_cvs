/** \file i18n.h
 * Internationalisation class for localisation of the system
 *
 * $Id: i18n.h,v 1.11.2.1 2003/04/24 13:55:57 boucher Exp $
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
#include <algorithm>


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
 *	Update 26-02-2002 Boris Boucher
 *
 *	Language are now preferably handled via official language code.
 *	We use the ISO 639-2 code for language.
 *	Optionnaly, we can append a country code (ISO 3066) to differentiate
 *	between language flavor (eg chinese is ISO 639-2 zh, but come in
 *	traditionnal or simplified form. So we append the country code :
 *	zh-CN (china) for simplified, zh for traditionnal).
 *	
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CI18N
{
public:

	/** Proxy interface for loading string file.
	 *	Implemente this interface in client code and set it inside I18N
	 *	in order to be able to do any work on string file before they
	 *	are read by CI18N.
	 *	This is used by Ryzom to merge the working string file with
	 *	the exploitation one when they are more recente.
	 */
	struct ILoadProxy
	{
		virtual void loadStringFile(const std::string &filename, ucstring &text) =0;
	};

	/// Set the load proxy class. Proxy can be NULL to unregister.
	static void setLoadProxy(ILoadProxy *loadProxy);

	/// Return a vector with all language available. The vector contains the name of the language.
	/// The index in the vector is used in \c load() function
	static const std::vector<ucstring> &getLanguageNames();
	/** Return a vector with all language code available.
	 *	Code are ISO 639-2 compliant.
	 *	As in getLanguageNames(), the index in the vector can be used to call load()
	 */
	static const std::vector<std::string> &getLanguageCodes();
	/// Load a language file depending of the language
//	static void load (uint32 lid);
	static void load (const std::string &languageCode);

	/// Returns the name of the language in english (french, english...)
	static ucstring getCurrentLanguageName ();

	/// Find a string in the selected language and return his association.
	static const ucstring &get (const std::string &label);

	/// Temporary, we don't have file system for now, so we do a tricky cheat. there s not check so be careful!
//	static void setPath (const char* str);

	/** Read the content of a file as a unicode text.
	 *	The method support 16 bits or 8bits utf-8 tagged files.
	 *	8 bits UTF-8 encofing can be reconized by a non official header :
	 *	EF,BB, BF.
	 *	16 bits encoding can be reconized by the official header :
	 *	FF, FE, witch can be reversed if the data are MSB first.
	 *	
	 *	Optionnaly, you can force the reader to consider the file as
	 *	UTF-8 encoded.
	 */
	static void readTextFile(const std::string &filename, ucstring &result, bool forceUtf8 = false, bool fileLookup = true);

	/** Read the content of a buffer as a unicode text.
	 *	This is to read preloaded unicode files.
	 *	The method support 16 bits or 8bits utf-8 tagged buffer.
	 *	8 bits UTF-8 encofing can be reconized by a non official header :
	 *	EF,BB, BF.
	 *	16 bits encoding can be reconized by the official header :
	 *	FF, FE, witch can be reversed if the data are MSB first.
	 *	
	 *	Optionnaly, you can force the reader to consider the file as
	 *	UTF-8 encoded.
	 */
	static void readTextBuffer(uint8 *buffer, uint size, ucstring &result, bool forceUtf8 = false);

	/** Remove any C style comment from the passed string.
	 */
	static void remove_C_Comment(ucstring &commentedString);

	/** Encode a unicode string into a string using UTF-8 encoding.
	*/
	static std::string CI18N::encodeUTF8(const ucstring &str);

	/** Write a unicode text file using unicode 16 or UTF-8 encoding.
	 */
	static void writeTextFile(const std::string filename, const ucstring &content, bool utf8 = true);

	static ucstring makeMarkedString(ucchar openMark, ucchar closeMark, const ucstring &text);

	//@{
	//\name Parsing utility
	/** Skip the white space.
	 *	You can optionnaly pass a ucstring pointer to receive any comments string that build the
	 *	white space.
	 *	This is usefull if you whant to keep the comments.
	 *	NB : comments are appended to the comments string.
	 */
	static void		skipWhiteSpace		(ucstring::const_iterator &it, ucstring::const_iterator &last, ucstring *storeComments = NULL);
	/// Parse a label
	static bool		parseLabel			(ucstring::const_iterator &it, ucstring::const_iterator &last, std::string &label);
	/// Parse a marked string. NB : usualy, we use [ and ] as string delimiters in translation files.
	static bool		parseMarkedString	(ucchar openMark, ucchar closeMark, ucstring::const_iterator &it, ucstring::const_iterator &last, ucstring &result);
	//@}

	//@{
	//\name Hash code tools. 
	// Generate a hash value for a given string
	static uint64	makeHash(const ucstring &str);
	// convert a hash value to a readable string 
	static std::string hashToString(uint64 hash);
	// convert a readable string into a hash value.
	static uint64 stringToHash(const std::string &str);
	//@}

private:

	typedef std::map<std::string, ucstring>						StrMapContainer;

	static ILoadProxy							*_LoadProxy;

	static StrMapContainer										 _StrMap;
	static bool													 _StrMapLoaded;

	static const std::string									 _LanguageCodes[];
	static const uint											_NbLanguages;

	static bool													 _LanguagesNamesLoaded;

	static sint32												 _SelectedLanguage;
	static const ucstring										_NotTranslatedValue;
};




} // NLMISC


#endif // NL_I18N_H

/* End of i18n.h */





















