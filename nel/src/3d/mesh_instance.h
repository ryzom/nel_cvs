/** \file mesh_instance.h
 * <File description>
 *
 * $Id: mesh_instance.h,v 1.1 2001/06/15 16:24:43 corvazier Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#ifndef NL_MESH_INSTANCE_H
#define NL_MESH_INSTANCE_H

#include "nel/misc/types_nl.h"
#include "3d/transform_shape.h"
#include "3d/material.h"
#include "3d/animated_material.h"


namespace NL3D
{


class CMesh;
class CMeshInstanceAnimDetailObs;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		MeshInstanceId=NLMISC::CClassId(0x6bfe0a34, 0x23b26dc9);


// ***************************************************************************
/**
 * An instance of CMesh.
 * NB: no observers are needed, but AnimDetailObs, since same functionnality as CTransformShape.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CMeshInstance : public CTransformShape
{
public:
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();

public:

	/** The list of materials, copied from the mesh.
	 * Each CMeshInstance has those materials, so they can be animated or modified for each instance.
	 * By default, they are copied from the Mesh.
	 */
	std::vector<CMaterial>			Materials;


	/// \name IAnimatable Interface (registering only IAnimatable sons).
	// @{
	enum	TAnimValues
	{
		OwnerBit= CTransformShape::AnimValueLast, 

		AnimValueLast,
	};


	virtual	void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix);

	// @}

	/// \name Derived from ITransformable.
	// @{
	/// Default Track Values.
	virtual ITrack* getDefaultTrack (uint valueId);
	// @}

	// Return true if this mesh is a skin apply on a skeleton else false.
	bool isSkinApply () const
	{
		return _ApplySkinOk;
	}

	/// \name LightMap properties
	// @{
	uint32 getNbLightMap();
	void getLightMapName( uint32 nLightMapNb, std::string &LightMapName );
	void setLightMapFactor( const std::string &LightMapName, CRGBA nFactor );
	// @}


protected:
	/// Constructor
	CMeshInstance()
	{
		IAnimatable::resize(AnimValueLast);
		_ApplySkinOk= false;
	}
	/// Destructor
	virtual ~CMeshInstance() {}


private:
	static IModel	*creator() {return new CMeshInstance;}
	friend	class CMesh;
	friend	class CMeshInstanceAnimDetailObs;
	friend	class CSkeletonModel;


	/** The list of animated materials, instanciated from the mesh.
	 */
	std::vector<CAnimatedMaterial>	_AnimatedMaterials;


	/// Skinning. true if skinned to the _Skeleton.
	bool		_ApplySkinOk;

};


// ***************************************************************************
/**
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - extend the traverse method.
 *
 * \sa CAnimDetailTrav IBaseAnimDetailObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CMeshInstanceAnimDetailObs : public CTransformAnimDetailObs
{
public:

	/** this do :
	 *  - call CTransformAnimDetailObs::traverse() => traverseSons.
	 *  - update animated materials.
	 */
	virtual	void	traverse(IObs *caller)
	{
		CTransformAnimDetailObs::traverse(caller);

		// update animated materials.
		CMeshInstance	*mi= (CMeshInstance*)Model;

		// test if animated materials must be updated.
		if(mi->IAnimatable::isTouched(CMeshInstance::OwnerBit))
		{
			// must test / update all AnimatedMaterials.
			for(uint i=0;i<mi->_AnimatedMaterials.size();i++)
			{
				// This test and update the pointed material.
				mi->_AnimatedMaterials[i].update();
			}

			mi->IAnimatable::clearFlag(CMeshInstance::OwnerBit);
		}
	}


public:
	static IObs	*creator() {return new CMeshInstanceAnimDetailObs;}
};



} // NL3D


#endif // NL_MESH_INSTANCE_H

/* End of mesh_instance.h */
