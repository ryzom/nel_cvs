/** \file animation_optimizer.cpp
 * <File description>
 *
 * $Id: animation_optimizer.cpp,v 1.1 2002/05/30 14:24:50 berenguier Exp $
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

#include "3d/animation_optimizer.h"
#include "nel/misc/mem_stream.h"
#include "3d/track.h"
#include "3d/track_keyframer.h"
#include "3d/animation.h"
#include "3d/track_sampled_quat.h"


using	namespace NLMISC;
using	namespace std;


namespace NL3D 
{


// ***************************************************************************
CAnimationOptimizer::CAnimationOptimizer()
{
	_SampleFrameRate= 30;
	_QuaternionThresholdLowPrec= 1.0 - 0.0001;
	_QuaternionThresholdHighPrec= 1.0 - 0.000001;
}


// ***************************************************************************
void		CAnimationOptimizer::setQuaternionThreshold(double lowPrecThre, double highPrecThre)
{
	nlassert(lowPrecThre>=0);
	nlassert(highPrecThre>=0);
	_QuaternionThresholdLowPrec= 1.0 - lowPrecThre;
	_QuaternionThresholdHighPrec= 1.0 - highPrecThre;
}


// ***************************************************************************
void		CAnimationOptimizer::setSampleFrameRate(float frameRate)
{
	nlassert(frameRate>0);
	_SampleFrameRate= frameRate;
}


// ***************************************************************************
void		CAnimationOptimizer::optimize(const CAnimation &animIn, CAnimation &animOut)
{
	// reset animOut
	contReset(animOut);

	// Parse all tracks of the animation.
	set<string>		setString;
	animIn.getTrackNames (setString);
	set<string>::iterator	it;
	for(it=setString.begin();it!=setString.end();it++)
	{
		const string	&trackName= *it;
		uint	trackId= animIn.getIdTrackByName(trackName);
		nlassert(trackId!=CAnimation::NotFound);
		const ITrack	*track= animIn.getTrack(trackId);

		// If the track is optimisable.
		ITrack	*newTrack;
		if(isTrackOptimisable(track))
		{
			// choose the threshold according to precision wanted
			if( isLowPrecisionTrack(trackName) )
				_QuaternionThreshold= _QuaternionThresholdLowPrec;
			else
				_QuaternionThreshold= _QuaternionThresholdHighPrec;

			// optimize it.
			newTrack= optimizeTrack(track);
		}
		else
		{
			// just clone it.
			newTrack= cloneTrack(track);
		}

		// Add it to the animation
		animOut.addTrack(trackName, newTrack);
	}


}

// ***************************************************************************
ITrack		*CAnimationOptimizer::cloneTrack(const ITrack	*trackIn)
{
	CMemStream	memStream;

	// write to the stream.
	ITrack	*trackInSerial= const_cast<ITrack*>(trackIn);
	memStream.serialPolyPtr(trackInSerial);

	// read from the stream.
	memStream.invert();
	ITrack	*ret= NULL;
	memStream.serialPolyPtr(ret);

	return ret;
}


// ***************************************************************************
bool		CAnimationOptimizer::isTrackOptimisable(const ITrack	*trackIn)
{
	nlassert(trackIn);

	// If the track is a Linear, Bezier or a TCB track, suppose we can optimize it. Constant may not be interressant....
	if(	dynamic_cast<const CTrackKeyFramerTCBQuat*>(trackIn) || 
		dynamic_cast<const CTrackKeyFramerBezierQuat*>(trackIn) ||
		dynamic_cast<const CTrackKeyFramerLinearQuat*>(trackIn) )
		return true;

	return false;
}


// ***************************************************************************
ITrack		*CAnimationOptimizer::optimizeTrack(const ITrack	*trackIn)
{
	// Must be a quaternion track for now.
	nlassert( dynamic_cast<const CAnimatedValueQuat *>(&trackIn->getValue()) );

	// Get track param.
	float beginTime= trackIn->getBeginTime();
	float endTime= trackIn->getEndTime();
	nlassert(endTime>=beginTime);

	// Get num Sample 
	uint	numSamples= (uint)ceil( (endTime-beginTime)*_SampleFrameRate);
	numSamples= max(1U, numSamples);
	nlassert(numSamples<65535);

	// sample the animation. Store result in _TimeList/_KeyList
	sampleQuatTrack(trackIn, beginTime, endTime, numSamples);

	// check if the sampled track can be reduced to a TrackDefaultQuat. Test _KeyList.
	if( testConstantQuatTrack() )
	{
		// create a default Track Quat.
		CTrackDefaultQuat	*trackDefault= new CTrackDefaultQuat;
		// setup the uniform value.
		trackDefault->setValue(_KeyList[0]);

		// return the result.
		return trackDefault;
	}
	// else optimize the sampled animation, and build.
	else
	{
		// optimize.
		optimizeQuatTrack();

		// Create a sampled quaternion track
		CTrackSampledQuat	*trackSQ= new CTrackSampledQuat;

		// Copy loop from track.
		trackSQ->setLoopMode(trackIn->getLoopMode());

		// Build it.
		trackSQ->build(_TimeList, _KeyList, beginTime, endTime);

		// return result.
		return trackSQ;
	}
}


// ***************************************************************************
void		CAnimationOptimizer::sampleQuatTrack(const ITrack *trackIn, float beginTime, float endTime, uint numSamples)
{
	// resize tmp samples
	_TimeList.resize(numSamples);
	_KeyList.resize(numSamples);

	// Sample the animation.
	float	t= beginTime;
	float	dt= 0;
	if(numSamples>1)
		dt= (endTime-beginTime)/(numSamples-1);
	for(uint i=0;i<numSamples; i++, t+=dt)
	{
		CQuat	quat;

		// make exact endTime match (avoid precision problem)
		if(i==numSamples-1)
			t= endTime;

		// evaluate the track
		const_cast<ITrack*>(trackIn)->interpolate(t, quat);

		// normalize this quaternion
		quat.normalize();

		// force on same hemisphere according to precedent frame.
		if(i>0)
		{
			quat.makeClosest(_KeyList[i-1]);
		}

		// store time and key.
		_TimeList[i]= i;
		_KeyList[i]= quat;
	}

}

// ***************************************************************************
bool		CAnimationOptimizer::testConstantQuatTrack()
{
	uint	numSamples= _KeyList.size();
	nlassert(numSamples>0);

	// Get the first sample as the reference quaternion, and test others from this one.
	CQuat	quatRef= _KeyList[0];
	for(uint i=0;i<numSamples;i++)
	{
		// All values must be nearly equal to the reference quaternion.
		if(!nearlySameQuaternion(quatRef, _KeyList[i]))
			return false;
	}

	// ok.
	return true;
}


// ***************************************************************************
void		CAnimationOptimizer::optimizeQuatTrack()
{
	uint	numSamples= _KeyList.size();
	nlassert(numSamples>0);

	// <=2 key? => no opt possible..
	if(numSamples<=2)
		return;

	// prepare dest opt
	std::vector<uint16>		optTimeList;
	std::vector<CQuat>		optKeyList;
	optTimeList.reserve(numSamples);
	optKeyList.reserve(numSamples);

	// Add the first key.
	optTimeList.push_back(_TimeList[0]);
	optKeyList.push_back(_KeyList[0]);
	double	timeRef= _TimeList[0];
	CQuatD	quatRef= _KeyList[0];

	// For all keys, but the first and the last, test if can remove them.
	for(uint i=1; i<numSamples-1; i++)
	{
		CQuatD	quatCur= _KeyList[i];
		CQuatD	quatNext= _KeyList[i+1];
		double	timeCur= _TimeList[i];
		double	timeNext= _TimeList[i+1];

		// must add the key?
		bool	mustAdd= false;

		// If the Delta time are too big, abort (CTrackSampledQuat limitation)
		if(timeNext-timeRef>255)
		{
			mustAdd= true;
		}
		// If the next quaternion or the current quaternion are not on same hemisphere than ref, abort.
		else if( CQuatD::dotProduct(quatCur, quatRef)<0 || CQuatD::dotProduct(quatNext, quatRef)<0 )
		{
			mustAdd= true;
		}
		// else, test interpolation
		else
		{
			// If the 3 quats are nearly equals, it is ok (avoid interpolation)
			if( nearlySameQuaternion(quatRef, quatCur) && nearlySameQuaternion(quatRef, quatNext) )
				mustAdd= false;
			else
			{
				// interpolate.
				CQuatD	quatInterpolated;
				double	t= (timeCur-timeRef)/(timeNext/timeRef);
				quatInterpolated= CQuatD::slerp(quatRef, quatNext, (float)t);

				// test if cur and interpolate are equal.
				if( !nearlySameQuaternion(quatCur, quatInterpolated) )
					mustAdd= true;
			}
		}

		// If must add the key to the optimized track.
		if(mustAdd)
		{
			optTimeList.push_back(_TimeList[i]);
			optKeyList.push_back(_KeyList[i]);
			timeRef= _TimeList[i];
			quatRef= _KeyList[i];
		}
	}

	// Add the last key.
	optTimeList.push_back(_TimeList[numSamples-1]);
	optKeyList.push_back(_KeyList[numSamples-1]);

	// copy the optimized track to the main one.
	_TimeList= optTimeList;
	_KeyList= optKeyList;
}


// ***************************************************************************
bool		CAnimationOptimizer::nearlySameQuaternion(const CQuatD &quat0, const CQuatD &quat1)
{
	// true if exactly same, or exactly inverse
	if(quat0==quat1 || quat0==-quat1)
		return true;

	// Else compute the rotation to go from qRef to q. Use double for better presion.
	CQuatD	quatDif;
	quatDif= quat1 * quat0.conjugate();
	// inverse the quaternion if necessary. ie make closest to the identity quaternion.
	if(quatDif.w<0)
		quatDif= -quatDif;

	// compare "angle threshold"
	return (quatDif.w >= _QuaternionThreshold);
}


// ***************************************************************************
void		CAnimationOptimizer::addLowPrecisionTrack(const std::string &name)
{
	_LowPrecTrackKeyName.push_back(name);
}

// ***************************************************************************
void		CAnimationOptimizer::clearLowPrecisionTracks()
{
	_LowPrecTrackKeyName.clear();
}

// ***************************************************************************
bool		CAnimationOptimizer::isLowPrecisionTrack(const std::string &trackName)
{
	for(uint i=0; i<_LowPrecTrackKeyName.size(); i++)
	{
		// if find a substr of the key, it is a low prec track
		if( trackName.find(_LowPrecTrackKeyName[i]) != string::npos )
			return true;
	}

	// no key found
	return false;
}


} // NL3D
