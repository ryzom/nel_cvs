/** \file ps_int.cpp
 * <File description>
 *
 * $Id: ps_int.cpp,v 1.1 2001/05/08 13:37:09 vizerie Exp $
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

#include "nel/3d/ps_int.h"


namespace NL3D {


sint32 CPSIntGradient::_DefaultGradient[] = { 0, 10 } ;


CPSIntGradient::CPSIntGradient(sint32 *intTab, uint32 nbValues, uint32 nbStages, float nbCycles) 
				: CPSValueGradient<sint32>(nbCycles)
{
	_F.setValues(intTab, nbValues, nbStages) ;
}



} // NL3D
