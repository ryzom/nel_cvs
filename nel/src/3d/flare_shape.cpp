/** \file flare_shape.cpp
 * <File description>
 *
 * $Id: flare_shape.cpp,v 1.2 2001/07/26 17:16:59 vizerie Exp $
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

#include "3d/flare_shape.h"
#include "3d/flare_model.h"
#include "3d/scene.h"

namespace NL3D {


/*
 * Constructor
 */
CFlareShape::CFlareShape()  : _Color(NLMISC::CRGBA::White), _Persistence(1), _Spacing(1)
							  ,_Attenuable(false), _AttenuationRange (1.0f), _FirstFlareKeepSize(false)
{
	for (uint k = 0 ; k < MaxFlareNum ; ++k)
	{
		_Tex [k]  = NULL ;
		_Size[k] = 1.f ;
	}
}


void CFlareShape::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1) ;
	f.serial(_Color, _Persistence, _Spacing) ;
	f.serial(_Attenuable) ;
	if (_Attenuable)
	{
		f.serial(_AttenuationRange) ;
	}
	f.serial(_FirstFlareKeepSize) ;
	for (uint k = 0 ; k < MaxFlareNum ; ++k)
	{
		ITexture *tex = (ITexture *) _Tex ;
		f.serialPolyPtr(tex) ;
		if (f.isReading())
		{
			_Tex[k] = tex ;
		}
		f.serial(_Size[k]) ;
	}
}

	
CTransformShape		*CFlareShape::createInstance(CScene &scene)
{
	CFlareModel *fm = NLMISC::safe_cast<CFlareModel *>(scene.createModel(FlareModelClassId) ) ;
	fm->Shape = this ;	
	fm->_Scene = &scene ;
	return fm ;
}

float				CFlareShape::getNumTriangles (float distance)
{
	float count ;
	for (uint k = 0 ; k < MaxFlareNum ; ++k)
	{
	if (_Tex[k]) count += 2 ;
	}
	return count ;
}

bool				CFlareShape::clip(const std::vector<CPlane>	&pyramid)
{
	for (std::vector<NLMISC::CPlane>::const_iterator it = pyramid.begin() ; it != pyramid.end() ; ++it)
	{
		if (it->d > _Size[0]) return false ;
	}
	return true ;
}


void				CFlareShape::getAABBox(NLMISC::CAABBox &bbox) const
{
	bbox.setCenter(CVector::Null) ;
	bbox.setHalfSize(CVector::Null) ;
}



} // NL3D
