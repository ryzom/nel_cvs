/** \file shadow_map.cpp
 * <File description>
 *
 * $Id: shadow_map.cpp,v 1.1 2003/08/07 08:49:13 berenguier Exp $
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

#include "std3d.h"

#include "3d/shadow_map.h"
#include "3d/texture_mem.h"
#include "nel/misc/common.h"
#include "3d/driver.h"
#include "3d/scene.h"


using namespace std;
using namespace NLMISC;

namespace NL3D {


// ***************************************************************************
CShadowMap::CShadowMap()
{
	_TextSize= 0;
	LastGenerationFrame= 0;
}


// ***************************************************************************
CShadowMap::~CShadowMap()
{
	resetTexture();
}

// ***************************************************************************
void			CShadowMap::initTexture(uint textSize)
{
	textSize= max(textSize, 2U);
	// if same size than setup, quit
	if(_TextSize==textSize)
		return;
	resetTexture();
	textSize= raiseToNextPowerOf2(textSize);
	_TextSize= textSize;
	// NB: the format must be RGBA; else slow copyFrameBufferToTexture()
	uint8	*tmpMem= new uint8[4*_TextSize*_TextSize];
	_Texture = new CTextureMem (tmpMem, 4*_TextSize*_TextSize, true, false, _TextSize, _TextSize);
	_Texture->setWrapS (ITexture::Clamp);
	_Texture->setWrapT (ITexture::Clamp);
	_Texture->setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);
	_Texture->generate();
	_Texture->setReleasable (false);
}


// ***************************************************************************
void			CShadowMap::resetTexture()
{
	// release the SmartPtr
	_Texture= NULL;
	_TextSize= 0;
}


// ***************************************************************************
void			CShadowMap::buildClipInfoFromMatrix()
{
	const	CMatrix	&cameraMatrix= LocalProjectionMatrix;
	CVector	vi= LocalProjectionMatrix.getI();
	CVector	vj= LocalProjectionMatrix.getJ();
	CVector	vk= LocalProjectionMatrix.getK();
	CVector	vp= LocalProjectionMatrix.getPos();

	// **** Compute the planes.
	LocalClipPlanes.resize(6);
	// The plane 0 is the back plane.
	LocalClipPlanes[0].make(-vj, vp);
	// Left and botom plane.
	LocalClipPlanes[1].make(-vi, vp);
	LocalClipPlanes[2].make(-vk, vp);
	// Right and top plane.
	LocalClipPlanes[3].make(vi, vp + vi);
	LocalClipPlanes[4].make(vk, vp + vk);
	// Front plane.
	LocalClipPlanes[5].make(vj, vp + vj);

	// **** Compute the AA bounding box.
	LocalBoundingBox.setHalfSize(CVector::Null);
	CVector		p0= vp;
	CVector		p1= p0 + vi + vj + vk;
	LocalBoundingBox.setCenter(p0);
	LocalBoundingBox.extend(p0 + vi);
	LocalBoundingBox.extend(p0 + vj);
	LocalBoundingBox.extend(p0 + vk);
	LocalBoundingBox.extend(p1 - vi);
	LocalBoundingBox.extend(p1 - vj);
	LocalBoundingBox.extend(p1 - vk);
	LocalBoundingBox.extend(p1);
}


// ***************************************************************************
void			CShadowMap::buildCasterCameraMatrix(const CVector &lightDir, const CMatrix &localPosMatrix, const CAABBox &bbShape, CMatrix &cameraMatrix)
{
	// compute the orthogonal LightSpace camera matrix. Remind that J look forward and K is up here.
	cameraMatrix.setRot(CVector::I, lightDir, CVector::K);
	cameraMatrix.normalize(CMatrix::YZX);

	CAABBox		bbLocal;
	bbLocal= CAABBox::transformAABBox(cameraMatrix.inverted() * localPosMatrix, bbShape);

	// Enlarge for 1 pixel left and right.
	float	scaleSize= (float)getTextureSize();
	scaleSize= (scaleSize+2)/scaleSize;
	CVector		hs= bbLocal.getHalfSize();
	hs.x*= scaleSize;
	hs.z*= scaleSize;
	// TestUniform
	/*float	maxLen= max(hs.x, hs.y);
	maxLen= max(maxLen, hs.z);
	hs.x= hs.y= hs.z= maxLen;*/
	// EndTestUniform
	bbLocal.setHalfSize(hs);

	// setup the orthogonal camera Matrix so that it includes all the BBox
	cameraMatrix.translate(bbLocal.getMin());
	CVector	vi= cameraMatrix.getI() * bbLocal.getSize().x;
	CVector	vj= cameraMatrix.getJ() * bbLocal.getSize().y;
	CVector	vk= cameraMatrix.getK() * bbLocal.getSize().z;
	cameraMatrix.setRot(vi,vj,vk);
}


// ***************************************************************************
void			CShadowMap::buildProjectionInfos(const CMatrix &cameraMatrix, const CVector &backPoint, const CScene *scene)
{
	// Modify the cameraMatrix to define the Aera of Shadow.
	CVector		projp= cameraMatrix.getPos();
	CVector		proji= cameraMatrix.getI();
	CVector		projj= cameraMatrix.getJ();
	CVector		projk= cameraMatrix.getK();
	// modify the J vector so that it gets the Wanted Len
	CVector	vj= projj.normed();
	projj= vj*scene->getShadowMapMaxDepth();
	// Must move Pos so that the IK plane include the backPoint
	projp+= (backPoint*vj-projp*vj) * vj;
	// set the matrix
	LocalProjectionMatrix.setRot(proji, projj, projk);
	LocalProjectionMatrix.setPos(projp);

	// compute The clipPlanes and bbox.
	buildClipInfoFromMatrix();
}

} // NL3D
