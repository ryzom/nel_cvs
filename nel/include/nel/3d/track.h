/** \file track.h
 * class ITrack
 *
 * $Id: track.h,v 1.1 2001/02/05 16:52:44 corvazier Exp $
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
#include "nel/3d/animation_time.h"
#include "nel/3d/animated_value.h"
#include "nel/3d/key.h"
#include <map>
#include <memory>


namespace NL3D 
{

template<class T> class CKey;

/**
 * The track store an animation of a IChannel. This animation can be interpolated
 * by several ways.
 *
 * The ITrack and IChannel types
 * MUST match else assertions will be raised.
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

/**
 * ITrack interface for keyframer.
 *
 * The ITrack and IChannel types
 * MUST match else assertions will be raised.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class ITrackKeyFramer : public ITrack
{
public:
	// NOT TESTED, JUST COMPILED. FOR PURPOSE ONLY.
	/// From ITrack. 
	virtual void eval (const CAnimationTime& date)
	{
		const CKey<T> *pPrevious=NULL;
		const CKey<T> *previous=NULL;
		const CKey<T> *next=NULL;
		const CKey<T> *nNext=NULL;
		CAnimationTime datePPrevious;
		CAnimationTime datePrevious;
		CAnimationTime dateNext;
		CAnimationTime dateNNext;

		// Return upper key
		std::map <CAnimationTime, std::auto_ptr<CKey<T> > >::iterator ite=_MapKey.upper_bound (date);

		// First next ?
		if (ite!=_MapKey.end())
		{
			// Next
			next=ite->second.get();
			dateNext=ite->first;

			// Next next
			std::map <CAnimationTime, std::auto_ptr<CKey<T> > >::iterator ite2=ite;
			ite2++;
			if (ite2!=_MapKey.end())
			{
				// Next
				nNext=ite2->second.get();
				dateNNext=ite2->first;
			}
		}

		// Previous
		ite--;

		// First previous ?
		if (ite!=_MapKey.end())
		{
			// Previous
			previous=ite->second.get();
			datePrevious=ite->first;

			// Previous previous
			std::map <CAnimationTime, std::auto_ptr<CKey<T> > >::iterator ite2=ite;
			ite2--;
			if (ite2!=_MapKey.end())
			{
				// Next
				pPrevious=ite2->second.get();
				datePPrevious=ite2->first;
			}
		}

		// Call evalutation fonction
		evalKey (pPrevious, previous, next, nNext, datePPrevious, datePrevious, dateNext, dateNNext, date);
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
	virtual void evalKey (	const CKey<T>* pPrevious, const CKey<T>* previous, const CKey<T>* next, const CKey<T>* nNext, 
							CAnimationTime datePPrevious, CAnimationTime datePrevious, 
							CAnimationTime dateNext, CAnimationTime dateNNext, 
							const CAnimationTime& date ) =0;
private:
	std::map <CAnimationTime, std::auto_ptr<CKey<T> > >		_MapKey;
};


/**
 * ITrack implementation for Constant keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CTrackKeyFramerConst : public ITrackKeyFramer<T>
{
public:

	/// From ITrack
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}
	
	/// From ITrackKeyFramer
	virtual void evalKey (	const CKey<T>* pPrevious, const CKey<T>* previous, const CKey<T>* next, const CKey<T>* nNext, 
							CAnimationTime datePPrevious, CAnimationTime datePrevious, 
							CAnimationTime dateNext, CAnimationTime dateNNext, 
							const CAnimationTime& date )
	{
		// Const key.
		if (previous)
			_Value.setValue (previous->Value);
		else
			if (next)
				_Value.setValue (next->Value);
	}

private:
	CAnimatedValueNotBlendable<T>		_Value;
};


/**
 * ITrack implementation for linear keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CTrackKeyFramerLinear : public ITrackKeyFramer<T>
{
public:

	/// From ITrack
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}
	
	/// From ITrackKeyFramer
	virtual void evalKey (	const CKey<T>* pPrevious, const CKey<T>* previous, const CKey<T>* next, const CKey<T>* nNext, 
							CAnimationTime datePPrevious, CAnimationTime datePrevious, 
							CAnimationTime dateNext, CAnimationTime dateNNext, 
							const CAnimationTime& date );

private:
	CAnimatedValueBlendable<T>	_Value;
};


/**
 * ITrack implementation for TCB keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CTrackKeyFramerTCB : public ITrackKeyFramer<T>
{
public:

	/// From ITrack
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}
	
	/// From ITrackKeyFramer
	virtual void evalKey (	const CKey<T>* pPrevious, const CKey<T>* previous, const CKey<T>* next, const CKey<T>* nNext, 
							CAnimationTime datePPrevious, CAnimationTime datePrevious, 
							CAnimationTime dateNext, CAnimationTime dateNNext, 
							const CAnimationTime& date );

private:
	CAnimatedValueBlendable<T>	_Value;
};


/**
 * ITrack implementation for Bezier keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CTrackKeyFramerBezier : public ITrackKeyFramer<T>
{
public:

	/// From ITrack
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}
	
	/// From ITrackKeyFramer
	virtual void evalKey (	const CKey<T>* pPrevious, const CKey<T>* previous, const CKey<T>* next, const CKey<T>* nNext, 
							CAnimationTime datePPrevious, CAnimationTime datePrevious, 
							CAnimationTime dateNext, CAnimationTime dateNNext, 
							const CAnimationTime& date );

private:
	CAnimatedValueBlendable<T>	_Value;
};

// Predefined types
typedef CTrackKeyFramerTCB<float> CTrackKeyFramerTCBFloat;
typedef CTrackKeyFramerTCB<NLMISC::CVector> CTrackKeyFramerTCBVector;
typedef CTrackKeyFramerTCB<NLMISC::CQuat> CTrackKeyFramerTCBQuat;
typedef CTrackKeyFramerTCB<int> CTrackKeyFramerTCBInt;

typedef CTrackKeyFramerBezier<float> CTrackKeyFramerBezierFloat;
typedef CTrackKeyFramerBezier<NLMISC::CVector> CTrackKeyFramerBezierVector;
typedef CTrackKeyFramerBezier<NLMISC::CQuat> CTrackKeyFramerBezierQuat;
typedef CTrackKeyFramerBezier<int> CTrackKeyFramerBezierInt;

typedef CTrackKeyFramerConst<std::string> CTrackKeyFramerConstString;
typedef CTrackKeyFramerConst<bool> CTrackKeyFramerConstBool;

} // NL3D


#endif // NL_TRACK_H

/* End of track.h */
