/** \file track.h
 * class ITrack
 *
 * $Id: track.h,v 1.7 2001/03/08 12:57:40 corvazier Exp $
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

#include "nel/misc/stream.h"
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
class ITrack : public NLMISC::IStreamable
{
public:
	/**
	  * Virtual destructor.
	  *
	  * \return the last value evaluated by ITrack::eval().
	  */
	virtual ~ITrack() {};

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
	// Some types
	typedef std::auto_ptr<CKeyT >					TAPtrCKey;
	typedef std::map <CAnimationTime, TAPtrCKey>	TMapTimeAPtrCKey;

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

	/// Serial the template
	virtual void serial (NLMISC::IStream& f) throw (NLMISC::EStream)
	{
		// Serial version
		sint version=f.serialVersion (0);

		// Is stream reading ?
		if (f.isReading())
		{
			// Read the size
			uint32 size;
			f.serial (size);

			// Reset the map
			_MapKey.clear();

			// Read element and insert in the map
			for (uint e=0; e<(uint)size; e++)
			{
				CAnimationTime time;
				CKeyT* keyPointer=NULL;

				// Serial element of the map
				f.serial (time);
				f.serialPolyPtr (keyPointer);

				// Insert in the map
				_MapKey.insert (TMapTimeAPtrCKey::value_type ( time, TAPtrCKey (keyPointer)));
			}
		}
		// Writing...
		else
		{
			// Size of the map
			uint32 size=(uint32)_MapKey.size();
			f.serial (size);

			// Write each element
			TMapTimeAPtrCKey::iterator ite=_MapKey.begin();
			while (ite!=_MapKey.end())
			{
				// Write the element
				CAnimationTime time=ite->first;
				CKeyT *keyPointer=ite->second.get();
				f.serial (time);
				f.serialPolyPtr (keyPointer);

				// Next element
				ite++;
			}
		}
	}
private:
	TMapTimeAPtrCKey		_MapKey;
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
							CAnimationTime date ) {};

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
							CAnimationTime date ) {};
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
class CTrackKeyFramerConstFloat : public CTrackKeyFramerConstBlendable<CKeyFloat,float>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstFloat);
};
class CTrackKeyFramerConstVector : public CTrackKeyFramerConstBlendable<CKeyVector, NLMISC::CVector>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstVector);
};
class CTrackKeyFramerConstQuat : public CTrackKeyFramerConstBlendable<CKeyQuat, NLMISC::CQuat>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstQuat);
};
class CTrackKeyFramerConstInt : public CTrackKeyFramerConstBlendable<CKeyInt, sint32>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstInt);
};
class CTrackKeyFramerConstString : public CTrackKeyFramerConstNotBlendable<CKeyString, std::string>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstString);
};
class CTrackKeyFramerConstBool : public CTrackKeyFramerConstNotBlendable<CKeyBool, bool>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstBool);
};


// Linear tracks.
class CTrackKeyFramerLinearFloat : public CTrackKeyFramerLinear<CKeyFloat, float>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerLinearFloat);
};
class CTrackKeyFramerLinearVector : public CTrackKeyFramerLinear<CKeyVector, NLMISC::CVector>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerLinearVector);
};
class CTrackKeyFramerLinearQuat : public CTrackKeyFramerLinear<CKeyQuat, NLMISC::CQuat>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerLinearQuat);
};
class CTrackKeyFramerLinearInt : public CTrackKeyFramerLinear<CKeyInt, sint32>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerLinearInt);
};


// TCB tracks.
class CTrackKeyFramerTCBFloat : public CTrackKeyFramerTCB<CKeyTCBFloat, float>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerTCBFloat);
};
class CTrackKeyFramerTCBVector : public CTrackKeyFramerTCB<CKeyTCBVector, NLMISC::CVector>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerTCBVector);
};
class CTrackKeyFramerTCBQuat : public CTrackKeyFramerTCB<CKeyTCBQuat, NLMISC::CQuat>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerTCBQuat);
};
class CTrackKeyFramerTCBInt : public CTrackKeyFramerTCB<CKeyTCBInt, sint32>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerTCBInt);
};


// Bezier tracks.
class CTrackKeyFramerBezierFloat : public CTrackKeyFramerBezier<CKeyBezierFloat, float>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerBezierFloat);
};
class CTrackKeyFramerBezierVector : public CTrackKeyFramerBezier<CKeyBezierVector, NLMISC::CVector>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerBezierVector);
};
class CTrackKeyFramerBezierQuat : public CTrackKeyFramerBezier<CKeyBezierQuat, NLMISC::CQuat>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerBezierQuat);
};
class CTrackKeyFramerBezierInt : public CTrackKeyFramerBezier<CKeyBezierInt, sint32>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerBezierInt);
};



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

	/// Serial the template
	virtual void serial (NLMISC::IStream& f) throw (NLMISC::EStream)
	{
		// Serial version
		sint version=f.serialVersion (0);

		// Serial the value
		f.serial (_Value.Value);
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

	/// Serial the template
	virtual void serial (NLMISC::IStream& f) throw (NLMISC::EStream)
	{
		// Serial version
		sint version=f.serialVersion (0);

		// Serial the value
		f.serial (_Value.Value);
	}
private:

	// The default value
	CAnimatedValueNotBlendable<T>	_Value;
};

// Predefined types
class CTrackDefaultFloat : public CTrackDefaultBlendable<float>
{
public:
	NLMISC_DECLARE_CLASS (CTrackDefaultFloat);
};
class CTrackDefaultVector : public CTrackDefaultBlendable<NLMISC::CVector>
{
public:
	NLMISC_DECLARE_CLASS (CTrackDefaultVector);
};
class CTrackDefaultQuat : public CTrackDefaultBlendable<NLMISC::CQuat>
{
public:
	NLMISC_DECLARE_CLASS (CTrackDefaultQuat);
};
class CTrackDefaultInt : public CTrackDefaultBlendable<sint32>
{
public:
	NLMISC_DECLARE_CLASS (CTrackDefaultInt);
};
class CTrackDefaultString : public CTrackDefaultNotBlendable<std::string>
{
public:
	NLMISC_DECLARE_CLASS (CTrackDefaultString);
};
class CTrackDefaultBool : public CTrackDefaultNotBlendable<bool>
{
public:
	NLMISC_DECLARE_CLASS (CTrackDefaultBool);
};

} // NL3D


#endif // NL_TRACK_H

/* End of track.h */
