/** \file target_anim_ctrl.cpp
 * <File description>
 *
 * $Id: target_anim_ctrl.cpp,v 1.1.2.1 2003/07/10 12:49:44 berenguier Exp $
 */

/* Copyright, 2000-2003 Nevrax Ltd.
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
#include "nel/misc/common.h"
#include "nel/3d/target_anim_ctrl.h"
#include "3d/bone.h"
#include "3d/skeleton_model.h"
#include "3d/scene.h"

using namespace std;
using namespace NLMISC;

namespace NL3D {


// ***************************************************************************
CTargetAnimCtrl::CTargetAnimCtrl()
{
	Mode= DirectionMode;
	WorldTarget= CVector::Null;
	EyePos= CVector::Null;
	// Default Direction to "LookBack".
	DefaultWorldDirection.set(0,0,1,0);
	MaxAngle= (float)Pi/3;
	MaxAngularVelocity= (float)(2*Pi);
	Enabled= true;
	_EnableToDisableTransition= false;
	_LastEnabled= true;
}


// ***************************************************************************
CTargetAnimCtrl::~CTargetAnimCtrl()
{
}


// ***************************************************************************
void	CTargetAnimCtrl::execute(CSkeletonModel *model, CBone *bone)
{
	// no op if req not met
	if(!bone || !model || bone->getTransformMode()!=ITransformable::RotQuat)
		return;

	// If the user changed the Enabled state, must do a transition.
	if(_LastEnabled!=Enabled)
	{
		_LastEnabled= Enabled;
		// if re-enable the control while completely disabled before
		if(Enabled && !_EnableToDisableTransition)
		{
			// set the LastLSRotation so that it match the current anim ones.
			_LastLSRotation= getCurrentLSRotationFromBone(model, bone);
		}
		// if disable the ctrl, then do a transition first
		if(!Enabled)
		{
			_EnableToDisableTransition= true;
		}
	}

	// If not enabled, and not in transition, no op.
	if( !Enabled && !_EnableToDisableTransition)
		return;

	// If Target mode, compute CurrentWorldDirection
	// ***********
	// NB: does need to compute direction if disabled (even if in transition)
	if(Mode==TargetMode && Enabled)
	{
		// get the eye pos in world.
		CVector	worldEye= bone->getWorldMatrix()*EyePos;
		// get the world dir
		CVector	worldDir= (WorldTarget - worldEye).normed();
		// get the associated quat
		CMatrix	mat;
		mat.setRot(CVector::I, worldDir, CVector::K);
		mat.normalize(CMatrix::YZX);
		CurrentWorldDirection= mat.getRot();
	}


	// compute rotation to apply In LocalSkeleton Space
	// ***********
	CQuat		currentLSRotation;

	/* Get the Skeleton default WorldMatrix (bind pos). used later
		TRICK: to get the default Skeleton WorldMatrix, get it from Bone[0] (ie "Bip01")
		We cannot use the Default Pos/Rot of the skeleton model because of export bug (not exported...)
		So, since Bip01 as no Local Rot (yes, its true, exporter report all Bip01 rots on Skeleton), 
		we are sure that Bip01 BindPos is the default Skeleton World Matrix.
	*/
	CQuat		rootInvBP= model->Bones[0].getBoneBase().InvBindPos.getRot();

	// If ctrl not enabled, take the LSRotation from the animation
	if(!Enabled)
	{
		currentLSRotation= getCurrentLSRotationFromBone(model, bone);
	}
	else
	{
		// Get the wanted direction in LocalSkeleton Space. 
		CQuat		currentLSDirection;

		// Get the current wanted WorldDirection into LocalSkeleton space (hence using current skeleton WorldMatrix).
		CMatrix		toLSSpace= model->getWorldMatrix();
		currentLSDirection= toLSSpace.getRot().conjugate() * CurrentWorldDirection;

		// Get the default WorldDirection into LocalSkeleton space (hence using default skeleton WorldMatrix).
		CQuat		defaultLSDirection= rootInvBP * DefaultWorldDirection;

		/* get the rotation to apply to the bone when it is in bind Pos. If this quat is identity, 
			then the bone will be in default (or bind) pos.
			It is in essence the "rotation to apply to defaultDirection in LS space, in order to get 
			the wanted current direction in LS space".
			The makeClosest() is here just to ensure that the resulting angle<Pi (for clamp direction later)
		*/
		currentLSDirection.makeClosest(defaultLSDirection);
		currentLSRotation= currentLSDirection * defaultLSDirection.conjugate();
	}


	// Clamp direction, and smooth direction changes.
	// ***********
	// if not enabled, then LSRotation comes from the animation => do not clamp
	if(Enabled)
	{
		// to AngleAxis.
		CAngleAxis	angleAxis= currentLSRotation.getAngleAxis();
		// Clamp the angle
		clamp(angleAxis.Angle, -MaxAngle, MaxAngle);
		// back To Quat.
		currentLSRotation.setAngleAxis(angleAxis);
	}

	// get the dt of the scene.
	CScene	*scene= model->getOwnerScene();
	float	sceneDt= scene->getEllapsedTime();
	float	maxDeltaAngle= MaxAngularVelocity*sceneDt;
	// get the quat that change from LastRotation to CurrentRotation
	CQuat	rotMod= _LastLSRotation.conjugate() * currentLSRotation;
	// compute how many rotation we are allowed to do.
	float	rotModAngle= (float)fabs(rotMod.getAngle());
	bool	rotSpeedClamped= false;
	if(rotModAngle)
	{
		float	factor= (float)fabs(maxDeltaAngle) / rotModAngle;
		// If cannot do all the rotation this frame
		if(factor<1)
		{
			// then slerp between last and current rotation
			currentLSRotation.makeClosest(_LastLSRotation);
			currentLSRotation= CQuat::slerp(_LastLSRotation, currentLSRotation, factor);
			rotSpeedClamped= true;
		}
	}

	// if the rotation has not been clamped for speed consideration, and if !Enabed, it's mean we have ended 
	// the transition => no more compute next time
	if(!Enabled && !rotSpeedClamped)
		_EnableToDisableTransition= false;

	// bkup last rotation
	_LastLSRotation= currentLSRotation;

	// Apply the weighted Rotation to the bone
	// ***********
	// Get the bone Bind Pos in LocalSkeleton space (hence using Default Skeleton WorldMatrix)
	CQuat	boneBindPosInWorld= bone->getBoneBase().InvBindPos.getRot().conjugate();
	CQuat	boneBindPosInLS= rootInvBP * boneBindPosInWorld;
	// rotate it to match our wanted direction.
	boneBindPosInLS= currentLSRotation * boneBindPosInLS;

	// get it in bone local space.
	// get the Bone Parent LocalSkeletonMatrix
	CBone	*boneParent= bone->getFatherId()==-1? NULL : &model->Bones[bone->getFatherId()];
	CQuat	currentLocalQuat;
	if(!boneParent)
		currentLocalQuat= boneBindPosInLS;
	else
	{
		// compute the rotation to apply, in local space
		CQuat		qp= boneParent->getLocalSkeletonMatrix().getRot();
		currentLocalQuat= qp.conjugate() * boneBindPosInLS;
	}

	// set the new LocalRotQuat
	bone->setRotQuat(currentLocalQuat);
	// and recompute the bone (but without AnimCtrl of course :) )
	bone->compute(boneParent, model->getWorldMatrix(), NULL);

}


// ***************************************************************************
CQuat	CTargetAnimCtrl::getCurrentLSRotationFromBone(CSkeletonModel *model, CBone *bone)
{
	// get the current rotation matrix (qmat) of this bone, in LS space
	CQuat	currentLSRot= bone->getLocalSkeletonMatrix().getRot();

	// get the default bindPos (qb) rotation of this bone, in LS space.
	CQuat	boneBindPosInWorld= bone->getBoneBase().InvBindPos.getRot().conjugate();
	CQuat	rootInvBP= model->Bones[0].getBoneBase().InvBindPos.getRot();
	CQuat	boneBindPosInLS= rootInvBP * boneBindPosInWorld;

	// The rotation (qrot) is computed such that qmat= qrot * qb
	currentLSRot.makeClosest(boneBindPosInLS);
	return currentLSRot * boneBindPosInLS.conjugate();
}



} // NL3D
