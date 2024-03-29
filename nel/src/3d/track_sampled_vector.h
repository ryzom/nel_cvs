/** \file track_sampled_vector.h
 * TODO: File description
 *
 * $Id: track_sampled_vector.h,v 1.4 2005/02/22 10:19:12 besson Exp $
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

#ifndef NL_TRACK_SAMPLED_VECTOR_H
#define NL_TRACK_SAMPLED_VECTOR_H

#include "nel/misc/types_nl.h"
#include "nel/misc/object_vector.h"
#include "nel/misc/quat.h"
#include "track_sampled_common.h"


namespace NL3D {


// ***************************************************************************
/**
 * This track is supposed to be Lighter in memory than CTrackKeyFramerTCBVector, and also is maybe faster.
 *	The track is an oversampled version of CTrackKeyFramerTCBVector (or any vector interpolator), to 30 fps for example,
 *	but each key is 13 bytes in memory, instead of 96.
 *	Only linear interpolation is performed between 2 keys. And Keys are precomputed
 *
 *	13 bytes per key is achieved by encoding this way:
 *		1 byte for the length/key time, measured in samples, and not in second (hence we can skip at max 255 keys).
 *		12 byte for the position. May be compressed in future to 6 bytes.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CTrackSampledVector : public CTrackSampledCommon
{
public:

	/// Constructor
	CTrackSampledVector();
	virtual ~CTrackSampledVector();
	NLMISC_DECLARE_CLASS (CTrackSampledVector);

	/// From UTrack/ITrack.
	// @{
	virtual const IAnimatedValue	&eval (const TAnimationTime& date, CAnimatedValueBlock &avBlock);
	virtual void					serial(NLMISC::IStream &f);
	virtual void					applySampleDivisor(uint sampleDivisor);
	// @}

	/** Build the track from a list of keys 
	 *	nlassert if ! same length.
	 *	\param timeList the list of key time. First must be ==0. nlassert if difference between 2 keys is > 255
	 *	\param keyList the list of keys, not yet compressed (done internally in future)
	 *	\param beginTime map to the timeList[0] time.
	 *	\param endTime map to the timeList[size-1] time.
	 */
	void	build(const std::vector<uint16> &timeList, const std::vector<CVector> &keyList, 
		float beginTime, float endTime);

// **********************
protected:

	// Key Values
	NLMISC::CObjectVector<CVector, false>		_Keys;
};



} // NL3D


#endif // NL_TRACK_SAMPLED_VECTOR_H

/* End of track_sampled_vector.h */
