/** \file track.h
 * class ITrack
 *
 * $Id: track.h,v 1.4 2001/03/07 17:07:58 berenguier Exp $
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

#ifndef NL_TRACK_H
#define NL_TRACK_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/3d/animation_time.h"
#include "nel/3d/animated_value.h"
#include "nel/3d/key.h"
#include <map>
#include <memory>


namespace NL3D 
{


// ***************************************************************************
/**
 * The track store an animation of an animated value. This animation can be interpolated
 * by several ways.
 *
 * The ITrack and animated value types MUST match else assertions will be raised.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class ITrack
{
public:
	/**
	  * Evaluation of the value of the track for this time.
	  *
	  * The result is internaly stored to simplify access at the polymorphic values.
	  * To get the value, call ITrack::getValue().
	  */
	virtual void eval (const CAnimationTime& date)=0;

	/**
	  * Get the track current value.
	  *
	  * \return the last value evaluated by ITrack::eval().
	  */
	virtual const IAnimatedValue& getValue () const=0;
};


// ***************************************************************************
// ***************************************************************************
// Templates for KeyFramer tracks.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
/**
 * ITrack interface for keyframer.
 *
 * The ITrack and animated value types MUST match else assertions will be raised.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class CKeyT>
class ITrackKeyFramer : public ITrack
{
public:
	// NOT TESTED, JUST COMPILED. FOR PURPOSE ONLY.
	/// From ITrack. 
	virtual void eval (const CAnimationTime& date)
	{
		const CKeyT *previous=NULL;
		const CKeyT *next=NULL;
		CAnimationTime datePrevious;
		CAnimationTime dateNext;

		// Return upper key
		std::map <CAnimationTime, std::auto_ptr<CKeyT > >::iterator ite=_MapKey.upper_bound (date);

		// First next ?
		if (ite!=_MapKey.end())
		{
			// Next
			next=ite->second.get();
			dateNext=ite->first;
		}

		// Previous
		ite--;

		// First previous ?
		if (ite!=_MapKey.end())
		{
			// Previous
			previous=ite->second.get();
			datePrevious=ite->first;
		}

		// Call evalutation fonction
		evalKey (previous, next, datePrevious, dateNext, date);
	}

	/**
	  * Evaluate the keyframe interpolation. 
	  *
	  * i is the keyframe with the bigger time value that is inferior or equal than date.
	  *
	  * \param pPrevious is the i-1 key in the keyframe. NULL if no key.
	  * \param previous is the i key in the keyframe. NULL if no key.
	  * \param next is the i+1 key in the keyframe. NULL if no key.
	  * \param nNext is the i+2 key in the keyframe. NULL if no key.
	  */
	virtual void evalKey (	const CKeyT* previous, const CKeyT* next,
							CAnimationTime datePrevious, CAnimationTime dateNext, 
							CAnimationTime date ) =0;
private:
	std::map <CAnimationTime, std::auto_ptr<CKeyT > >		_MapKey;
};


