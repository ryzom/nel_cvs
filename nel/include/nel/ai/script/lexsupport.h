/** \file lexsupport.h
 * Method and class for lex&yacc support.
 *
 * $Id: lexsupport.h,v 1.2 2001/01/08 11:16:29 chafik Exp $
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
#ifndef NL_LEX_SUPPORT_H
#define NL_LEX_SUPPORT_H

#ifndef PARSER_NO_REDEF_YYTABLE_H_
	#include "script/ytable.h"
#endif
#pragma warning (disable: 4666)
#pragma warning (disable: 4786)
#pragma warning (disable: 4275)
#pragma warning (disable: 4275)

#pragma warning (disable: 4251)
#include "agent/agent_method_def.h"
#include <list>
#include <map>
#include <string>

namespace NLAISCRIPT
{
	
	class iStack;	
	
	/**
	* Class CStringType.
	* 
	* Grammar is used in a dictionary stored in a std::map, this class define the comparison between two string.
	*
	* \author Chafik sameh
	* \author Nevrax France
	* \date 2000
	*/	
	class CStringType: public std::string
	{
		public:
			
			CStringType(): std::basic_string<char> ()
			{
			}

			CStringType(const char *T): std::basic_string<char> (T)
			{
			}

			CStringType(std::string T): std::basic_string<char> (T)
			{
			}
		
	
			///Need for the std::map.
			bool operator    <(const CStringType & a) const
			{
				return compare(a) < 0;
			}

			bool operator    >(const CStringType & a) const
			{
				return compare(a) > 0;
			}
	};

	///For a given CString the method returns the token defined in the yacc.
	int GetIdentType(char *CStringType);
	///Initialize the dictionary.
	void InitDico();
	///Erase the dictionary.
	void EraseDico();
	///get a nomber forom a given numerical string.
	float GetNombre(char *CStringType,int Base);
	
}
#endif
