/** \file material.cpp
 * CMaterial implementation
 *
 * $Id: material.cpp,v 1.7 2000/11/14 13:23:21 berenguier Exp $
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

#include "nel/3d/driver.h"

using std::vector;

namespace NL3D
{


// ***************************************************************************

void	CMaterial::initUnlit()
{
	 setShader(normal);
	 setLighting(false);
	 setColor(CRGBA(255,255,255,255));
	 setTexture(NULL, 0);
	 setTexture(NULL, 1);
	 setTexture(NULL, 2);
	 setTexture(NULL, 3);
	 setZBias(0);
	 setZFunction(lessequal);
	 setBlend(false);
}

// ***************************************************************************

void	CMaterial::initLighted()
{
	 setShader(normal);
	 setLighting(true);
	 setColor(CRGBA(255,255,255,255));
	 setTexture(NULL, 0);
	 setTexture(NULL, 1);
	 setTexture(NULL, 2);
	 setTexture(NULL, 3);
	 setZBias(0);
	 setZFunction(lessequal);
	 setBlend(false);
}


}

