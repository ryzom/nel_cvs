/** \file flare_model.cpp
 * <File description>
 *
 * $Id: flare_model.cpp,v 1.2 2001/07/24 10:00:55 vizerie Exp $
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


namespace NL3D {


/*
 * Constructor
 */
CFlareModel::CFlareModel() : _LastViewDate(0), _Intensity(0)
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

	// Setup the matrix.
	drv->setupModelMatrix(HrcObs->WorldMatrix);
		
	

	// transform the flare on screen
	
	const CVector		pt = trav->ViewMatrix * m->getMatrix().getPos() ;
	if (pt.y <= trav->Near) return ;

	uint32 width, height ;
	drv->getWindowSize(width, height) ;

	const float middleX = .5f * trav->Left + trav->Right ;
	const float middleZ = .5f * trav->Bottom + trav->Top ;

	const sint xPos = (width>>1) + (sint) (width * (((trav->Near * pt.x) / pt.y) - middleX) / (trav->Right - trav->Left))  ;
	const sint yPos = (height>>1) - (sint) (height * (((trav->Near * pt.z) / pt.y) - middleZ) / (trav->Right - trav->Left))  ;
	

	static std::vector<float> v(1) ;
	NLMISC::CRect rect(xPos, yPos, 1, 1) ;
	drv->getZBufferPart(v, rect) ;

	const float z = (pt.z - trav->Near)  / (trav->Far - trav->Near) ;
	

	if (z > v[0]) // test against z-buffer
	{
		m->_Intensity -= 0.01f ;
		if (m->_Intensity < 0.f) m->_Intensity =0.f ;
	}
	else
	{
		m->_Intensity += 0.01f ;
		if (m->_Intensity > 1.f) m->_Intensity = 1.f ;
	}

	static CMaterial material ;
	static CVertexBuffer vb ; 
	CRGBA  col ;
	nlassert(m->Shape) ;
	CFlareShape *fs = NLMISC::safe_cast<CFlareShape *>((IShape *) m->Shape) ;
	CRGBA        flareColor = fs->getColor() ; 
	col.modulateFromui(flareColor, (uint) (255.f * m->_Intensity)) ;
	material.setColor(col) ;
	material.setBlend(true) ;
	material.setBlendFunc(CMaterial::one, CMaterial::one) ;
	material.setZWrite(false) ;
	material.setTexture(0, fs->getTexture()) ;
	material.setZFunc(CMaterial::always) ;

	vb.setVertexFormat(IDRV_VF_XYZ | IDRV_VF_UV[0]) ;
	vb.setNumVertices(4) ;

	const CVector I = trav->CamMatrix.getI() ;
	const CVector K = trav->CamMatrix.getK() ;

	const float size = fs->getSize() ;

	vb.setVertexCoord(0, pt + size * (I + K)) ;
	vb.setVertexCoord(1, pt + size * (I - K)) ;
	vb.setVertexCoord(2, pt + size * (-I - K)) ;
	vb.setVertexCoord(3, pt + size * (-I + K)) ;

	vb.setTexCoord(0, 0, NLMISC::CUV(1, 0)) ;
	vb.setTexCoord(1, 0, NLMISC::CUV(1, 1)) ;
	vb.setTexCoord(2, 0, NLMISC::CUV(0, 1)) ;
	vb.setTexCoord(3, 0, NLMISC::CUV(0, 0)) ;


	drv->activeVertexBuffer(vb) ;

	drv->renderQuads(material, 0, 1) ;
	
	this->traverseSons() ;
}



} // NL3D
