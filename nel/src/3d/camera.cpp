/** \file camera.cpp
 * <File description>
 *
 * $Id: camera.cpp,v 1.13 2002/02/28 12:59:49 besson Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#include "3d/camera.h"


namespace	NL3D
{


// ***************************************************************************
void	CCamera::registerBasic()
{
	CMOT::registerModel(CameraId, TransformId, CCamera::creator);
}


// ***************************************************************************
CCamera::CCamera()
{
	setFrustum(1.0f, 1.0f, 0.01f, 1.0f);

	// IAnimatable.
	IAnimatable::resize(AnimValueLast);

	_FovAnimationEnabled= false;
	_TargetAnimationEnabled= false;
	_FovAnimationAspectRatio= 4.0f/3.0f;

	// Default Anims.
	_Fov.Value= (float)NLMISC::Pi/2;
	_Target.Value= CVector::Null;
	_Roll.Value= 0;
}
// ***************************************************************************
void		CCamera::setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective)
{
	_Frustum.init( left, right,bottom, top, znear, zfar, perspective);
}
// ***************************************************************************
void		CCamera::setFrustum(float width, float height, float znear, float zfar, bool perspective)
{
	_Frustum.init(width, height, znear, zfar, perspective);
}
// ***************************************************************************
void		CCamera::setPerspective(float fov, float aspectRatio, float znear, float zfar)
{
	_Frustum.initPerspective(fov, aspectRatio, znear, zfar);
}
// ***************************************************************************
void		CCamera::getFrustum(float &left, float &right, float &bottom, float &top, float &znear, float &zfar) const
{
	left= _Frustum.Left;
	right= _Frustum.Right;
	bottom=	_Frustum.Bottom;
	top= _Frustum.Top;
	znear= _Frustum.Near;
	zfar= _Frustum.Far;
}
// ***************************************************************************
bool		CCamera::isOrtho() const
{
	return !_Frustum.Perspective;
}
// ***************************************************************************
bool		CCamera::isPerspective() const
{
	return _Frustum.Perspective;
}


// ***************************************************************************
// ***************************************************************************
// Anims.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
IAnimatedValue* CCamera::getValue (uint valueId)
{
	// what value ?
	switch (valueId)
	{
	case FovValue:			return &_Fov;
	case TargetValue:		return &_Target;
	case RollValue:			return &_Roll;
	}

	return CTransform::getValue(valueId);
}
// ***************************************************************************
const char *CCamera::getValueName (uint valueId) const
{
	// what value ?
	switch (valueId)
	{
	case FovValue:			return getFovValueName();
	case TargetValue:		return getTargetValueName();
	case RollValue:			return getRollValueName();
	}

	return CTransform::getValueName(valueId);
}

// ***************************************************************************
CTrackDefaultFloat		CCamera::DefaultFov( (float)NLMISC::Pi/2 );
CTrackDefaultVector		CCamera::DefaultTarget( CVector::Null );
CTrackDefaultFloat		CCamera::DefaultRoll( 0 );


ITrack* CCamera::getDefaultTrack (uint valueId)
{
	// what value ?
	switch (valueId)
	{
	case FovValue:			return &DefaultFov;
	case TargetValue:		return &DefaultTarget;
	case RollValue:			return &DefaultRoll;
	}

	return CTransform::getDefaultTrack(valueId);
}
// ***************************************************************************
void	CCamera::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix)
{
	// For CCamera, channels are not detailled.
	addValue(chanMixer, FovValue, OwnerBit, prefix, false);
	addValue(chanMixer, TargetValue, OwnerBit, prefix, false);
	addValue(chanMixer, RollValue, OwnerBit, prefix, false);

	CTransform::registerToChannelMixer(chanMixer, prefix);
}



// ***************************************************************************
void	CCamera::update()
{
	CTransform::update();
	
	// test animations
	if(IAnimatable::isTouched(OwnerBit))
	{
		// FOV.
		if( _FovAnimationEnabled && IAnimatable::isTouched(FovValue))
		{
			// keep the same near/far.
			setPerspective(_Fov.Value, _FovAnimationAspectRatio, _Frustum.Near, _Frustum.Far);
		}

		// Target / Roll.
		// If target/Roll is animated, compute our own quaternion.
		if( _TargetAnimationEnabled && (IAnimatable::isTouched(TargetValue) || IAnimatable::isTouched(RollValue)) )
		{
			CQuat	q0, q1;

			// compute rotation of target.
			CMatrix	mat;
			mat.setRot(CVector::I, _Target.Value, CVector::K);
			mat.normalize(CMatrix::YZX);
			q0= mat.getRot();

			// compute roll rotation.
			q1.setAngleAxis(CVector::J, _Roll.Value);


			// combine and set rotquat!!
			setRotQuat(q0*q1);
		}


		IAnimatable::clearFlag(FovValue);
		IAnimatable::clearFlag(TargetValue);
		IAnimatable::clearFlag(RollValue);

		// We are OK!
		IAnimatable::clearFlag(OwnerBit);
	}
}


}

