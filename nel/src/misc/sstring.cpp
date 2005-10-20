/** \file sstring.cpp
 *
 * This file contains a string class derived from the STL string
 * The string compare functions of the class are case insensitive 
 *
 * The coding style is not CPU efficient - the routines are not designed for performance
 *
 * $Id: sstring.cpp,v 1.2 2005/10/20 13:16:26 miller Exp $
 */

#include "stdmisc.h"
#include "nel/misc/sstring.h"

namespace NLMISC
{

	CSString CSString::strtok(	const char *separators,
										bool useSmartExtensions,			// if true then match brackets etc (and refine with following args)
										bool useAngleBrace,					// - treat '<' and '>' as brackets
										bool useSlashStringEscape,			// - treat '\' as escape char so "\"" == '"'
										bool useRepeatQuoteStringEscape)	// - treat """" as '"')
	{
		if (useSmartExtensions)
		{
			CSString token;
			
			// split to the first non empty token, or until the this string is empty
			while (!empty() && token.empty())
				token = splitToOneOfSeparators(separators,true,useAngleBrace,useSlashStringEscape,useRepeatQuoteStringEscape,true);

			return token;
		}

		unsigned int i;
		CSString result;

		// skip leading junk
		for (i=0;i<size();++i)
		{
			// look for the next character in the 'separator' character list supplied
			unsigned j;
			for (j=0;separators[j] && (*this)[i]!=separators[j];++j)
			{}
			// if not found then we're at end of leading junk
			if (!separators[j])
				break;
		}

		// copy out everything up to the next separator character
		for (;i<size();++i)
		{
			// look for the next character in the 'separator' character list supplied
			unsigned j;
			for (j=0;separators[j] && (*this)[i]!=separators[j];++j)
			{}
			// if not found then we're at end of text chunk
			if (separators[j])
				break;
			result+=(*this)[i];
		}

		// skip trailing junk
		for (;i<size();++i)
		{
			// look for the next character in the 'separator' character list supplied
			unsigned j;
			for (j=0;separators[j] && (*this)[i]!=separators[j];++j)
			{}
			// if not found then we're at end of leading junk
			if (!separators[j])
				break;
		}

		// delete the treated bit from this string
		(*this)=substr(i);

		return result;
	}


	CSString CSString::splitToOneOfSeparators(	const CSString& separators,
													bool truncateThis,
													bool useAngleBrace,					// treat '<' and '>' as brackets
													bool useSlashStringEscape,			// treat '\' as escape char so "\"" == '"'
													bool useRepeatQuoteStringEscape,	// treat """" as '"'
													bool truncateSeparatorCharacter)	// if true tail begins after separator char
	{
		// iterate over our string
		uint32 i;
		for (i=0;i<size();++i)
		{
			char thisChar=(*this)[i];

			// if we've found the separator character then all's cool so break out of the loop
			if (separators.contains(thisChar))
				break;

			// if we have a bracket or quote of any type then match to it's matching bracket, quote or whatever
			if (isOpeningDelimiter(thisChar,useAngleBrace) || isStringDelimiter(thisChar))
			{
				if (i != 0)
				{
					// we are not at begining of the string, delimiter is considered as separator
					break;
				}
				uint32 j=i;
				i=findMatchingDelimiterPos(useAngleBrace,useSlashStringEscape,useRepeatQuoteStringEscape,i);
				// if there was a problem then break here
				if (j==i)
					break;
				continue;
			}
		}

		// build the return string
		CSString result=left(i);

		// if need be truncate '*this' before returning
		if (truncateThis)
		{
			if (truncateSeparatorCharacter && separators.contains((*this)[i]))
				++i;
			*this=leftCrop(i);
		}

		return result;
	}

} // namespace NLMISC

