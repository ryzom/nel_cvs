/** \file export_lod.h
 * LOD define
 *
 * $Id: export_lod.h,v 1.9 2001/11/14 15:43:32 vizerie Exp $
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

#define NEL3D_APPDATA_LOD_NAME_COUNT_MAX			10

// ***************************************************************************

#define NEL3D_APPDATA_LOD							1423062537
#define NEL3D_APPDATA_LOD_NAME_COUNT				(NEL3D_APPDATA_LOD)
#define NEL3D_APPDATA_LOD_NAME						(NEL3D_APPDATA_LOD_NAME_COUNT+1)
#define NEL3D_APPDATA_LOD_BLEND_IN					(NEL3D_APPDATA_LOD_NAME+NEL3D_APPDATA_LOD_NAME_COUNT_MAX)
#define NEL3D_APPDATA_LOD_BLEND_OUT					(NEL3D_APPDATA_LOD_BLEND_IN+1)
#define NEL3D_APPDATA_LOD_COARSE_MESH				(NEL3D_APPDATA_LOD_BLEND_OUT+1)
#define NEL3D_APPDATA_LOD_DYNAMIC_MESH				(NEL3D_APPDATA_LOD_COARSE_MESH+1)
#define NEL3D_APPDATA_LOD_DIST_MAX					(NEL3D_APPDATA_LOD_DYNAMIC_MESH+1)
#define NEL3D_APPDATA_LOD_BLEND_LENGTH				(NEL3D_APPDATA_LOD_DIST_MAX+1)
#define NEL3D_APPDATA_LOD_MRM						(NEL3D_APPDATA_LOD_BLEND_LENGTH+1)
#define NEL3D_APPDATA_LOD_SKIN_REDUCTION			(NEL3D_APPDATA_LOD_MRM+1)
#define NEL3D_APPDATA_LOD_NB_LOD					(NEL3D_APPDATA_LOD_SKIN_REDUCTION+1)
#define NEL3D_APPDATA_LOD_DIVISOR					(NEL3D_APPDATA_LOD_NB_LOD+1)
#define NEL3D_APPDATA_LOD_DISTANCE_FINEST			(NEL3D_APPDATA_LOD_DIVISOR+1)
#define NEL3D_APPDATA_LOD_DISTANCE_MIDDLE			(NEL3D_APPDATA_LOD_DISTANCE_FINEST+1)
#define NEL3D_APPDATA_LOD_DISTANCE_COARSEST			(NEL3D_APPDATA_LOD_DISTANCE_MIDDLE+1)

// Last APPDATA_LOD value is						1423062560

#define NEL3D_APPDATA_ACCEL							(1423062561)
#define NEL3D_APPDATA_INSTANCE_NAME					(1423062562)
#define NEL3D_APPDATA_DONT_ADD_TO_SCENE				(1423062563)
#define NEL3D_APPDATA_IGNAME						(1423062564)

#define NEL3D_APPDATA_DONTEXPORT					(1423062565)
#define NEL3D_APPDATA_EXPORT_NOTE_TRACK				(1423062566)

#define NEL3D_APPDATA_LUMELSIZEMUL					(1423062567)
#define NEL3D_APPDATA_SOFTSHADOW_RADIUS				(1423062568)
#define NEL3D_APPDATA_SOFTSHADOW_CONELENGTH			(1423062569)

#define NEL3D_APPDATA_FLOATING_OBJECT			    (1423062570)

// ***************************************************************************

#define NEL3D_APPDATA_LOD_BLEND_IN_DEFAULT			1
#define NEL3D_APPDATA_LOD_BLEND_OUT_DEFAULT			1
#define NEL3D_APPDATA_LOD_COARSE_MESH_DEFAULT		0
#define NEL3D_APPDATA_LOD_DYNAMIC_MESH_DEFAULT		0
#define NEL3D_APPDATA_LOD_DIST_MAX_DEFAULT			100.f
#define NEL3D_APPDATA_LOD_BLEND_LENGTH_DEFAULT		5.f
#define NEL3D_APPDATA_LOD_SKIN_REDUCTION_DEFAULT	1
#define NEL3D_APPDATA_LOD_NB_LOD_DEFAULT			11
#define NEL3D_APPDATA_LOD_DIVISOR_DEFAULT			20
#define NEL3D_APPDATA_LOD_DISTANCE_FINEST_DEFAULT	5.f
#define NEL3D_APPDATA_LOD_DISTANCE_MIDDLE_DEFAULT	30.f
#define NEL3D_APPDATA_LOD_DISTANCE_COARSEST_DEFAULT	200.f

#define NEL3D_APPDATA_SOFTSHADOW_RADIUS_DEFAULT		1.4f
#define NEL3D_APPDATA_SOFTSHADOW_CONELENGTH_DEFAULT	15.0f

// ***************************************************************************

#endif // NL_EXPORT_LOD_H

/* End of export_lod.h */
