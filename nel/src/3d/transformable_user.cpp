/** \file transformable_user.cpp
 * <File description>
 *
 * $Id: transformable_user.cpp,v 1.5 2002/10/28 17:32:13 corvazier Exp $
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

#include "3d/transformable_user.h"


namespace NL3D 
{


const char *UTransformable::getPosValueName ()
{
	NL3D_MEM_TRANSFORMABLE
	return ITransformable::getPosValueName ();
}
const char *UTransformable::getRotEulerValueName()
{
	NL3D_MEM_TRANSFORMABLE
	return ITransformable::getRotEulerValueName();
}
const char *UTransformable::getRotQuatValueName()
{
	NL3D_MEM_TRANSFORMABLE
	return ITransformable::getRotQuatValueName();
}
const char *UTransformable::getScaleValueName()
{
	NL3D_MEM_TRANSFORMABLE
	return ITransformable::getScaleValueName();
}
const char *UTransformable::getPivotValueName()
{
	NL3D_MEM_TRANSFORMABLE
	return ITransformable::getPivotValueName();
}


} // NL3D
