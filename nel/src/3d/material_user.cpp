/** \file material_user.cpp
 * <File description>
 *
 * $Id: material_user.cpp,v 1.5 2004/04/26 17:15:43 corvazier Exp $
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

#include "std3d.h"


#include "3d/material_user.h"
#include "nel/3d/u_driver.h"
#include "3d/driver_user.h"


namespace NL3D
{

bool CMaterialUser::isSupportedByDriver(UDriver &drv)
{
	NL3D_MEM_MATERIAL		
	return _Material.isSupportedByDriver(*(NLMISC::safe_cast<CDriverUser *>(&drv)->getDriver()));
}

void CMaterialUser::setAlphaTest(bool active)
{
	_Material.setAlphaTest(active);
}

bool CMaterialUser::getAlphaTest() const
{
	return _Material.getAlphaTest();
}

void CMaterialUser::setAlphaTestThreshold(float threshold)
{
	_Material.setAlphaTestThreshold(threshold);
}

float CMaterialUser::getAlphaTestThreshold() const
{
	return _Material.getAlphaTestThreshold();
}

} // NL3D
