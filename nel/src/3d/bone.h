/** \file bone.h
 * <File description>
 *
 * $Id: bone.h,v 1.5 2002/03/21 16:07:51 berenguier Exp $
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

#ifndef NL_BONE_H
#define NL_BONE_H

#include "nel/misc/types_nl.h"
#include "3d/transformable.h"
#include "nel/misc/smart_ptr.h"


namespace NL3D
{


// ***************************************************************************
/**
 * This is a bone default value. This object is stored in the SkeletonShape, and is serialised.
 * For hierarchy of bones, It contains the id of his father.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CBoneBase : public NLMISC::CRefCount
{
public:
	/// Name of this bone, for Animation access.
	std::string				Name;

	/// The Inverse of bindpos for this bone.
	CMatrix					InvBindPos;

	/// The Father of this bone. -1 means no one.
	sint32					FatherId;

	/// true if unheritScale from father (default==true).
	bool					UnheritScale;

	/// Default tracks.
	CTrackDefaultVector		DefaultPos;
	CTrackDefaultVector		DefaultRotEuler;
	CTrackDefaultQuat		DefaultRotQuat;
	CTrackDefaultVector		DefaultScale;
	CTrackDefaultVector		DefaultPivot;

	/// The distance at which the bone is disabled in the skeleton. If 0, never disable.
	float					LodDisableDistance;

public:

	/// ctor, with default pos as NULL (but scale as 1,1,1).
	CBoneBase();

	/// save/load.
	void			serial(NLMISC::IStream &f);

};


// ***************************************************************************
/**
 * This is a bone, for skeleton animation, with information for result WorldMatrix.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CBone : public ITransformable
{
public:

	/** Constructor. build a bone from a CBoneBase*.
	 * By defautl, a bone is in RotQuat transform mode.
	 * This ctor:
	 *	- store a RefPtr on the bonebase (for getDefaultTracks() method). the refptr is just for nlassert.
	 *	- copy the bonebase default track value into Animated Values Pos/Rot etc....
	 */
	CBone(CBoneBase *boneBase);


	/// retrieve the boneName from BoneBase.
	const std::string	&getBoneName() const {nlassert(_BoneBase); return _BoneBase->Name;}
	/// retrieve the fatherId from BoneBase.
	sint32				getFatherId() const {nlassert(_BoneBase); return _BoneBase->FatherId;}


	/// \name Herited from ITransformable
	// @{
	/// retrive the default trak from skeleton shape.
	virtual ITrack* getDefaultTrack (uint valueId);

	/// register the ITransformable channels as detailled channels.
	virtual	void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix);
	// @}


	/** Compute the LocalSkeletonMatrix, the WorldMatrix, and the BoneSkinMatrix (for skinning).
	 * NB: the result localSkeletonMatrix depends on BoneBase::UnheritScale. \n
	 * NB: the result worldMatrix depends on BoneBase::UnheritScale. \n
	 * NB: the result boneSkinMatrix depends on BoneBase::InvBindPos. \n
	 * \param parent the parent of this bone (maybe NULL if root). his WorldMatrix is used, so it should be computed before.
	 * \param rootMatrix is used as father worldmatrix if parent==NULL. Usefull for computing WorldMatrix.
	 */
	void			compute(CBone *parent, const CMatrix &rootMatrix);

	/** Interpolate the current result of _BoneSkinMatrix fith otherMatrix.
	 *	when interp==0.f, _BoneSkinMatrix= otherMatrix.
	 *	NB: the interpolation is made on per-vector basis => bad matrix interpolation.
	 */
	void			interpolateBoneSkinMatrix(const CMatrix &otherMatrix, float interp);

	/// retrieve the matrix local to the skeleton, computed in compute().
	const CMatrix	&getLocalSkeletonMatrix() const {return _LocalSkeletonMatrix;}

	/// retrieve the WorldMatrix computed in compute().
	const CMatrix	&getWorldMatrix() const {return _WorldMatrix;}

	/// retrieve the BoneSkinMatrix computed in compute().
	const CMatrix	&getBoneSkinMatrix() const {return _BoneSkinMatrix;}


	/// enable the channels (lodEnable) associated to this bone in the channelMixer.
	void			lodEnableChannels(CChannelMixer *chanMixer, bool enable);


// *************************
private:
	// the boneBase of the skeletonShape which create this bone..
	NLMISC::CRefPtr<CBoneBase>	_BoneBase;

	// The result Matrix, local to the skeleton.
	CMatrix						_LocalSkeletonMatrix;
	// The result WorldMatrix.
	CMatrix						_WorldMatrix;
	// The result Disaplcement _LocalSkeletonMatrix, local to the skeleton.
	CMatrix						_BoneSkinMatrix;

	// The bkuped channelIds for each channel of the bone. -1 if not registered (or no tracks in animationSet).
	sint						_PosChannelId;
	sint						_RotEulerChannelId;
	sint						_RotQuatChannelId;
	sint						_ScaleChannelId;
	sint						_PivotChannelId;
};


} // NL3D


#endif // NL_BONE_H

/* End of bone.h */
