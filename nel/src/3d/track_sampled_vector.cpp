/** \file track_sampled_vector.cpp
 * <File description>
 *
 * $Id: track_sampled_vector.cpp,v 1.1 2002/06/06 08:47:16 berenguier Exp $
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

#include "nel/misc/common.h"
#include "3d/track_sampled_vector.h"

using namespace NLMISC;
using namespace std;


namespace NL3D 
{

// ***************************************************************************
// ***************************************************************************
// CTrackSampledVector
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CTrackSampledVector::CTrackSampledVector()
{
}

// ***************************************************************************
CTrackSampledVector::~CTrackSampledVector()
{
}

// ***************************************************************************
const IAnimatedValue&	CTrackSampledVector::getValue () const
{
	return _Value;
}

// ***************************************************************************
void					CTrackSampledVector::serial(NLMISC::IStream &f)
{
	/*
	Version 0:
		- base version.
	*/
	sint	ver= f.serialVersion(0);

	// serial Time infos.
	CTrackSampledCommon::serialCommon(f);

	// serial Keys.
	f.serial(_Keys);

}

// ***************************************************************************
void	CTrackSampledVector::build(const std::vector<uint16> &timeList, const std::vector<CVector> &keyList, 
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
		// no compression for now
		_Keys[i]= keyList[i];
	}

}

// ***************************************************************************
void	CTrackSampledVector::eval (const TAnimationTime& date)
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
		_Value.Value= _Keys[keyId0];
	}
	// interpolate
	else if( evalType==EvalInterpolate )
	{
		const CVector	&valueKey0= _Keys[keyId0];
		const CVector	&valueKey1= _Keys[keyId1];

		// interpolate
		_Value.Value= valueKey0*(1-interpValue) + valueKey1*interpValue;
	}
	else
	{
		nlstop;
	}

}


} // NL3D
