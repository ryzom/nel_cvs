/** \file mesh_base_instance.h
 * <File description>
 *
 * $Id: mesh_base_instance.h,v 1.11 2002/03/29 14:19:55 berenguier Exp $
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

#ifndef NL_MESH_BASE_INSTANCE_H
#define NL_MESH_BASE_INSTANCE_H

#include "nel/misc/types_nl.h"
#include "3d/transform_shape.h"
#include "3d/material.h"
#include "3d/animated_material.h"
#include "3d/animated_lightmap.h"
#include "3d/animated_morph.h"


namespace NL3D
{


class CMeshBase;
class CMesh;
class CMeshMRM;
class CMeshBaseInstanceAnimDetailObs;
class CAnimatedLightmap;

// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		MeshBaseInstanceId=NLMISC::CClassId(0xef44331, 0x739f6bcf);


// ***************************************************************************
/**
 * An base class for instance of CMesh and CMeshMRM  (which derive from CMeshBase).
 * NB: this class is a model but is not designed to be instanciated in CMOT.
 * NB: no observers are needed, but AnimDetailObs, since same functionality as CTransformShape.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CMeshBaseInstance : public CTransformShape
{
public:
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();

public:


	/** The list of materials, copied from the mesh.
	 * Each CMeshBaseInstance has those materials, so they can be animated or modified for each instance.
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

	/// \name LightMap properties
	// @{
	uint32 getNbLightMap();
	void getLightMapName( uint32 nLightMapNb, std::string &LightMapName );
	void setLightMapFactor( const std::string &LightMapName, CRGBA nFactor );
	// @}

	// To build lightmap
	void setAnimatedLightmap (CAnimatedLightmap *alm)
	{
		_AnimatedLightmap.push_back( alm );
	}

	/// \name BlendShape properties
	// @{
	// Interface
	uint32 getNbBlendShape();
	void getBlendShapeName (uint32 nBlendShapeNb, std::string &BlendShapeName );
	void setBlendShapeFactor (const std::string &BlendShapeName, float rFactor);

	// Internal
	std::vector<CAnimatedMorph>* getBlendShapeFactors()
	{
		return &_AnimatedMorphFactor;
	}
	// @}


	// Return true if this mesh is a skin apply on a skeleton else false.
	bool isSkinApply () const
	{
		return _ApplySkinOk;
	}


	/// Lighting behavior. return true if mesh!=NULL and if not all lightMapped.
	virtual bool		isLightable() const;


	/// Select a texture set for this mesh (if available)
	void selectTextureSet(uint index);


	/// Get the scene which owns this instance.
	CScene				*getScene() const {return _OwnerScene;}


	/** Change MRM Distance setup. Only for mesh which support MRM. NB MeshMultiLod apply it only on Lod0.
	 *	NB: This apply to the shape direclty!! ie All instances using same shape will be affected
	 *	NB: no-op if distanceFinest<0, distanceMiddle<=distanceFinest or if distanceCoarsest<=distanceMiddle.
	 *	\param distanceFinest The MRM has its max faces when dist<=distanceFinest.
	 *	\param distanceMiddle The MRM has 50% of its faces at dist==distanceMiddle.
	 *	\param distanceCoarsest The MRM has faces/Divisor (ie near 0) when dist>=distanceCoarsest.
	 */
	virtual void		changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest) {}


protected:
	/// Constructor
	CMeshBaseInstance()
	{
		IAnimatable::resize(AnimValueLast);
		_ApplySkinOk= false;
		_OwnerScene= NULL;
	}
	/// Destructor
	virtual ~CMeshBaseInstance() {}


	/// \name Skinning Behavior.
	// @{
	/// I can be skinned
	virtual	bool			isSkinnable() const {return true;}
	/// Am I skinned ??
	virtual	bool			isSkinned() const {return _ApplySkinOk;}
	/// change my state.
	virtual	void			setApplySkin(bool state) {_ApplySkinOk= state;}
	// @}

private:
	static IModel	*creator() {return new CMeshBaseInstance;}
	friend	class CMeshBase;
	friend	class CMeshBaseInstanceAnimDetailObs;


	/** The list of animated materials, instanciated from the mesh.
	 */
	std::vector<CAnimatedMaterial>	_AnimatedMaterials;

	std::vector<CAnimatedLightmap*> _AnimatedLightmap;

	std::vector<CAnimatedMorph> _AnimatedMorphFactor; 

	/// Skinning. true if skinned to the _Skeleton.
	bool		_ApplySkinOk;

	/// The Scene where the instance is created.
	CScene		*_OwnerScene;

/// public only for IMeshVertexProgram classes.
public:

	/// CMeshVPWindTree instance specific part.
	float		_VPWindTreePhase;		// Phase time of the wind animation. 0-1

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
class	CMeshBaseInstanceAnimDetailObs : public CTransformAnimDetailObs
{
public:

	/** this do :
	 *  - call CTransformAnimDetailObs::traverse()
	 *  - update animated materials.
	 */
	virtual	void	traverse(IObs *caller);


public:
	static IObs	*creator() {return new CMeshBaseInstanceAnimDetailObs;}
};



} // NL3D


#endif // NL_MESH_BASE_INSTANCE_H

/* End of mesh_base_instance.h */
