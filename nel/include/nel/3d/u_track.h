/** \file u_track.h
 * User interface to access tracks fonctionnalities
 *
 * $Id: u_track.h,v 1.1 2001/07/03 09:46:22 corvazier Exp $
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

#ifndef NL_U_TRACK_H
#define NL_U_TRACK_H

#include "nel/misc/types_nl.h"

namespace NLMISC
{

class CQuat;
class CRGBA;
class CVector;

}

namespace NL3D 
{

/**
 * A track is a fonction that interpolate a value over the time.
 *
 * Kind of interpolation is hidden to the user. It can be Bezier, TCB, linear, noise interpolation.
 * This interface give access to the interpolation fonction.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class UTrack
{
public:

	/// Constructor
	UTrack();

	/// \name Time range methods.
	// @{

	/**
	  * Get the begin time of the track
	  */
	virtual CAnimationTime getBeginTime () const=0;

	/**
	  * Get the end time of the track
	  */
	virtual CAnimationTime getEndTime () const=0;

	// @}

	/// \name Interpolation methods.
	// @{

	/**
	  * Interplation a float value. You should be sure that the track you use to interpolate
	  * your value is a float track! An assertion will be raised in debug if the type is wrong.
	  *
	  * \param time is the time you want the evaluate the value. If time higher than the time
	  * gived by getEndTime (), the value returned is the interpolation value at getEndTime ().
	  * If time smaller than the time gived by getBeginTime (), the value returned is the 
	  * interpolation value at getBeginTime ().
	  * \param res is the reference on the value to get the result.
	  * \return true if interplation is successful. false if the type asked is wrong.
	  */
	virtual bool interpolate (CAnimationTime time, float& res) =0;

	/**
	  * Interplation an integer value. You should be sure that the track you use to interpolate
	  * your value is an integer track! An assertion will be raised in debug if the type is wrong.
	  *
	  * \param time is the time you want the evaluate the value. If time higher than the time
	  * gived by getEndTime (), the value returned is the interpolation value at getEndTime ().
	  * If time smaller than the time gived by getBeginTime (), the value returned is the 
	  * interpolation value at getBeginTime ().
	  * \param res is the reference on the value to get the result.
	  * \return true if interplation is successful. false if the type asked is wrong.
	  */
	virtual bool interpolate (CAnimationTime time, sint32& res) =0;

	/**
	  * Interplation a CRGBA value. You should be sure that the track you use to interpolate
	  * your value is an CRGBA track! An assertion will be raised in debug if the type is wrong.
	  *
	  * \param time is the time you want the evaluate the value. If time higher than the time
	  * gived by getEndTime (), the value returned is the interpolation value at getEndTime ().
	  * If time smaller than the time gived by getBeginTime (), the value returned is the 
	  * interpolation value at getBeginTime ().
	  * \param res is the reference on the value to get the result.
	  * \return true if interplation is successful. false if the type asked is wrong.
	  */
	virtual bool interpolate (CAnimationTime time, NLMISC::CRGBA& res) =0;

	/**
	  * Interplation a CVector value. You should be sure that the track you use to interpolate
	  * your value is a CVector track! An assertion will be raised in debug if the type is wrong.
	  *
	  * \param time is the time you want the evaluate the value. If time higher than the time
	  * gived by getEndTime (), the value returned is the interpolation value at getEndTime ().
	  * If time smaller than the time gived by getBeginTime (), the value returned is the 
	  * interpolation value at getBeginTime ().
	  * \param res is the reference on the value to get the result.
	  * \return true if interplation is successful. false if the type asked is wrong.
	  */
	virtual bool interpolate (CAnimationTime time, NLMISC::CVector& res) =0;

	/**
	  * Interplation a CQuat value. You should be sure that the track you use to interpolate
	  * your value is a CQuat track! An assertion will be raised in debug if the type is wrong.
	  *
	  * \param time is the time you want the evaluate the value. If time higher than the time
	  * gived by getEndTime (), the value returned is the interpolation value at getEndTime ().
	  * If time smaller than the time gived by getBeginTime (), the value returned is the 
	  * interpolation value at getBeginTime ().
	  * \param res is the reference on the value to get the result.
	  * \return true if interplation is successful. false if the type asked is wrong.
	  */
	virtual bool interpolate (CAnimationTime time, NLMISC::CQuat& res) =0;

	/**
	  * Interplation a string value. You should be sure that the track you use to interpolate
	  * your value is a string track! An assertion will be raised in debug if the type is wrong.
	  *
	  * \param time is the time you want the evaluate the value. If time higher than the time
	  * gived by getEndTime (), the value returned is the interpolation value at getEndTime ().
	  * If time smaller than the time gived by getBeginTime (), the value returned is the 
	  * interpolation value at getBeginTime ().
	  * \param res is the reference on the value to get the result.
	  * \return true if interplation is successful. false if the type asked is wrong.
	  */
	virtual bool interpolate (CAnimationTime time, std::string& res) =0;

	/**
	  * Interplation a bool value. You should be sure that the track you use to interpolate
	  * your value is a bool track! An assertion will be raised in debug if the type is wrong.
	  *
	  * \param time is the time you want the evaluate the value. If time higher than the time
	  * gived by getEndTime (), the value returned is the interpolation value at getEndTime ().
	  * If time smaller than the time gived by getBeginTime (), the value returned is the 
	  * interpolation value at getBeginTime ().
	  * \param res is the reference on the value to get the result.
	  * \return true if interplation is successful. false if the type asked is wrong.
	  */
	virtual bool interpolate (CAnimationTime time, bool& res) =0;

	// @}
};


} // NL3D


#endif // NL_U_TRACK_H

/* End of u_track.h */
