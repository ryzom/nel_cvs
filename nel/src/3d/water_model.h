/** \file water_model.h
 * A model for water
 *
 * $Id: water_model.h,v 1.13 2003/05/28 12:54:43 vizerie Exp $
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

#ifndef NL_WATER_MODEL_H
#define NL_WATER_MODEL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/polygon.h"
#include "nel/3d/u_water.h"
//
#include "3d/transform_shape.h"
#include "3d/material.h"
#include "3d/vertex_buffer.h"
#include "3d/texture_emboss.h"


namespace MISC
{
	class CVector;
}

namespace NL3D {


class CWaterPoolManager;
class CWaterShape;
class IDriver;

/**
 * A water quad
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CWaterModel : public CTransformShape, public UWaterInstance
{
public:
	/// ctor
	CWaterModel();

	// to call the first time after the shape & the matrix  has been set
	void init()
	{
		updateDiffuseMapMatrix(true);
	}

	// register this model
	static void registerBasic();
	static CTransform *creator() { return new CWaterModel; }

	// get default tracks
	virtual ITrack* getDefaultTrack (uint valueId);

	/// inherited from UWaterInstance	
	virtual uint32	getWaterHeightMapID() const;

	/// inherited from UWaterInstance	
	virtual float	getHeightFactor() const;

	/// inherited from UWaterInstance	
	virtual float   getHeight(const NLMISC::CVector2f &pos);

	/// inherited from UWaterInstance	
	virtual float   getAttenuatedHeight(const NLMISC::CVector2f &pos, const NLMISC::CVector &viewer);

	/// \name CTransform traverse specialisation
	// @{
	virtual void	traverseRender();
	virtual	bool	clip();
	// @}

protected:
	friend class CWaterShape;	
	void setupMaterialNVertexShader(IDriver *drv, CWaterShape *shape, const NLMISC::CVector &obsPos, bool above, float maxDist, float zHeight);
	// compute the clipped poly for cards that have vertex shaders
	void computeClippedPoly();
	// compute the clipped poly for simple shader version
	void computeSimpleClippedPoly();
	// simple rendering version
	void doSimpleRender(IDriver *drv);

private:
	NLMISC::CPolygon _ClippedPoly;
	NLMISC::CPolygon _EndClippedPoly;	
	CSmartPtr<CTextureEmboss> _EmbossTexture;
	// Matrix to compute uv of diffuse map
	NLMISC::CVector2f   _ColorMapMatColumn0, _ColorMapMatColumn1, _ColorMapMatPos;	
	uint64              _MatrixUpdateDate;

	void updateDiffuseMapMatrix(bool force = false);

	// vertex buffer for simple rendering
	static CVertexBuffer _SimpleRenderVB;
	static CMaterial _WaterMat;
	static CMaterial _SimpleWaterMat;
};

//=====================================================================================================================

/// This model can create wave where it is located. It has no display...
class CWaveMakerModel : public CTransformShape
{
	public:
	
	CWaveMakerModel();

	// register this model
	static void		registerBasic();
	
	static CTransform *creator() { return new CWaveMakerModel; }

	// get default tracks
	virtual ITrack* getDefaultTrack (uint valueId);
	
	/// \name CTransform traverse specialisation
	// @{
	/** this do :
	 *  - call CTransformShape::traverseAnimDetail()
	 *  - perform perturbation
	 */
	virtual void	traverseAnimDetail();
	// @}

protected:	

	friend class	CWaveMakerShape;
	TAnimationTime  _Time;
};



} // NL3D


#endif // NL_WATER_MODEL_H

/* End of water_model.h */
