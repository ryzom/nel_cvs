/** \file flare_shape.cpp
 * <File description>
 *
 * $Id: flare_shape.cpp,v 1.9 2002/05/14 13:03:27 vizerie Exp $
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

#include "std3d.h"

#include "3d/flare_shape.h"
#include "3d/flare_model.h"
#include "3d/scene.h"
#include "3d/driver.h"

namespace NL3D {


/*
 * Constructor
 */
CFlareShape::CFlareShape()  : _Color(NLMISC::CRGBA::White), _Persistence(1), _Spacing(1)
							  ,_Attenuable(false), _AttenuationRange (1.0f), _FirstFlareKeepSize(false)
							  ,_MaxViewDist(1000), _MaxViewDistRatio (0.9f), _InfiniteDist(false)
{
	// init default pos
	for (uint k = 0; k < MaxFlareNum; ++k)
	{
		_Tex [k]  = NULL;
		_Size[k] = 1.f;
		_Pos[k]  = k * (1.f / MaxFlareNum);
	}

	_DefaultPos.setValue(CVector::Null);
}


void CFlareShape::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);
	f.serial(_Color, _Persistence, _Spacing);	
	f.serial(_Attenuable);
	if (_Attenuable)
	{
		f.serial(_AttenuationRange);
	}
	f.serial(_FirstFlareKeepSize);
	for (uint k = 0; k < MaxFlareNum; ++k)
	{
		ITexture *tex = _Tex[k];
		f.serialPolyPtr(tex);
		if (f.isReading())
		{
			_Tex[k] = tex;
		}
		f.serial(_Size[k], _Pos[k]);
	}
	f.serial(_InfiniteDist);
	if (!_InfiniteDist)
	{
		f.serial(_MaxViewDist, _MaxViewDistRatio);
	}
	f.serial(_DazzleEnabled);
	if (_DazzleEnabled)
	{
		f.serial(_DazzleColor, _DazzleAttenuationRange);
	}
	f.serial(_InfiniteDist);
}

	
CTransformShape		*CFlareShape::createInstance(CScene &scene)
{
	CFlareModel *fm = NLMISC::safe_cast<CFlareModel *>(scene.createModel(FlareModelClassId) );
	fm->Shape = this;	
	fm->_Scene = &scene;
	// set default pos
	fm->ITransformable::setPos( ((CAnimatedValueVector&)_DefaultPos.getValue()).Value  );
	return fm;
}

float				CFlareShape::getNumTriangles (float distance)
{
	float count = 0;
	for (uint k = 0; k < MaxFlareNum; ++k)
	{
	if (_Tex[k]) count += 2;
	}
	return count;
}

bool				CFlareShape::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
{		
	// compute flare pos in world basis : 
	const NLMISC::CVector pos = worldMatrix.getPos();
	for (std::vector<NLMISC::CPlane>::const_iterator it = pyramid.begin(); it != pyramid.end(); ++it)
	{
		if ((*it) * pos > 0) return false;
	}
	return true;
}


void				CFlareShape::getAABBox(NLMISC::CAABBox &bbox) const
{
	// the flare himself is a point
	bbox.setCenter(CVector::Null);
	bbox.setHalfSize(CVector::Null);
}


void				CFlareShape::flushTextures (IDriver &driver)
{
	// Flush each texture
	for (uint tex=0; tex<MaxFlareNum; tex++)
	{
		if (_Tex[tex] != NULL)
		{
			/// Flush texture
			driver.setupTexture (*_Tex[tex]);
		}
	}
}


} // NL3D
