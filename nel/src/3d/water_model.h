/** \file water_model.h
 * A model for water
 *
 * $Id: water_model.h,v 1.4 2001/11/14 15:38:21 vizerie Exp $
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

namespace MISC
{
	class CVector;
}

namespace NL3D {


class CWaterPoolManager;
class CWaterShape;
class IDriver;
class CScene;

/**
 * A water quad
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CWaterModel : public CTransformShape
{
public:
	/// ctor
	CWaterModel();

	// register this model and his observers
	static void registerBasic();
	static IModel *creator() { return new CWaterModel; }

	// get default tracks
	virtual ITrack* getDefaultTrack (uint valueId);
protected:
	friend class CWaterRenderObs;
	friend class CWaterShape;
	CScene	*_Scene;

};


class	CWaterRenderObs : public CTransformShapeRenderObs
{
public:
	virtual	void	traverse(IObs *caller);	
	static IObs	    *creator() {return new CWaterRenderObs;}
private:
	void setupMaterialNVertexShader(IDriver *drv, CWaterShape *shape, const NLMISC::CVector &obsPos, bool above, float maxDist);
	///   setup the vertex program to perform the right attenuation
	//void setAttenuationFactor(IDriver *drv, bool reversed, const NLMISC::CVector &obsPos, const NLMISC::CVector &cameraJ, float farDist);
	// disable attenuation with distance
	//void disableAttenuation(IDriver *drv);
};



} // NL3D


#endif // NL_WATER_MODEL_H

/* End of water_model.h */
