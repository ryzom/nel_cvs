/** \file patchdlm_context.h
 * <File description>
 *
 * $Id: patchdlm_context.h,v 1.4 2002/04/16 13:58:53 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#ifndef NL_PATCHDLM_CONTEXT_H
#define NL_PATCHDLM_CONTEXT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/object_vector.h"
#include "3d/tess_list.h"
#include "nel/misc/bsphere.h"
#include "nel/misc/aabbox.h"
#include "3d/landscape_def.h"


namespace NL3D 
{


class	CPatch;
class	CTextureDLM;
class	CPointLight;
class	CPatchDLMContextList;

// ***************************************************************************
/**
 * A PointLight for Dynamic LightMap (DLM) context for a patch.
 *	It contains precomputed values.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CPatchDLMPointLight
{
public:
	// Diffuse Color of the Spot. 0..255
	float		R, G, B;
	// Is this a spot? NB: if false, cosMin/cosMax are still well computed for correctLighting (cosMax=-1, cosMin= -2).
	bool		IsSpot;
	// World Position of the spot
	CVector		Pos;
	// Direction of the spot, normalized
	CVector		Dir;
	// cosMax, where influence==1.
	float		CosMax;
	// cosMin, where influence==0. NB: cosMax>cosMin (ie angleMax<angleMin)
	float		CosMin;
	// 1.f / (cosMax-cosMin);
	float		OOCosDelta;
	// Attenuation distance, where influence==0.
	float		AttMax;
	// Attenuation distance, where influence==1. NB: attMax>attMin
	float		AttMin;
	// 1.f / (attMin-attMax);
	float		OOAttDelta;


	// The estimated sphere which englobe the light. NB: approximated for SpotLight
	NLMISC::CBSphere	BSphere;
	// The BBox which englobe the light. NB: Sphere and Box are best fit to the light, ie bbox may not
	// englobe the sphere and vice versa
	NLMISC::CAABBox		BBox;


public:
	// compile from a pointlight. NB: attenuation end is clamped to maxAttEnd (must be >0)
	void		compile(const CPointLight &pl, float maxAttEnd= 30.f);
};


// ***************************************************************************
/**
 * A Dynamic LightMap (DLM) context for a patch.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CPatchDLMContext : public CTessNodeList
{
public:

	struct	CVertex
	{
		CVector		Pos;
		CVector		Normal;
	};

public:

	/// DLM info
	/// The position and size of the DLM in the texture, in pixels.
	uint			TextPosX, TextPosY, Width, Height;
	/// Mapping to this rectangle in 0-1 basis
	float			DLMUScale, DLMVScale, DLMUBias, DLMVBias;

	/** Lighting Process: number of light contribution to this patch in last rneder, and in cur Render.
	 *	Modified by CPatch and CLandscape
	 */
	uint			OldPointLightCount;
	uint			CurPointLightCount;


	// TextureFar only info
	// The render Pass of Far0 and Far1.
/*	CRdrPatchId		Pass0, Pass1;
	float			Far0UScale, Far0VScale, Far0UBias, Far0VBias;
	float			Far1UScale, Far1VScale, Far1UBias, Far1VBias;*/


public:

	/// Constructor
	CPatchDLMContext();
	/// Destructor: lightmap is released from _DLMTexture
	~CPatchDLMContext();

	/** generate: link to patch and allocate texture space, compile vertices etc...
	 *	The context is also linked to the list, and removed automaticllay at dtor
	 *	Texture space is filled with black (RAM only)
	 *	\return false if cannot allocate texture space for this patch
	 */
	bool			generate(CPatch *patch, CTextureDLM *textureDLM, CPatchDLMContextList *ctxList);

	/**	Fill texture space with Black: RAM texture is updated.
	 *	NB: full src blackness is cached.
	 */
	void			clearLighting();

	/**	Add a pointLight influence to the lighting: RAM texture is updated.
	 *	NB: full src blackness is reseted.
	 */
	void			addPointLightInfluence(const CPatchDLMPointLight &pl);

	/**	update VRAM texture with RAM texture. Uploaded in 16 bits format.
	 *	NB: full dst blackness is cached.
	 */
	void			compileLighting();

	CPatch			*getPatch() const {return _Patch;}

	// For Bench. Get the size in memory this class use.
	uint			getMemorySize() const;

// *************************
private:

	/// The patch which owns us.
	CPatch							*_Patch;
	/// The DLM texture (only one per landscape)
	CTextureDLM						*_DLMTexture;
	// The ctx list where this context is appened.
	CPatchDLMContextList			*_DLMContextList;

	/// Bezier Patch Array information: Width*Height.
	NLMISC::CObjectVector<CVertex>	_Vertices;

	/// The computed lightmap: Width*Height.
	NLMISC::CObjectVector<CRGBA>	_LightMap;

	/// A clip cluster, for quadTree of clusters.
	struct	CCluster
	{
		// The bounding sphere of the cluster
		NLMISC::CBSphere			BSphere;
		// If cluster not clipped, how many cluster to skip. NB: if NSkips==0, then it is a leaf cluster.
		uint						NSkips;
		// For leaf cluster: logical position of the cluster
		uint16						X, Y;
	};

	/// Bounding Sphere QuadTree (with NSkips paradigm)
	NLMISC::CObjectVector<CCluster>	_Clusters;

	// Tells if all _LightMap[] is all black.
	bool							_IsSrcTextureFullBlack;
	// Tells if all dst texture in _DLMTexture is black.
	bool							_IsDstTextureFullBlack;


private:
	// as it sounds...
	void							modulateSrcWithTileColor();

};


// ***************************************************************************
/// A List of CPatchDLMContext.
class	CPatchDLMContextList : public CTessList<CPatchDLMContext>
{
};



} // NL3D


#endif // NL_PATCHDLM_CONTEXT_H

/* End of patchdlm_context.h */
