/** \file mesh_multi_lod_instance.h
 * An instance of CMeshMulitLod
 *
 * $Id: mesh_multi_lod_instance.h,v 1.11 2002/04/29 13:12:10 berenguier Exp $
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

#ifndef NL_MESH_MULTI_LOD_INSTANCE_H
#define NL_MESH_MULTI_LOD_INSTANCE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/class_id.h"

#include "3d/mesh_base_instance.h"


namespace NL3D 
{

// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		MeshMultiLodInstanceId=NLMISC::CClassId(0x1ade6ef8, 0x75c5a84);


// ***************************************************************************
/**
 * An instance of CMeshMulitLod
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CMeshMultiLodInstance : public CMeshBaseInstance
{
public:
	
	/// Ctor
	CMeshMultiLodInstance ();

	/// Dstructor
	~CMeshMultiLodInstance ();

	/** Change MRM Distance setup. See CMeshBaseInstance::changeMRMDistanceSetup()
	 */
	virtual void		changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest);


	/** erase Coarses instances from coarseMesh Manager (if instances exist)
	 */
	void				deleteCoarseInstances();


	enum
	{
		Lod0Blend		=	0x1,
		Coarse0Loaded	=	0x2,
		Coarse1Loaded	=	0x4,
	};

	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();

	/// Last Matrix date for Lods
	uint64			_LastLodMatrixDate[2];

	/// Last Lighting date for Lods
	sint64			_LastLodLightingDate[2];


	// return the contribution of lights (for Coarse Mesh render).
	const CLightContribution	&getLightContribution() {return _LightContribution;}

private:

	/// Computed first lod to display for this distance
	uint	Lod0;
	uint	Lod1;

	/// Active blending on lod 0
	uint	Flags;

	/// Coarse mesh id
	uint64	CoarseMeshId[2];

	/// Computed polygon count for the load balancing result
	float	PolygonCountLod0;
	float	PolygonCountLod1;

	/// Alpha blending to use
	float	BlendFactor;

	static IModel	*creator() {return new CMeshMultiLodInstance;}
	friend	class CMeshMultiLod;
	friend	class CMeshMultiLodBalancingObs;

	/// get average color for Sun lighting. Get result from _LightContribution
	CRGBA			getCoarseMeshLighting();
};


// ***************************************************************************
/**
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - extend clip() method to update CoarseMeshManager
 *
 * \sa CClipTrav CTransformClipObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CMeshMultiLodClipObs : public CTransformShapeClipObs
{
public:

	/// clip the shape, and set renderable.
	virtual	bool	clip(IBaseClipObs *caller);


	static IObs	*creator() {return new CMeshMultiLodClipObs;}
};


// ***************************************************************************
/**
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - implement the traverse method.
 *
 * \sa CHrcTrav IBaseHrcObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CMeshMultiLodBalancingObs : public CTransformShapeLoadBalancingObs
{
public:

	/** this do all the good things:
	 *	- LoadBalancing: get the position of the transform (or the skeleton), and use it as center.
	 *	- traverseSons().
	 */
	virtual	void	traverse(IObs *caller);

	static IObs	*creator() {return new CMeshMultiLodBalancingObs;}


protected:
};


} // NL3D


#endif // NL_MESH_MULTI_LOD_INSTANCE_H

/* End of mesh_multi_lod_instance.h */
