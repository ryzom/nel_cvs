/** \file driver.cpp
 * Generic driver.
 * Low level HW classes : CTexture, Cmaterial, CVertexBuffer, CPrimitiveBlock, IDriver
 *
 * $Id: driver.cpp,v 1.1 2000/11/07 15:34:57 berenguier Exp $
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


#include "nel/misc/types_nl.h"
#include "nel/3d/driver.h"
using namespace std;
using namespace NLMISC;


namespace NL3D
{


// ***************************************************************************
GfxMode::GfxMode(uint16 w, uint16 h, uint8 d, bool windowed)
{
	Windowed= windowed;
	Width= w;
	Height= h;
	Depth= d;
}


}