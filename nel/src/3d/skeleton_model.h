/** \file skeleton_model.h
 * <File description>
 *
 * $Id: skeleton_model.h,v 1.2 2001/06/21 12:57:43 berenguier Exp $
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

#ifndef NL_SKELETON_MODEL_H
#define NL_SKELETON_MODEL_H

#include "nel/misc/types_nl.h"
#include "3d/transform_shape.h"
#include "3d/bone.h"


namespace NL3D
{


class CSkeletonShape;
class CMeshBaseInstance;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		SkeletonModelId=NLMISC::CClassId(0x7d4703b4, 0x43ad6ab1);


// ***************************************************************************
/**
 * A Skeleton model, instance of CSkeletonShape.
 *
 * Skeletons sons are added with bindSkin(), stickObject(). They are removed auto at dtor.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CSkeletonModel : public CTransformShape
{
public:
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();

public:

	/** The list of CBone, created from the CSkeletonShape.
	 * They are odered in depth-first order.
	 */
	std::vector<CBone>			Bones;


public:
	/// \name IAnimatable Interface (registering only IAnimatable sons (bones)).
	// @{
	enum	TAnimValues
	{
		OwnerBit= CTransformShape::AnimValueLast, 

		AnimValueLast,
	};

	// Register bones into chanMixer.
	virtual	void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix);
	// @}



	/// \name Skin operation.
	// @{
	/** bind a MeshInstance skin to the skeleton. NB: ~CTransform() calls detachSkeletonSon().
	 * NB: an object can't be skinned and sticked at same time :)
	 * NB: replaced if already here.
	 * NB: mi is made son of skeleton model in Traversals Hrc and Clip.
	 */
	void		bindSkin(CMeshBaseInstance *mi);
	/** parent a CTransform to a bone of the skeleton. NB: ~CTransform() calls detachSkeletonSon().
	 * NB: an object can't be skinned and sticked at same time :)
	 * NB: replaced if already here.
	 * NB: mi is made son of skeleton model in Traversals Hrc and Clip.
	 */
	void		stickObject(CTransform *mi, uint boneId);
	/** unparent a CTransform from a bone of the skeleton, or unbind a skin. No-op if not here.
	 * NB: mi is made son of Root in Traversals Hrc and Clip.
	 */
	void		detachSkeletonSon(CTransform *mi);
	// @}


	/// \name Misc.
	// @{
	/// return, from skeleton shape, the BoneIdByName. -1 if not here.
	sint32		getBoneIdByName(const std::string &name) const;
	// @}



// ***********************
protected:
	/// Constructor
	CSkeletonModel()
	{
		IAnimatable::resize(AnimValueLast);
		HrcTrav= NULL;
		ClipTrav= NULL;
	}
	/// Destructor
	virtual ~CSkeletonModel();


private:
	static IModel	*creator() {return new CSkeletonModel;}
	friend	class CSkeletonShape;
	friend	class CSkeletonModelAnimDetailObs;


	class	CStickObject
	{
	public:
		CTransform		*Transform;
		uint			BoneId;

		bool	operator<(const CStickObject &o) const
		{
			return Transform<o.Transform;
		}
	};

	/// The skins.
	typedef	std::set<CMeshBaseInstance*>	TMeshInstanceSet;
	typedef	TMeshInstanceSet::iterator		ItMeshInstanceSet;
	TMeshInstanceSet			_Skins;
	/// The StickedObjects.
	typedef	std::set<CStickObject>		TStickObjectSet;
	typedef	TStickObjectSet::iterator	ItStickObjectSet;
	TStickObjectSet				_StickedObjects;


	// The Hrc and Clip traversals of the Scene which owns this Skeleton.
	CHrcTrav		*HrcTrav;
	CClipTrav		*ClipTrav;
	// test if HrcTrav!=NULL, else get from observers (done only one time).
	void			cacheTravs();

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
class	CSkeletonModelAnimDetailObs : public CTransformAnimDetailObs
{
public:

	/** this do :
	 *  - call CTransformAnimDetailObs::traverse() => traverseSons.
	 *  - update animated bones.
	 */
	virtual	void	traverse(IObs *caller)
	{
		CTransformAnimDetailObs::traverse(caller);

		// test if bones must be updated.
		CSkeletonModel	*sm= (CSkeletonModel*)Model;
		if(sm->IAnimatable::isTouched(CSkeletonModel::OwnerBit))
		{
			// Retrieve the WorldMatrix of the current CTransformShape.
			CMatrix		&modelWorldMatrix= HrcObs->WorldMatrix;

			// must test / update the hierarchy of Bones.
			// Since they are orderd in depth-first order, we are sure that parent are computed before sons.
			for(uint i=0;i<sm->Bones.size();i++)
			{
				sint	fatherId= sm->Bones[i].getFatherId();
				// if a root bone...
				if(fatherId==-1)
					// Compute root bone worldMatrix.
					sm->Bones[i].compute( NULL, modelWorldMatrix);
				else
					// Compute bone worldMatrix.
					sm->Bones[i].compute( &sm->Bones[fatherId], modelWorldMatrix);
			}

			sm->IAnimatable::clearFlag(CSkeletonModel::OwnerBit);
		}

		// Sticked Objects: must update their WorldMatrix.
		CSkeletonModel::ItStickObjectSet	it;
		for(it=sm->_StickedObjects.begin(); it!=sm->_StickedObjects.end(); it++)
		{
			CBone	&bone= sm->Bones[it->BoneId];
			it->Transform->updateWorldMatrixFromSkeleton(bone.getWorldMatrix());
		}
	}


public:
	static IObs	*creator() {return new CSkeletonModelAnimDetailObs;}
};


} // NL3D


#endif // NL_SKELETON_MODEL_H

/* End of skeleton_model.h */
