/* pt_callback_item.h
 *
 * Copyright, 2000 Nevrax Ltd.
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

/*
 * $Id: pt_callback_item.h,v 1.1 2000/10/02 16:42:23 cado Exp $
 *
 * <Replace this by a description of the file>
 */

#ifndef NL_PT_CALLBACK_ITEM_H
#define NL_PT_CALLBACK_ITEM_H

#include "nel/net/socket.h"

namespace NLNET {


/// Callback function for message processing
typedef void (*TMsgCallback) ( CMessage&, TSenderId );


/// Callback items
typedef struct
{
	char			*Key;
	TMsgCallback	Callback;

} TCallbackItem;


/**
 * Elements of the set used for fast search in callback arrays
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CPtCallbackItem
{
public:

	/// Constructor
	CPtCallbackItem( const char *key )
	{
		_Key = (char*)key; // TODO: or strdup() ?
		_Pt = NULL;
	}

	/// Constructor
	CPtCallbackItem( const TCallbackItem *pt )
	{
		_Pt = (TCallbackItem*)pt;
		_Key = NULL;
	}

	/// Return pointer
	TCallbackItem *pt() const
	{
		return _Pt;
	}

	/// Return key (either internal or linked by pt())
	char	*key() const
	{
		if ( _Pt == NULL )
		{
			return (_Key);
		}
		else
		{
			return _Pt->Key;
		}
	}

	/// Comparison operator
	friend bool operator< ( const CPtCallbackItem& left, const CPtCallbackItem& right )
	{
		return ( std::string(left.key()) < std::string(right.key()) );
	}

private:

	TCallbackItem	*_Pt;
	char			*_Key;

};


} // NLNET


#endif // NL_PT_CALLBACK_ITEM_H

/* End of pt_callback_item.h */