// ***************************************************************************
/**
 * ITrack interface for default tracks.
 *
 * The ITrack and animated value types MUST match else assertions will be raised.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class ITrackDefault : public ITrack
{
public:
	/// From ITrack. Does nothing, no interpolation.
	virtual void eval (const CAnimationTime& date) 
	{}
};


// ***************************************************************************
/**
 * ITrack implementation for Constant keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class CKeyT, class T>
class CTrackKeyFramerConstNotBlendable : public ITrackKeyFramer<CKeyT>
{
public:

	/// From ITrack
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}
	
	/// From ITrackKeyFramer
	virtual void evalKey (	const CKeyT* previous, const CKeyT* next,
							CAnimationTime datePrevious, CAnimationTime dateNext,
							CAnimationTime date )
	{
		// Const key.
		if (previous)
			_Value.Value=previous->Value;
		else
			if (next)
				_Value.Value=next->Value;
	}

private:
	CAnimatedValueNotBlendable<T>		_Value;
};


// ***************************************************************************
/**
 * ITrack implementation for Constant keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class CKeyT, class T>
class CTrackKeyFramerConstBlendable : public ITrackKeyFramer<CKeyT>
{
public:

	/// From ITrack
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}
	
	/// From ITrackKeyFramer
	virtual void evalKey (	const CKeyT* previous, const CKeyT* next,
							CAnimationTime datePrevious, CAnimationTime dateNext,
							CAnimationTime date )
	{
		// Const key.
		if (previous)
			_Value.Value=previous->Value;
		else
			if (next)
				_Value.Value=next->Value;
	}

private:
	CAnimatedValueBlendable<T>		_Value;
};


// ***************************************************************************
/**
 * ITrack implementation for linear keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class CKeyT, class T>
class CTrackKeyFramerLinear : public ITrackKeyFramer<CKeyT>
{
public:

	/// From ITrack
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}
	
	/// From ITrackKeyFramer
	virtual void evalKey (	const CKeyT* previous, const CKeyT* next,
							CAnimationTime datePrevious, CAnimationTime dateNext,
							CAnimationTime date )
	{
		if(previous && next)
		{
			// lerp from previous to cur.
			date-= datePrevious;
			date/= (dateNext-datePrevious);
			NLMISC::clamp(date, 0,1);
			_Value.Value= (T) (previous->Value*(1.f-date) + next->Value*date);
		}
		else
		{
			if (previous)
				_Value.Value=previous->Value;
			else
				if (next)
					_Value.Value=next->Value;
		}

	}

private:
	CAnimatedValueBlendable<T>	_Value;
};


// ***************************************************************************
/**
 * ITrack implementation for TCB keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class CKeyT, class T>
class CTrackKeyFramerTCB : public ITrackKeyFramer<CKeyT>
{
public:

	/// From ITrack
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}
	
	/// From ITrackKeyFramer
	virtual void evalKey (	const CKeyT* previous, const CKeyT* next,
							CAnimationTime datePrevious, CAnimationTime dateNext,
							CAnimationTime date );

private:
	CAnimatedValueBlendable<T>	_Value;
};


// ***************************************************************************
/**
 * ITrack implementation for Bezier keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class CKeyT, class T>
class CTrackKeyFramerBezier : public ITrackKeyFramer<CKeyT>
{
public:

	/// From ITrack
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}
	
	/// From ITrackKeyFramer
	virtual void evalKey (	const CKeyT* previous, const CKeyT* next,
							CAnimationTime datePrevious, CAnimationTime dateNext,
							CAnimationTime date );

private:
	CAnimatedValueBlendable<T>	_Value;
};



// ***************************************************************************
// ***************************************************************************
// Quaternions special implementation..
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
/**
 * ITrack implementation for CQuat linear keyframer.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CTrackKeyFramerLinear<CKeyQuat, NLMISC::CQuat> : public ITrackKeyFramer<CKeyQuat>
{
public:

	/// From ITrack
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}
	
	/// From ITrackKeyFramer
	virtual void evalKey (	const CKeyQuat* previous, const CKeyQuat* next, 
							CAnimationTime datePrevious, CAnimationTime dateNext,
							CAnimationTime date );

private:
	CAnimatedValueBlendable<NLMISC::CQuat>	_Value;
};



// ***************************************************************************
// ***************************************************************************
// Predefined types for KeyFramer tracks.
// ***************************************************************************
// ***************************************************************************


// Const tracks.
typedef CTrackKeyFramerConstBlendable<CKeyFloat,float>				CTrackKeyFramerConstFloat;
typedef CTrackKeyFramerConstBlendable<CKeyVector, NLMISC::CVector>	CTrackKeyFramerConstVector;
typedef CTrackKeyFramerConstBlendable<CKeyQuat, NLMISC::CQuat>		CTrackKeyFramerConstQuat;
typedef CTrackKeyFramerConstBlendable<CKeyInt, int>					CTrackKeyFramerConstInt;
typedef CTrackKeyFramerConstNotBlendable<CKeyString, std::string>	CTrackKeyFramerConstString;
typedef CTrackKeyFramerConstNotBlendable<CKeyBool, bool>			CTrackKeyFramerConstBool;


// Linear tracks.
typedef CTrackKeyFramerLinear<CKeyFloat, float>						CTrackKeyFramerLinearFloat;
typedef CTrackKeyFramerLinear<CKeyVector, NLMISC::CVector>			CTrackKeyFramerLinearVector;
typedef CTrackKeyFramerLinear<CKeyQuat, NLMISC::CQuat>				CTrackKeyFramerLinearQuat;
typedef CTrackKeyFramerLinear<CKeyInt, int>							CTrackKeyFramerLinearInt;


// TCB tracks.
typedef CTrackKeyFramerTCB<CKeyTCBFloat, float>						CTrackKeyFramerTCBFloat;
typedef CTrackKeyFramerTCB<CKeyTCBVector, NLMISC::CVector>			CTrackKeyFramerTCBVector;
typedef CTrackKeyFramerTCB<CKeyTCBQuat, NLMISC::CQuat>				CTrackKeyFramerTCBQuat;
typedef CTrackKeyFramerTCB<CKeyTCBInt, int>							CTrackKeyFramerTCBInt;


// Bezier tracks.
typedef CTrackKeyFramerBezier<CKeyBezierFloat, float>				CTrackKeyFramerBezierFloat;
typedef CTrackKeyFramerBezier<CKeyBezierVector, NLMISC::CVector>	CTrackKeyFramerBezierVector;
typedef CTrackKeyFramerBezier<CKeyBezierQuat, NLMISC::CQuat>		CTrackKeyFramerBezierQuat;
typedef CTrackKeyFramerBezier<CKeyBezierInt, int>					CTrackKeyFramerBezierInt;



// ***************************************************************************
// ***************************************************************************
// TrackDefault implemenations.
// ***************************************************************************
// ***************************************************************************



/**
 * ITrackDefault implementation for blendable values.
 *
 * The ITrack and animated value types MUST match else assertions will be raised.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CTrackDefaultBlendable : public ITrackDefault
{
public:
	/// From ITrack. Return a const value.
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}

private:

	// The default value
	CAnimatedValueBlendable<T>	_Value;
};


/**
 * ITrackDefault implementation for blendable values.
 *
 * The ITrack and animated value types MUST match else assertions will be raised.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CTrackDefaultNotBlendable : public ITrackDefault
{
public:
	/// From ITrack. Return a const value.
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}

private:

	// The default value
	CAnimatedValueNotBlendable<T>	_Value;
};

// Predefined types
typedef CTrackDefaultBlendable<float>				CTrackDefaultFloat;
typedef CTrackDefaultBlendable<NLMISC::CVector>		CTrackDefaultVector;
typedef CTrackDefaultBlendable<NLMISC::CQuat>		CTrackDefaultQuat;
typedef CTrackDefaultBlendable<int>					CTrackDefaultInt;
typedef CTrackDefaultNotBlendable<std::string>		CTrackDefaultString;
typedef CTrackDefaultNotBlendable<bool>				CTrackDefaultBool;

} // NL3D


#endif // NL_TRACK_H

/* End of track.h */
