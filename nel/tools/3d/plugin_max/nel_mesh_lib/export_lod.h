/** \file export_lod.h
 * LOD define
 *
 * $Id: export_lod.h,v 1.1 2001/07/04 16:38:39 corvazier Exp $
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

#ifndef NL_EXPORT_LOD_H
#define NL_EXPORT_LOD_H

// ***************************************************************************

#define NEL3D_APPDATA_LOD							1423062537
#define NEL3D_APPDATA_LOD_BLEND_IN					(NEL3D_APPDATA_LOD+0)
#define NEL3D_APPDATA_LOD_BLEND_OUT					(NEL3D_APPDATA_LOD+1)
#define NEL3D_APPDATA_LOD_COARSE_MESH				(NEL3D_APPDATA_LOD+2)
#define NEL3D_APPDATA_LOD_DYNAMIC_MESH				(NEL3D_APPDATA_LOD+3)
#define NEL3D_APPDATA_LOD_DIST_MAX					(NEL3D_APPDATA_LOD+5)
#define NEL3D_APPDATA_LOD_BLEND_LENGTH				(NEL3D_APPDATA_LOD+6)
#define NEL3D_APPDATA_LOD_NAME_COUNT				(NEL3D_APPDATA_LOD+7)
#define NEL3D_APPDATA_LOD_NAME						(NEL3D_APPDATA_LOD+8)

// ***************************************************************************

#define NEL3D_APPDATA_LOD_NAME_COUNT_MAX			10

// ***************************************************************************

#define NEL3D_APPDATA_LOD_DIST_MAX_DEFAULT			100.f
#define NEL3D_APPDATA_LOD_BLEND_LENGTH_DEFAULT		5.f

// ***************************************************************************

#endif // NL_EXPORT_LOD_H

/* End of export_lod.h */
