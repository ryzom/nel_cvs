/** \file flare_model.cpp
 * <File description>
 *
 * $Id: flare_model.cpp,v 1.4 2001/07/26 17:16:59 vizerie Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "3d/flare_model.h"
#include "3d/flare_shape.h"
#include "3d/driver.h"
#include "3d/material.h"
#include "3d/dru.h"
#include "3d/scene.h"



namespace NL3D {


/*
 * Constructor
 */
CFlareModel::CFlareModel() : _Intensity(0)
{
	setTransparency(true) ;
	setOpacity(false) ;
}

void CFlareModel::registerBasic()
{
	// register the model and his observers
	CMOT::registerModel(FlareModelClassId, TransformShapeId, CFlareModel::creator);	
	CMOT::registerObs(RenderTravId, FlareModelClassId, CFlareRenderObs::creator) ;	
}




void	CFlareRenderObs::traverse(IObs *caller)
{
			
	CRenderTrav			*trav = NLMISC::safe_cast<CRenderTrav *>(Trav);
	CFlareModel			*m    = NLMISC::safe_cast<CFlareModel *>(Model) ;
	IDriver				*drv  = trav->getDriver();

	if (trav->isCurrentPassOpaque()) return ;


	// transform the flare on screen
	
	const CVector		upt = HrcObs->WorldMatrix.getPos() ; // unstransformed pos	
	const CVector	pt = trav->ViewMatrix * upt ;
	if (pt.y <= trav->Near) return ; // flare behind us


	nlassert(m->Shape) ;
	CFlareShape *fs = NLMISC::safe_cast<CFlareShape *>((IShape *) m->Shape) ;

	// compute position on screen
	uint32 width, height ;
	drv->getWindowSize(width, height) ;
	const float middleX = .5f * (trav->Left + trav->Right) ;
	const float middleZ = .5f * (trav->Bottom + trav->Top) ;
	const sint xPos = (width>>1) + (sint) (width * (((trav->Near * pt.x) / pt.y) - middleX) / (trav->Right - trav->Left))  ;
	const sint yPos = (height>>1) - (sint) (height * (((trav->Near * pt.z) / pt.y) - middleZ) / (trav->Top - trav->Bottom))  ;	


	// read z-buffer value at the pos we are
	static std::vector<float> v(1) ;
	NLMISC::CRect rect(xPos, height - yPos, 1, 1) ;
	drv->getZBufferPart(v, rect) ;


	


	// project in screen space
	const float z = (float) (1.0 - (1.0 / pt.y - 1.0 / trav->Far) / (1.0 /trav->Near - 1.0 / trav->Far)) ;
	

	if (!v.size() || z > v[0]) // test against z-buffer
	{
		float p = fs->getPersistence() ;
		if (fs == 0)
		{
			m->_Intensity = 0 ;
			return ;
		}
		else
		{
			m->_Intensity -= 1.f / p * m->_Scene->getEllapsedTime() ;	
			if (m->_Intensity < 0.f) 
			{
				m->_Intensity = 0.f ;
				return ;	// nothing to draw
			}
		}			
	}
	else
	{
		float p = fs->getPersistence() ;
		if (fs == 0)
		{
			m->_Intensity = 1 ;
		}
		else
		{
			m->_Intensity += 1.f / p * m->_Scene->getEllapsedTime() ;	
			if (m->_Intensity > 1.f) m->_Intensity = 1.f ;
		}			
	}

	static CMaterial material ;
	static CVertexBuffer vb ; 
	CRGBA  col ;	
	CRGBA        flareColor = fs->getColor() ; 
	if (!fs->getAttenuable() )
	{
		col.modulateFromui(flareColor, (uint) (255.f * m->_Intensity)) ;
	}
	else
	{
		const float norm = sqrtf((float) (((xPos - (width>>1)) * (xPos - (width>>1)) + (yPos - (height>>1))*(yPos - (height>>1)))))
						   / (float) (width>>1) ;
		if (norm > fs->getAttenuationRange() || fs->getAttenuationRange() == 0.f) return ; // nothing to draw ;

		col.modulateFromui(flareColor, (uint) (255.f * m->_Intensity * (1.f - norm / fs->getAttenuationRange() ))) ;
	}

	material.setColor(col) ;	
	material.setBlend(true) ;
	material.setBlendFunc(CMaterial::one, CMaterial::one) ;
	material.setZWrite(false) ;	
	material.setZFunc(CMaterial::always) ;
	material.setLighting(false) ;	
	material.setDoubleSided(true) ;

	vb.setVertexFormat(IDRV_VF_XYZ | IDRV_VF_UV[0]) ;
	vb.setNumVertices(4) ;

	const CVector I = trav->CamMatrix.getI() ;
	const CVector J = trav->CamMatrix.getJ() ;
	const CVector K = trav->CamMatrix.getK() ;

	CVector scrPos ; // vector that will map to the center of the flare on screen
	


	vb.setTexCoord(0, 0, NLMISC::CUV(1, 0)) ;
	vb.setTexCoord(1, 0, NLMISC::CUV(1, 1)) ;
	vb.setTexCoord(2, 0, NLMISC::CUV(0, 1)) ;
	vb.setTexCoord(3, 0, NLMISC::CUV(0, 0)) ;


	
	drv->setupModelMatrix(CMatrix::Identity) ;
	
	drv->activeVertexBuffer(vb) ;

	// we don't change the fustrum to draw 2d shapes : it is costly, and we need to restore it after the drawing has been done
	// we setup Z to be (near + far) / 2, and setup x and y to get the screen coordinates we want
	const float zPos             = 0.5f * (trav->Near + trav->Far) ; 
	const float zPosDivNear      = zPos / trav->Near ;

	// compute the coeeff so that x = ax * px + bx ; y = ax * py + by
	const float aX = ( (trav->Right - trav->Left) / (float) width) * zPosDivNear ;
	const float bX = - (sint) (width>>1) * aX + middleX * zPosDivNear ;
	const float aY = - ( (trav->Top - trav->Bottom) / (float) height) * zPosDivNear ;
	const float bY = - (sint) (height>>1) * aY - middleZ * zPosDivNear ;
   
	float px = (float) xPos ;
	float py = (float) yPos ;

	// process each flare

	// delta for each new Pos 
	const float iK = 1.f / (MaxFlareNum) ;
	const float dX = fs->getFlareSpacing() * ((width>>1) - px) * iK ;
	const float dY = fs->getFlareSpacing() * ((height>>1) - py) * iK ;

	float size ; // size of the current flare


	uint k ;
	ITexture *tex ;

	if (fs->getFirstFlareKeepSize())
	{
		tex = fs->getTexture(0) ;
		if (tex)
		{
			size = fs->getSize(0) ;

			vb.setVertexCoord(0, upt + size * (I + K) ) ;
			vb.setVertexCoord(1, upt + size * (I - K) ) ;
			vb.setVertexCoord(2, upt + size * (-I - K) ) ;
			vb.setVertexCoord(3, upt + size * (-I + K) ) ;


			material.setTexture(0, tex) ;
			drv->renderQuads(material, 0, 1) ;	

			k = 1 ;
		}
	}
	else
	{
		k = 0 ;
	}

	for (; k < MaxFlareNum ; ++k)
	{
		tex = fs->getTexture(k) ;
		if (tex)
		{
			// compute vector that map to the center of the flare

			scrPos = (aX * px + bX) * I +  zPos * J + (aY * py + bY) * K ; 


			
			size = fs->getSize(k) / trav->Near ;
			

			vb.setVertexCoord(0, scrPos + size * (I + K) ) ;
			vb.setVertexCoord(1, scrPos + size * (I - K) ) ;
			vb.setVertexCoord(2, scrPos + size * (-I - K) ) ;
			vb.setVertexCoord(3, scrPos + size * (-I + K) ) ;


			material.setTexture(0, tex) ;
			drv->renderQuads(material, 0, 1) ;


			px += dX ;
			py += dY ;
		}
	}
	
	
	
	this->traverseSons() ;
}



} // NL3D
