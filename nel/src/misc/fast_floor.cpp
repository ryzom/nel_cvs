/** \file fast_floor.cpp
 * <File description>
 *
 * $Id: fast_floor.cpp,v 1.1 2003/07/30 16:09:56 vizerie Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "nel/misc/fast_floor.h"
#include <math.h>

namespace NLMISC {

double	OptFastFloorMagicConst = pow(2,52) + pow(2,51);
int OptFastFloorBkupCW ;
float	OptFastFloorMagicConst24 = (float)pow(2,23);
int OptFastFloorBkupCW24 ;

} // NLMISC
