/** \file ps_direction.h
 * <File description>
 *
 * $Id: ps_direction.h,v 1.1 2001/07/04 12:33:24 vizerie Exp $
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

#ifndef NL_PS_DIRECTION_H
#define NL_PS_DIRECTION_H

#include "nel/misc/types_nl.h"


namespace NL3D {


/** this class is an interface for object for a particle system that need a direction (forces, emitter...)
 * 
 */

class CPSDirection
{
public :	
	virtual void setDir(const CVector &v) = 0 ;
	virtual CVector getDir(void) const = 0 ;
} ;


} // NL3D


#endif // NL_PS_DIRECTION_H

/* End of ps_direction.h */
