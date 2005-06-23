/** \file module_common.cpp
 * basic type and forward declaration for module system
 *
 * $Id: module_common.cpp,v 1.1 2005/06/23 16:38:14 boucher Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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


#include "stdnet.h"
#include "nel/misc/sstring.h"
#include "nel/net/module_common.h"

using namespace std;
using namespace NLMISC;

namespace NLNET
{

	bool TModuleInitInfo::parseParamList(const std::string &rawParamString)
	{
		// Cleanup the struct
		SubParams.clear();
		ParamName.resize(0);
		ParamValue.resize(0);

		return _parseParamList(rawParamString);
	}


	bool TModuleInitInfo::_parseParamList(const std::string &rawParamString)
	{
		CSString parsedString(rawParamString);
		
		for (CSString part = parsedString.strtok(" \t", true, false);
			!part.empty();
			part = parsedString.strtok(" \t", true, false))
		{
			if (part[0] == '(')
			{
				// this is a sub parameter list
				if (SubParams.empty() || SubParams.back().ParamName.empty())
				{
					nlwarning("While parsing param string '%s', missing param header", rawParamString.c_str());
					return false;
				}
				if (!SubParams.back().ParamValue.empty())
				{
					nlwarning("While parsing param string '%s', Invalid sub param header '%s' for sub part '%s', must not define value",
						rawParamString.c_str(),
						SubParams.back().ParamName.c_str(),
						part.c_str());

					return false;
				}

				if (part[part.size()-1] != ')')
				{
					nlwarning("While parsing param string '%s', Invalid sub param value '%s' missing closing ')'",
						rawParamString.c_str(),
						part.c_str());

					return false;
				}

				part = part.stripBlockDelimiters();

				if (!SubParams.back()._parseParamList(part))
				{
					nlwarning("Error parsing sub param list for header '%s' in '%s'", 
						SubParams.back().ParamName.c_str(),
						rawParamString.c_str());
					return false;
				}
			}
			else if (part[part.size()-1] == ')')
			{
				nlwarning("While parsing param string '%s', Invalid param value '%s' : missing openning '('",
					rawParamString.c_str(),
					part.c_str());

				return false;
			}
			else if (part[0] == '\"')
			{
				// this is a quoted parameter value
				if (SubParams.empty() || !SubParams.back().ParamValue.empty())
				{
					nlwarning("While parsing param string '%s', param '%s' already have the value '%s'", 
						rawParamString.c_str(),
						SubParams.back().ParamName.c_str(),
						SubParams.back().ParamValue.c_str());
					return false;
				}
				SubParams.back().ParamValue = part.unquote();
			}
			else
			{
				// this is a simple param
				CSString name = part.splitTo('=', true, true);
				if (name.empty())
				{
					nlwarning("Can't find param name for value '%s' in the param string '%s'",
						part.c_str(),
						rawParamString.c_str());
					return false;
				}
				CSString value = part.strtok("=");

				SubParams.push_back(TModuleInitInfo());
				SubParams.back().ParamName = name;
				SubParams.back().ParamValue = value;
			}
		}

		return true;
	}

	const TModuleInitInfo *TModuleInitInfo::getParam(const std::string &name) const
	{
		vector<string>	parts;
		NLMISC::explode(name, ".", parts);

		return _getParam(parts.begin(), parts.end());
	}

	const TModuleInitInfo *TModuleInitInfo::_getParam(std::vector<std::string>::iterator it, std::vector<std::string>::iterator end) const
	{
		if (it == end)
		{
			// end of recursion, we found the searched param
			return this;
		}

		// look for sub param
		for (uint i=0; i<SubParams.size(); ++i)
		{
			if (SubParams[i].ParamName == *it)
				return SubParams[i]._getParam(++it, end);
		}

		// parameter not found
		return NULL;
	}


} // namespace NLMISC
