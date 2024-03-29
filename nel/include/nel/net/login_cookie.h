/** \file login_cookie.h
 * container used by the login process to identify a user
 *
 * $Id: login_cookie.h,v 1.9 2006/09/14 16:56:08 cado Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX NeL Network Services.
 * NEVRAX NeL Network Services is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NeL Network Services is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_LOGIN_COOKIE_H
#define NL_LOGIN_COOKIE_H

#include "nel/misc/types_nl.h"

#include "nel/misc/stream.h"
#include "nel/misc/common.h"


namespace NLNET {


/**
 * Manage cookie during the authenticate procedure.
 *
 * _UserAddr is the ipv4 address of the client in uint32
 * _UserKey is an uint32 generated by the login_service at each login password verification
 * _UserId is an uint32 uniq for each account (an account could have more than one avatar)
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CLoginCookie
{
public:

	CLoginCookie (uint32 addr, uint32 id);
	CLoginCookie () : _Valid(false) { }

	void serial (NLMISC::IStream &s)
	{
		// verify that we initialized the cookie before writing it
		if (!s.isReading() && !_Valid) nlwarning ("LC: serialize a non valid cookie");

		s.serial (_UserAddr);
		s.serial (_UserKey);
		s.serial (_UserId);

		if (s.isReading()) _Valid = true;
	}

	std::string setToString () const
	{
		if (_Valid)
		{
			char cstr[8*3+2+1];
			NLMISC::smprintf(cstr, 8*3+2+1, "%08X|%08X|%08X", _UserAddr, _UserKey, _UserId);
			nlinfo ("LC: setToString %s -> %s", toString().c_str (), cstr);
			return cstr;
		}
		else
		{
			return "0|0|0";
		}
	}

	void setFromString (const std::string &str)
	{
		sscanf(str.c_str(), "%08X|%08X|%08X", &_UserAddr, &_UserKey, &_UserId);

		if(str.empty () || (_UserAddr==0 && _UserKey==0 && _UserId==0))
			_Valid = 0;
		else
			_Valid = 1;

		nlinfo ("LC: setFromString %s -> %s, isValid: %d", str.c_str (), toString().c_str (), _Valid);
	}

	std::string toString () const
	{
		if (_Valid)
			return "'" + NLMISC::toString("%08X", (unsigned int)_UserAddr) + "|" + NLMISC::toString("%08X", (unsigned int)_UserKey) + "|" + NLMISC::toString("%08X", (unsigned int)_UserId) + "'";
		else
			return "<InvalidCookie>";
	}

	uint32	getUserAddr () const { nlassert (_Valid); return _UserAddr; }
	uint32	getUserKey () const { nlassert (_Valid); return _UserKey; }
	uint32	getUserId () const { nlassert (_Valid); return _UserId; }

	void	set (uint32 ua, uint32 uk, uint32 ui) { _Valid = true; _UserAddr = ua; _UserKey = uk; _UserId = ui; }

	bool	isValid() const { return _Valid; }
	void	clear () { _Valid = false; }

	uint32	generateKey();

	/// Comparison == operator
	friend bool operator== (const CLoginCookie &c1, const CLoginCookie &c2);

	/// Strict weak ordering operator
	bool operator <(const CLoginCookie &other) const
	{
		if(_UserAddr != other._UserAddr)
			return _UserAddr < other._UserAddr;
		if(_UserKey != other._UserKey)
			return _UserKey < other._UserKey;
		return _UserId < other._UserId;
	}

private:

	bool	_Valid;

	uint32	_UserAddr;
	uint32	_UserKey;
	uint32	_UserId;

};

/*
 * Comparison == operator
 */
bool operator== (const CLoginCookie &c1, const CLoginCookie &c2);
/*
 * Comparison != operator
 */
bool operator!= (const CLoginCookie &c1, const CLoginCookie &c2);

} // NLNET



#endif // NL_LOGIN_COOKIE_H

/* End of login_cookie.h */
