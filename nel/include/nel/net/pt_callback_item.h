/** \file pt_callback_item.h
 * CPtCallbackItem class
 *
 * $Id: pt_callback_item.h,v 1.6 2000/11/14 15:58:34 cado Exp $
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

#ifndef NL_PT_CALLBACK_ITEM_H
#define NL_PT_CALLBACK_ITEM_H

#include "nel/net/socket.h"

namespace NLNET {


/// Callback function for message processing
typedef void (*TMsgCallback) ( CMessage&, TSenderId );


/// Callback items. See CMsgSocket::update() for an explanation on how the callbacks are called.
typedef struct
{
	/// Key C string. It is a message type name, or "C" for connection or "D" for disconnection
	char			*Key;
	/// The callback function
	TMsgCallback	Callback;

} TCallbackItem;


/**
 * Elements of the set used for fast search in callback arrays.
 * A CPtCallbackItem object can contain either a pointer to a callback item {Key,Callback} 
 * (it is the case when it is saved in the search set), or a key (use it when you want
 * to find a callback item by name).
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CPtCallbackItem
{
public:

	/// Constructor for searching
	CPtCallbackItem( const char *key )
	{
		_Key = (char*)key; // no need for using strdup()
		_Pt = NULL;
		// Not modifying _BindSent
	}

	/// Constructor for registering
	CPtCallbackItem( const TCallbackItem *pt )
	{
		_Pt = (TCallbackItem*)pt;
		_Key = NULL;
		_BindSent = false;
	}

	/// Return pointer
	TCallbackItem *pt() const
	{
		return _Pt;
	}

	/// Call this method when sending a binding message
	void	setBindSentFlag()
	{
		_BindSent = true;
	}

	/// Returns true if a binding message has been sent
	bool	bindSent() const
	{
		return _BindSent;
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
	bool			_BindSent;

};


} // NLNET


#endif // NL_PT_CALLBACK_ITEM_H

/* End of pt_callback_item.h */
