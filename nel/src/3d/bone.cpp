/** \file bone.cpp
 * <File description>
 *
 * $Id: bone.cpp,v 1.9 2002/03/21 16:07:51 berenguier Exp $
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

#include "std3d.h"

#include "3d/bone.h"


namespace NL3D
{


// ***************************************************************************
// ***************************************************************************
// CBoneBase
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CBoneBase::CBoneBase() : DefaultPos(CVector(0,0,0)), DefaultRotEuler(CVector(0,0,0)), 
	DefaultScale(CVector(1,1,1)), DefaultPivot(CVector(0,0,0))
{
	FatherId= -1;
	UnheritScale= true;
	// Default: never disable.
	LodDisableDistance= 0.f;
}


// ***************************************************************************
void			CBoneBase::serial(NLMISC::IStream &f)
{
	/*
	Version 1:
		- LodDisableDistance
	*/
	sint	ver= f.serialVersion(1);

	f.serial(Name);
	f.serial(InvBindPos);
	f.serial(FatherId);
	f.serial(UnheritScale);

	if(ver>=1)
		f.serial(LodDisableDistance);
	else
	{
		// Default: never disable.
		LodDisableDistance= 0.f;
	}

	f.serial(DefaultPos);
	f.serial(DefaultRotEuler);
	f.serial(DefaultRotQuat);
	f.serial(DefaultScale);
	f.serial(DefaultPivot);
}


// ***************************************************************************
// ***************************************************************************
// CBone
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CBone::CBone(CBoneBase *boneBase)
{
	nlassert(boneBase);
	_BoneBase= boneBase;

	// IAnimatable.
	IAnimatable::resize(AnimValueLast);

	ITransformable::setTransformMode(ITransformable::RotQuat);
	ITransformable::setPos( ((CAnimatedValueVector&)_BoneBase->DefaultPos.getValue()).Value  );
	ITransformable::setRotQuat( ((CAnimatedValueQuat&)_BoneBase->DefaultRotQuat.getValue()).Value  );
	ITransformable::setScale( ((CAnimatedValueVector&)_BoneBase->DefaultScale.getValue()).Value  );
	ITransformable::setPivot( ((CAnimatedValueVector&)_BoneBase->DefaultPivot.getValue()).Value  );

	// By default, the bone is not binded to a channelMixer.
	_PosChannelId= -1;
	_RotEulerChannelId= -1;
	_RotQuatChannelId= -1;
	_ScaleChannelId= -1;
	_PivotChannelId= -1;
}

// ***************************************************************************
ITrack* CBone::getDefaultTrack (uint valueId)
{
	nlassert(_BoneBase);
	
	// what value ?
	switch (valueId)
	{
	case PosValue:			return &_BoneBase->DefaultPos;
	case RotEulerValue:		return &_BoneBase->DefaultRotEuler;
	case RotQuatValue:		return &_BoneBase->DefaultRotQuat;
	case ScaleValue:		return &_BoneBase->DefaultScale;
	case PivotValue:		return &_BoneBase->DefaultPivot;
	}

	// No, only ITrnasformable values!
	nlstop;
	// Deriver note: else call BaseClass::getDefaultTrack(valueId);

	return NULL;
}

// ***************************************************************************
void	CBone::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix)
{
	// For CBone, channels are detailled.
	// Bkup each channelId (for disable).
	_PosChannelId= addValue(chanMixer, PosValue, OwnerBit, prefix, true);
	_RotEulerChannelId= addValue(chanMixer, RotEulerValue, OwnerBit, prefix, true);
	_RotQuatChannelId= addValue(chanMixer, RotQuatValue, OwnerBit, prefix, true);
	_ScaleChannelId= addValue(chanMixer, ScaleValue, OwnerBit, prefix, true);
	_PivotChannelId= addValue(chanMixer, PivotValue, OwnerBit, prefix, true);

	// Deriver note: if necessary, call	BaseClass::registerToChannelMixer(chanMixer, prefix);
}

