/** \file landscape_def.h
 * Global Definitions for Landscape
 *
 * $Id: landscape_def.h,v 1.1 2001/09/14 09:44:26 berenguier Exp $
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

#ifndef NL_LANDSCAPE_DEF_H
#define NL_LANDSCAPE_DEF_H

#include "nel/misc/types_nl.h"


namespace NL3D 
{

// ***************************************************************************
// 4th pass is always the Lightmapped one (Lightmap*clouds).
#define	NL3D_MAX_TILE_PASS 5
// There is no Face for lightmap, since lightmap pass share the RGB0 face.
#define	NL3D_MAX_TILE_FACE	NL3D_MAX_TILE_PASS-1

#define	NL3D_TILE_PASS_RGB0		0
#define	NL3D_TILE_PASS_RGB1		1
#define	NL3D_TILE_PASS_RGB2		2
#define	NL3D_TILE_PASS_ADD		3
#define	NL3D_TILE_PASS_LIGHTMAP	4
// NB: RENDER ORDER: CLOUD*LIGHTMAP is done BEFORE ADDITIVE.




} // NL3D


#endif // NL_LANDSCAPE_DEF_H

/* End of landscape_def.h */
