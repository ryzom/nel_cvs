/** \file rect.cpp
 * <File description>
 *
 * $Id: rect.cpp,v 1.2 2000/12/22 10:42:05 corvazier Exp $
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

#include "nel/misc/rect.h"


namespace NLMISC {

// *********************************************************************
void CRect::extend (sint32 x, sint32 y)
{
	if (x<X)
	{
		Width+=(uint32)(X-x);
		x=X;
	}
	else if (x>=(X+(sint32)Width))
		Width=(uint32)(x-X+1);
	if (y<Y)
	{
		Height+=(uint32)(Y-y);
		y=Y;
	}
	else if (y>=(Y+(sint32)Height))
		Height=(uint32)(y-Y+1);
}


} // NLMISC
