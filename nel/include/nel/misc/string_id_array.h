/** \file string_id_array.h
 * <File description>
 *
 * $Id: string_id_array.h,v 1.2 2001/03/05 09:29:54 lecroart Exp $
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

#ifndef NL_STRING_ID_ARRAY_H
#define NL_STRING_ID_ARRAY_H

#include <string>
#include <vector>

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"


namespace NLMISC {


/**
 * <Class description>
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CStringIdArray
{
public:

	typedef sint16 TStringId;

	void addString(const std::string &str, TStringId id)
	{
		nlassert (id > 0 && id < sizeof (TStringId));

		if (id > (sint32) _StringArray.size())
			_StringArray.resize(id+1);

		_StringArray[id] = str;
	}

	void addString(const std::string &str)
	{
		nlassert (_StringArray.size () < sizeof (TStringId));

		// add at the end
		addString (str, _StringArray.size ());
	}

	TStringId getId (const std::string &str) const
	{
		for (TStringId i = 0; i < (TStringId) _StringArray.size(); i++)
		{
			if (_StringArray[i] == str)
				return i;
		}
		return -1;
	}

	std::string getString (TStringId id) const
	{
		nlassert (id > 0 && id < (TStringId)_StringArray.size());

		return _StringArray[id];
	}

	void resize (TStringId size)
	{
		_StringArray.resize (size);
	}

	TStringId size () const
	{
		return _StringArray.size ();
	}

private:
	std::vector<std::string>	_StringArray;

	// todo: liste des string demande qui ont pas d assoc => a l update prochain, il faudra les demander
	std::vector<std::string>	_StringArray;
};


} // NLMISC


#endif // NL_STRING_ID_ARRAY_H

/* End of string_id_array.h */
