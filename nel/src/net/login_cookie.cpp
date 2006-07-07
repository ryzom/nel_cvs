/** \file login_cookie.cpp
 * TODO: File description
 *
 * $Id: login_cookie.cpp,v 1.6.14.1 2006/07/07 08:38:14 boucher Exp $
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

#include "stdnet.h"

#include "nel/net/login_cookie.h"

using namespace std;
using namespace NLMISC;


namespace NLNET {


/*
 * Comparison == operator
 */
bool operator== (const CLoginCookie &c1, const CLoginCookie &c2)
{
	nlassert (c1._Valid && c2._Valid);

	return c1._UserAddr==c2._UserAddr && c1._UserKey==c2._UserKey && c1._UserId==c2._UserId;
}

/*
 * Comparison != operator
 */
bool operator!= (const CLoginCookie &c1, const CLoginCookie &c2)
{
	return !(c1 == c2);
}

CLoginCookie::CLoginCookie (uint32 addr, uint32 id) : _Valid(true), _UserAddr(addr), _UserId(id)
{
	// generates the key for this cookie
	_UserKey = generateKey();
}

/// \todo ace: is the cookie enough to avoid hackers to predice keys?

uint32 CLoginCookie::generateKey()
{
	uint32 t = (uint32)time (NULL);
    srand (t);

	uint32 r = rand ();
	static uint32 n = 0;
	n++;

	// 12bits for the time (in second) => loop in 1 hour
	//  8bits for random => 256 case
	// 12bits for the inc number => can generate 4096 keys per second without any problem (if you generate more than this number, you could have 2 same keys)
	return (t&0xFFF)<<20 | (r&0xFF)<<12 | (n&0xFFF);

	// 12bits for the time (in second) => loop in 1 hour
	// 20bits for the inc number => can generate more than 1 million keys per second without any problem (never exceed on my computer)
//	return (t&0xFFF)<<20 | (n&0xFFFFF);
}


/* test key generation
void main()
{
	set<uint32> myset;

	// generates the key for this cookie
	while (true)
	{
		uint32 val = (t&0xFFF)<<20 | (r&0xFF)<<12 | (n&0xFFF);
		pair<set<uint32>::iterator,bool> p = myset.insert (val);
		if (!p.second) printf("%10u 0x%x already inserted\n", val, val);
	}
}
*/

} // NL.
