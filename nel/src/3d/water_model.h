/** \file water_model.h
 * A model for water
 *
 * $Id: water_model.h,v 1.8 2003/03/26 10:20:55 berenguier Exp $
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
#include "3d/transform_shape.h"
#include "nel/3d/u_water.h"

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
	// @}

protected:
	friend class CWaterShape;

	void setupMaterialNVertexShader(IDriver *drv, CWaterShape *shape, const NLMISC::CVector &obsPos, bool above, float maxDist, float zHeight);
	///   setup the vertex program to perform the right attenuation
	//void setAttenuationFactor(IDriver *drv, bool reversed, const NLMISC::CVector &obsPos, const NLMISC::CVector &cameraJ, float farDist);
	// disable attenuation with distance
	//void disableAttenuation(IDriver *drv);

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
	virtual void	traverseAnimDetail(CTransform *caller);
	// @}

protected:	

	friend class	CWaveMakerShape;
	TAnimationTime  _Time;
};



} // NL3D


#endif // NL_WATER_MODEL_H

/* End of water_model.h */
