/** \file algo.cpp
 * <File description>
 *
 * $Id: algo.cpp,v 1.2 2002/11/12 15:57:28 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#include "nel/misc/algo.h"
#include <string>


using	namespace std;


namespace NLMISC 
{


// ***************************************************************************
bool		testWildCard(const char *strIn, const char *wildCard)
{
	// run the 2 string in //el
	while(*wildCard!=0 && *strIn!=0)
	{
		// if same char, continue.
		if(*wildCard==*strIn)
		{
			wildCard++;
			strIn++;
		}
		// if wildCard is ?, continue
		else if(*wildCard=='?')
		{
			wildCard++;
			strIn++;
		}
		// if wildcard is *, recurs check.
		else if(*wildCard=='*')
		{
			wildCard++;
			// if last *, its OK.
			if(*wildCard==0)
				return true;
			// else must check next strings.
			else
			{
				// build the wilcard token. eg from "*pipo?", take "pipo"
				string	token;
				while(*wildCard!='*' && *wildCard!='?' && *wildCard!=0)
				{
					token+= *wildCard;
					wildCard++;
				}
				// if token size is empty, error
				if(token.empty())
					return false;

				// in strIn, search all the occurence of token. For each solution, recurs test.
				string	sCopy= strIn;
				uint	pos= sCopy.find(token, 0);
				while(pos!=string::npos)
				{
					// do a testWildCard test on the remaining string/wildCard
					if( testWildCard(strIn+pos+token.size(), wildCard) )
						// if succeed, end
						return true;
					// fails=> test with an other occurence of token in the string.
					pos= sCopy.find(token, pos+1);
				}
				
				// if all failed, fail
				return false;
			}
		}
		// else fail
		else
			return false;
	}

	// If quit here because end Of 2 strs, OK.
	if(*wildCard==0 && *strIn==0)
		return true;
	// if quit here because wildCard=="*" and s="", OK too.
	if(*strIn==0 && wildCard[0]=='*' && wildCard[1]==0)
		return true;

	/*
		Else false:
			It may be wildCard="?aez" and s="" => error
			It may be wildCard="" and s="aer" => error
	*/
	return false;
}



} // NLMISC
