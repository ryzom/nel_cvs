/** \file calc_lm.h
 * LightMap Calculation settings
 *
 * $Id: calc_lm.h,v 1.4 2002/02/26 17:30:25 corvazier Exp $
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

#ifndef NL_CALC_LM_H
#define NL_CALC_LM_H

// ***********************************************************************************************

#include "nel/misc/rgba.h"

#include <string>
#include <set>

// ***********************************************************************************************

#define MAXLIGHTMAPSIZE		1024

// ***********************************************************************************************

#define NEL3D_APPDATA_LM			41654684
#define NEL3D_APPDATA_LM_GROUPNAME	(NEL3D_APPDATA_LM+1)
#define NEL3D_APPDATA_LM_ANIMATED	(NEL3D_APPDATA_LM+2)

// ***********************************************************************************************
extern void convertToWorldCoordinate	(NL3D::CMesh::CMeshBuild *pMB, 
										NL3D::CMeshBase::CMeshBaseBuild *pMBB, 
										NLMISC::CVector &translation = NLMISC::CVector(0.0f, 0.0f, 0.0f));

extern NLMISC::CMatrix getObjectToWorldMatrix	(NL3D::CMesh::CMeshBuild *pMB, 
												NL3D::CMeshBase::CMeshBaseBuild *pMBB);

// ***********************************************************************************************
// Light representation from max
struct SLightBuild
{
	std::string GroupName;
	enum EType { LightAmbient, LightPoint, LightDir, LightSpot };
	EType			Type;
	NLMISC::CVector Position;				// Used by LightPoint and LightSpot
	NLMISC::CVector Direction;				// Used by LightSpot and LightDir
	float			rRadiusMin, rRadiusMax;	// Used by LightPoint and LightSpot
	float			rHotspot, rFallof;		// Used by LightSpot
	NLMISC::CRGBA	Ambient;
	NLMISC::CRGBA	Diffuse;
	NLMISC::CRGBA	Specular;
	bool			bCastShadow;
	bool			bAmbientOnly;			// true if LightPoint|LightSpot|LightDir with GetAmbientOnly().
	float			rMult;

	NLMISC::CBitmap ProjBitmap;				// For projector (bitmap)
	NLMISC::CMatrix mProj;					// For projector (matrix)

	float			rDirRadius;				// Accel for directionnal lights : Radius of the 
											// cylinder passing trough the bounding sphere of the 
											// object under consideration

	std::set<std::string> setExclusion;		// List of object name excluded by this light

	float			rSoftShadowRadius;		// The radius max used when calculating soft shadows
	float			rSoftShadowConeLength;	// The distance between vertex and cylinder beginning
	
	// -------------------------------------------------------------------------------------------

	SLightBuild ();
	bool canConvertFromMaxLight (INode *node, TimeValue tvTime);
	void convertFromMaxLight (INode *node,TimeValue tvTime);

};

// ***********************************************************************************************

#endif // NL_CALC_LM_H

/* End of calc_lm.h */