// ***************************************************************************
void	CBone::compute(CBone *parent, const CMatrix &rootMatrix)
{
	nlassert(_BoneBase);

	// Compute LocalSkeletonMatrix.
	// Root case?
	if(!parent)
	{
		_LocalSkeletonMatrix= getMatrix();
	}
	// Else, son case, take world matrix from parent.
	else
	{
		// UnheritScale case.
		if(_BoneBase->UnheritScale)
		{
			CMatrix		invScaleComp;
			CVector		fatherScale;
			CVector		trans;

			// retrieve our translation
			if( getTransformMode()==ITransformable::DirectMatrix )
				getMatrix().getPos(trans);
			else
				getPos(trans);
			// retrieve scale from our father.
			parent->getScale(fatherScale);
			// inverse this scale.
			fatherScale.x= 1.0f / fatherScale.x;
			fatherScale.y= 1.0f / fatherScale.y;
			fatherScale.z= 1.0f / fatherScale.z;


			// Compute InverseScale compensation:
			// with UnheritScale, formula per bone should be  T*Sf-1*P*R*S*P-1.
			// But getMatrix() return T*P*R*S*P-1.
			// So we must compute T*Sf-1*T-1, in order to get wanted result.
			invScaleComp.translate(trans);
			invScaleComp.scale(fatherScale);
			invScaleComp.translate(-trans);

			// And finally, we got ParentWM * T*Sf-1*P*R*S*P-1.
			_LocalSkeletonMatrix= parent->_LocalSkeletonMatrix * invScaleComp * getMatrix();
		}
		// Normal case.
		else
			_LocalSkeletonMatrix= parent->_LocalSkeletonMatrix * getMatrix();
	}

	// Compute WorldMatrix.
	_WorldMatrix= rootMatrix * _LocalSkeletonMatrix;

	// Compute BoneSkinMatrix.
	_BoneSkinMatrix= _LocalSkeletonMatrix * _BoneBase->InvBindPos;
}


// ***************************************************************************
void	CBone::interpolateBoneSkinMatrix(const CMatrix &otherMatrix, float interp)
{
	CMatrix		&curMatrix= _BoneSkinMatrix;

	// interpolate rot/scale. Just interpolate basis vectors
	CVector		fatherI= otherMatrix.getI();
	CVector		curI= curMatrix.getI();
	curI= fatherI*(1-interp) + curI*interp;
	CVector		fatherJ= otherMatrix.getJ();
	CVector		curJ= curMatrix.getJ();
	curJ= fatherJ*(1-interp) + curJ*interp;
	CVector		fatherK= otherMatrix.getK();
	CVector		curK= curMatrix.getK();
	curK= fatherK*(1-interp) + curK*interp;
	// replace rotation
	curMatrix.setRot(curI, curJ, curK);

	// interpolate pos
	CVector		fatherPos= otherMatrix.getPos();
	CVector		curPos= curMatrix.getPos();
	curPos= fatherPos*(1-interp) + curPos*interp;
	curMatrix.setPos(curPos);
}


// ***************************************************************************
void	CBone::lodEnableChannels(CChannelMixer *chanMixer, bool enable)
{
	nlassert(chanMixer);

	// Lod Enable channels if they are correclty registered to the channelMixer.
	if( _PosChannelId>=0 )
		chanMixer->lodEnableChannel(_PosChannelId, enable);
	if( _RotEulerChannelId>=0 )
		chanMixer->lodEnableChannel(_RotEulerChannelId, enable);
	if( _RotQuatChannelId>=0 )
		chanMixer->lodEnableChannel(_RotQuatChannelId, enable);
	if( _ScaleChannelId>=0 )
		chanMixer->lodEnableChannel(_ScaleChannelId, enable);
	if( _PivotChannelId>=0 )
		chanMixer->lodEnableChannel(_PivotChannelId, enable);

}


} // NL3D
