/** \file shadow_map.h
 * <File description>
 *
 * $Id: shadow_map.h,v 1.1 2003/08/07 08:49:13 berenguier Exp $
 */

/* Copyright, 2000-2003 Nevrax Ltd.
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

#ifndef NL_SHADOW_MAP_H
#define NL_SHADOW_MAP_H

#include "nel/misc/types_nl.h"
#include "3d/texture.h"
#include "3d/vertex_buffer.h"
#include "3d/vertex_buffer_hard.h"
#include "nel/misc/aabbox.h"


namespace NL3D 
{


using	NLMISC::CPlane;
using	NLMISC::CRefPtr;
using	NLMISC::CAABBox;


// ***************************************************************************
/**
 * This class encapsulate all Data generated by a Shadow Caster, and read for a Shadow Receiver.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CShadowMap
{
public:
	/** Computed at shadow casting time. The matrix used to cast the Shadow.
	 *	It is actualy a World ProjectionMatrix, but is only local to the Position of the Model (not rotation/scale).
	 *	Hence receiver have to add the position of the caster model to this matrix to get true World.
	 *	The usage of this matrix is for UV projection: XYZ= WorldProjectionMatrix * UVW.
	 *	NB: Vj (ie for W) is mapped such that Vp means NearClip of the shadow and Vp+Vj means FarClip of the shadow
	 */
	CMatrix					LocalProjectionMatrix;

	/** Computed at shadow casting time. They are clipping planes used to clip receivers (mirror of the OBB).
	 *	Receivers may use them to clip sub received parts (as they which)
	 *	Like the ProjectionLocalMatrix, this plane are in World, but the position of the Caster model.
	 *	\see generateClipInfoFromMatrix()
	 */
	std::vector<CPlane>		LocalClipPlanes;

	/** Computed at shadow casting time. This is the LocalPos Bouding Box containing the shadow (AxisAligned).
	 *	\see generateClipInfoFromMatrix()
	 */
	CAABBox					LocalBoundingBox;


	// Filled by ShadowMapManager. This is the Last Frame Id we had update the texture.
	uint64					LastGenerationFrame;

public:

	/// Constructor
	CShadowMap();
	~CShadowMap();

	/// create the Texture. It reset the texture if not of same size.
	void			initTexture(uint textSize);
	/// reset the Texture
	void			resetTexture();
	/// get the TextureSize
	uint32			getTextureSize() const {return _TextSize;}

	/// You can only get the texture for filling / use in a material.
	ITexture		*getTexture() const {return _Texture;}


	/// /name Tools
	// @{
	/** From A BBox in Object Space, the lightDir, and the (nearly) worldMatrix, build the Camera for common Render Projection
	 *	The caller has then just to do
	 *		driver->setFrustum(0,1,0,1,0,1,false);
	 *		driver->setupViewMatrix(cameraMatrix.inverted());
	 *		driver->setupModelMatrix(localPosMatrix);
	 *	Then render his mesh.
	 */
	void			buildCasterCameraMatrix(const CVector &lightDir, const CMatrix &localPosMatrix, const CAABBox &bbShape, CMatrix &cameraMatrix);

	/** From the Camera matrix computed with buildCasterCameraMatrix, compute the LocalProjectionMatrix, which modify the
	 *	J axis according to backPoint and Shadow Depth.
	 *	NB: automatically calls the buildClipInfoFromMatrix() method
	 */
	void			buildProjectionInfos(const CMatrix &cameraMatrix, const CVector &backPoint, const CScene *scene);

	/** The ShadowMap Caster can call this method after setting LocalProjectionMatrix. It computes auto the
	 *	LocalClipPlanes and LocalBoundingBox from it. NB: don't use it if you use buildProjectionInfos().
	 */
	void			buildClipInfoFromMatrix();
	// @}

private:
	NLMISC::CSmartPtr<ITexture>		_Texture;
	uint32							_TextSize;

};


} // NL3D


#endif // NL_SHADOW_MAP_H

/* End of shadow_map.h */