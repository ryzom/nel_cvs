/** \file words_dictionary.cpp
 * Words dictionary
 *
 * $Id: words_dictionary.cpp,v 1.1 2003/11/12 17:52:58 cado Exp $
 */

/* Copyright, 2000-2003 Nevrax Ltd.
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

#include <stdmisc.h>

#include "nel/misc/words_dictionary.h"
#include "nel/misc/config_file.h"
#include "nel/misc/path.h"
#include "nel/misc/diff_tool.h"

using namespace std;


namespace NLMISC {

/*
 * Constructor
 */
CWordsDictionary::CWordsDictionary()
{
}


/*
 * Load the config file and the related words files. Return false in case of failure.
 */
bool CWordsDictionary::init( const string& configFileName )
{
	// Read config file
	bool cfFound = false;
	CConfigFile cf;
	try
	{
		cf.load( configFileName );
		cfFound = true;
	}
	catch ( EConfigFile& e )
	{
		nlwarning( "WD: %s", e.what() );
	}
	string wordsPath, languageCode;
	if ( cfFound )
	{
		CConfigFile::CVar *v = cf.getVarPtr( "WordsPath" );
		if ( v )
		{
			wordsPath = v->asString();
			/*if ( (!wordsPath.empty()) && (wordsPath[wordsPath.size()-1]!='/') )
				wordsPath += '/';*/
		}
		v = cf.getVarPtr( "LanguageCode" );
		if ( v )
			languageCode = v->asString();
	}
	if ( languageCode.empty() )
		languageCode = "en";

	// Load all found words files
	const string ext = ".txt";
	vector<string> fileList;
	CPath::getPathContent( wordsPath, false, false, true, fileList );
	for ( vector<string>::const_iterator ifl=fileList.begin(); ifl!=fileList.end(); ++ifl )
	{
		const string& filename = (*ifl);
		string::size_type p;
		if ( (p = filename.find( string("_words_") + languageCode + ext )) != string::npos )
		{
			nldebug( "WD: Loading %s", filename.c_str() );
			string::size_type origSize = filename.size() - ext.size();
			const string truncFilename = CFile::getFilenameWithoutExtension( filename );
			const string wordType = truncFilename.substr( 0, p - (origSize - truncFilename.size()) );

			// Load Unicode Excel words file
			STRING_MANAGER::TWorksheet worksheet;
			STRING_MANAGER::loadExcelSheet( filename, worksheet );
			uint ck, cw;
			if ( worksheet.findId( ck ) && worksheet.findCol( ucstring("name"), cw ) ) // => 
			{
				for ( std::vector<STRING_MANAGER::TWorksheet::TRow>::iterator ip = worksheet.begin(); ip!=worksheet.end(); ++ip )
				{
					if ( ip == worksheet.begin() ) // skip first row
						continue;
					STRING_MANAGER::TWorksheet::TRow& row = *ip;
					_Keys.push_back( row[ck].toString() );
					_Words.push_back( row[cw].toUtf8() );
				}
			}
			else
				nlwarning( "WD: %s ID or name not found in %s", wordType.c_str(), filename.c_str() );
		}
	}

	if ( _Keys.empty() )
	{
		if ( wordsPath.empty() )
			nlwarning( "WD: WordsPath missing in config file %s", configFileName.c_str() );
		nlwarning( "WD: *_words_%s.txt not found", languageCode.c_str() );
		return false;
	}
	else
		return true;
}


/*
 * Set the result vector with strings corresponding to the input string:
 * - If inputStr is partially or completely found in the keys, all the matching <key,words> are returned;
 * - If inputStr is partially or completely in the words, all the matching <key, words> are returned.
 * The following tags can modify the behaviour of the search algorithm:
 * - ^mystring returns mystring only if it is at the beginning of a key or word
 * - mystring$ returns mystring only if it is at the end of a key or word
 * All returned words are in UTF8.
 */
void CWordsDictionary::lookup( const CSString& inputStr, CVectorSString& resultVec )
{
	// Prepare search string
	if ( inputStr.empty() )
		return;

	CSString searchStr = inputStr;
	bool findAtBeginning = false, findAtEnd = false;
	if ( searchStr[0] == '^' )
	{
		searchStr = searchStr.substr( 1 );
		findAtBeginning = true;
	}
	if ( searchStr[searchStr.size()-1] == '$' )
	{
		searchStr = searchStr.rightCrop( 1 );
		findAtEnd = true;
	}

	// Search
	for ( CVectorSString::const_iterator ivs=_Keys.begin(); ivs!=_Keys.end(); ++ivs )
	{
		const CSString& key = *ivs;
		string::size_type p;
		if ( (p = key.find( searchStr.c_str() )) != string::npos )
		{
			if ( ((!findAtBeginning) || (p==0)) && ((!findAtEnd) || (p==key.size()-searchStr.size())) )
				resultVec.push_back( makeResult( key, _Words[ivs-_Keys.begin()] ) );
		}
	}
	for ( CVectorSString::const_iterator ivs=_Words.begin(); ivs!=_Words.end(); ++ivs )
	{
		const CSString& word = *ivs;
		string::size_type p;
		if ( (p = word.find( searchStr.c_str() )) != string::npos )
		{
			if ( ((!findAtBeginning) || (p==0)) && ((!findAtEnd) || (p==word.size()-searchStr.size())) )
				resultVec.push_back( makeResult( _Keys[ivs-_Words.begin()], word ) );
		}
	}
}


/*
 * Make a result string
 */
inline CSString CWordsDictionary::makeResult( const CSString key, const CSString word )
{
	CSString res = key + CSString(": ") + word;
	return res;
}


/*
 * Return the key contained in the provided string returned by lookup() (without extension)
 */
CSString CWordsDictionary::getWordsKey( const CSString& resultStr )
{
	return resultStr.splitTo( ':' );
}

} // NLMISC
