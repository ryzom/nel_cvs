/** \file track_sampled_quat.cpp
 * <File description>
 *
 * $Id: track_sampled_quat.cpp,v 1.3 2002/06/06 08:47:16 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#include "nel/misc/quat.h"
#include "nel/misc/common.h"
#include "3d/track_sampled_quat.h"

using namespace NLMISC;
using namespace std;


namespace NL3D 
{

// ***************************************************************************
// ***************************************************************************
// Quaternion compression
// ***************************************************************************
// ***************************************************************************

const	double	NL3D_OO32767= 1.0f/32767;
const	double	NL3D_OO65535= 1.0f/65535;

#ifdef NL3D_TSQ_ALLOW_QUAT_COMPRESS
// ***************************************************************************
void		CTrackSampledQuat::CQuatPack::pack(const CQuat &quat)
{
	/*
		This is the most precise/faster compression we can have. Some other tries have been made.

		- deducing w from x,y,z is possible with w= 1-sqrt(x²+y²+z²) (with tradeoff of the W sign) 
			but very not precise.
		- Transform the quaternion to an AxisAngle is possible, but slower (some cos/sin or LUT).
			Axis is encoded with sint16, and angle is encoded with uint16.
		- The same than above, but encode the axis as X/Y only, and deduce Z from
			them, is possible but precision problems arise. 
			
		You can see that the operation "deduce a 3/4 member from unit lenght rule" is definetly not precise.

		Hence this simpler but workable way.
	*/

	// normalize the quaterion.
	CQuatD nquat= quat;
	nquat.normalize();

	sint	ax= (sint)floor(nquat.x * 32767 + 0.5);
	sint	ay= (sint)floor(nquat.y * 32767 + 0.5);
	sint	az= (sint)floor(nquat.z * 32767 + 0.5);
	sint	aw= (sint)floor(nquat.w * 32767 + 0.5);
	clamp(ax, -32767, 32767);
	clamp(ay, -32767, 32767);
	clamp(az, -32767, 32767);
	clamp(aw, -32767, 32767);
	x= ax;
	y= ay;
	z= az;
	w= aw;
}

// ***************************************************************************
void		CTrackSampledQuat::CQuatPack::unpack(CQuat &quat)
{
	// unpack x/y/z.
	CQuatD	quatD;
	quatD.x= x * NL3D_OO32767;
	quatD.y= y * NL3D_OO32767;
	quatD.z= z * NL3D_OO32767;
	quatD.w= w * NL3D_OO32767;
	quatD.normalize();

	quat= quatD;
}
#endif


// ***************************************************************************
// ***************************************************************************
// CTrackSampledQuat
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CTrackSampledQuat::CTrackSampledQuat()
{
}

// ***************************************************************************
CTrackSampledQuat::~CTrackSampledQuat()
{
}

// ***************************************************************************
const IAnimatedValue&	CTrackSampledQuat::getValue () const
{
	return _Value;
}

// ***************************************************************************
void					CTrackSampledQuat::serial(NLMISC::IStream &f)
{
	/*
	Version 1:
		- split class with base CTrackSampledCommon (must add a version in it).
	Version 0:
		- base version.
	*/
	sint	ver= f.serialVersion(1);

	if( ver<=0 )
	{
		// serial Time infos, directly in CTrackSampledCommon
		f.serial(_LoopMode);
		f.serial(_BeginTime);
		f.serial(_EndTime) ;
		f.serial(_TotalRange);
		f.serial(_OOTotalRange);
		f.serial(_DeltaTime);
		f.serial(_OODeltaTime);
		f.serial(_TimeBlocks);
	}
	else
	{
		// serial Time infos.
		CTrackSampledCommon::serialCommon(f);
	}

	// serial Keys.
	f.serial(_Keys);

}

// ***************************************************************************
void	CTrackSampledQuat::build(const std::vector<uint16> &timeList, const std::vector<CQuat> &keyList, 
	float beginTime, float endTime)
{
	nlassert( endTime>beginTime || (beginTime==endTime && keyList.size()<=1) );
	nlassert( keyList.size()==timeList.size() );
	uint i;

	// reset.
	uint	numKeys= keyList.size();
	_Keys.clear();
	_TimeBlocks.clear();

	// Build Common time information
	CTrackSampledCommon::buildCommon(timeList, beginTime, endTime);


	// Compute All Key values.
	//===================
	_Keys.resize(numKeys);
	for(i=0; i<numKeys;i++)
	{
		_Keys[i].pack(keyList[i]);
	}

}

// ***************************************************************************
void	CTrackSampledQuat::eval (const TAnimationTime& date)
{
	// Eval time, and get key interpolation info
	uint	keyId0;
	uint	keyId1;
	float	interpValue;
	TEvalType	evalType= evalTime(date, _Keys.size(), keyId0, keyId1, interpValue);

	// Discard? 
	if( evalType==EvalDiscard )
		return;
	// One Key? easy, and quit.
	else if( evalType==EvalKey0 )
	{
		_Keys[keyId0].unpack(_Value.Value);
	}
	// interpolate
	else if( evalType==EvalInterpolate )
	{
		CQuatPack	valueKey0= _Keys[keyId0];
		CQuatPack	valueKey1= _Keys[keyId1];

		// If the 2 keys have same value, just unpack.
		if(valueKey0 == valueKey1)
		{
			valueKey0.unpack(_Value.Value);
		}
		// else interpolate
		else
		{
			// unpack key value.
			CQuat	quat0, quat1;
			valueKey0.unpack(quat0);
			valueKey1.unpack(quat1);

			// interpolate
			_Value.Value= CQuat::slerp(quat0, quat1, interpValue);
		}
	}
	else
	{
		nlstop;
	}

}


} // NL3D
