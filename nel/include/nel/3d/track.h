/** \file track.h
 * class ITrack
 *
 * $Id: track.h,v 1.14 2001/03/16 16:42:32 berenguier Exp $
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

	/**
	  * Get begin time of the track.
	  *
	  * \return the begin time.
	  */
	virtual CAnimationTime getBeginTime () const=0;

	/**
	  * Get end time of the track.
	  *
	  * \return the end time.
	  */
	virtual CAnimationTime getEndTime () const=0;
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
	typedef std::map <CAnimationTime, CKeyT>	TMapTimeCKey;


	/// ctor.
	ITrackKeyFramer ()
	{
		_Dirty= false;
	}


	/// Destructor
	~ITrackKeyFramer ()
	{
	}

	/**
	  * Add a key in the keyframer.
	  *
	  * The key passed is duplicated in the track.
	  *
	  * \param key is the key value to add in the keyframer.
	  * \param time is the time of the key to add in the keyframer.
	  */
	void addKey (const CKeyT &key, CAnimationTime time)
	{
		// Insert the key in the map
		_MapKey.insert (TMapTimeCKey::value_type (time, key));

		// must precalc at next eval.
		_Dirty= true;
	}

	/// From ITrack. 
	virtual void eval (const CAnimationTime& date)
	{
		const CKeyT *previous=NULL;
		const CKeyT *next=NULL;
		CAnimationTime datePrevious;
		CAnimationTime dateNext;

		// must precalc ??
		if(_Dirty)
		{
			compile();
			_Dirty= false;
		}

		// Return upper key
		TMapTimeCKey::iterator ite=_MapKey.upper_bound (date);

		// First next ?
		if (ite!=_MapKey.end())
		{
			// Next
			next= &(ite->second);
			dateNext=ite->first;
		}

		// Previous
		ite--;

		// First previous ?
		if (ite!=_MapKey.end())
		{
			// Previous
			previous= &(ite->second);
			datePrevious=ite->first;
		}

		// Call evalutation fonction
		evalKey (previous, next, datePrevious, dateNext, date);
	}
	CAnimationTime getBeginTime () const
	{
		// Get first key
		TMapTimeCKey::const_iterator ite=_MapKey.begin ();
		if (ite==_MapKey.end())
			return 0.f;
		else
			return ite->first;
	}
	CAnimationTime getEndTime () const
	{
		// Get first key
		TMapTimeCKey::const_iterator ite=_MapKey.end ();
		ite--;
		if (ite==_MapKey.end())
			return 0.f;
		else
			return ite->first;
	}

	/// Serial the template
	virtual void serial (NLMISC::IStream& f) throw (NLMISC::EStream)
	{
		// Serial version
		sint version=f.serialVersion (0);

		f.serialMap(_MapKey);
	}

private:
	TMapTimeCKey		_MapKey;
	bool				_Dirty;


protected:
	/**
	  * Precalc keyframe runtime infos for interpolation (OODTime...). All keys should be processed.
	  * This is called by evalKey when necessary. Deriver should call ITrackKeyFramer::compile(), to compile basic
	  * Key runtime info.
	  */
	virtual void compile   ()
	{
		TMapTimeCKey::iterator	it= _MapKey.begin();
		for(;it!=_MapKey.end();it++)
		{
			TMapTimeCKey::iterator	next= it;
			next++;
			if(next!=_MapKey.end())
				it->second.OODeltaTime= 1.0f/(next->first - it->first);
			else
				it->second.OODeltaTime= 0.0f;
		}
	}

	/**
	  * Evaluate the keyframe interpolation. 
	  *
	  * i is the keyframe with the bigger time value that is inferior or equal than date.
	  *
	  * \param previous is the i key in the keyframe. NULL if no key.
	  * \param next is the i+1 key in the keyframe. NULL if no key.
	  */
	virtual void evalKey   (const CKeyT* previous, const CKeyT* next,
							CAnimationTime datePrevious, CAnimationTime dateNext, 
							CAnimationTime date ) =0;

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
	CAnimationTime getBeginTime () const
	{
		return 0.f;
	}
	CAnimationTime getEndTime () const
	{
		return 0.f;
	}
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
							CAnimationTime date )
	{
		if(previous && next)
		{
			// slerp from previous to cur.
			date-= datePrevious;
			date/= (dateNext-datePrevious);
			NLMISC::clamp(date, 0,1);
			_Value.Value= NLMISC::CQuat::slerp(previous->Value, next->Value, date);
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
	CAnimatedValueBlendable<NLMISC::CQuat>	_Value;
};


// ***************************************************************************
/**
 * ITrack implementation for CQuat TCB keyframer.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CTrackKeyFramerTCB<CKeyTCBQuat, NLMISC::CAngleAxis> : public ITrackKeyFramer<CKeyTCBQuat>
{
public:

	/// From ITrack
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}
	
	/// From ITrackKeyFramer
	virtual void evalKey (	const CKeyTCBQuat* previous, const CKeyTCBQuat* next, 
							CAnimationTime datePrevious, CAnimationTime dateNext,
							CAnimationTime date ) {};

private:
	CAnimatedValueBlendable<NLMISC::CQuat>	_Value;
};


// ***************************************************************************
// ***************************************************************************
// RGBA special implementation..
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
/**
 * ITrack implementation for linear CRGBA keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CTrackKeyFramerLinear<CKeyRGBA, NLMISC::CRGBA>: public ITrackKeyFramer<CKeyRGBA>
{
public:

	/// From ITrack
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}
	
	/// From ITrackKeyFramer
	virtual void evalKey (	const CKeyRGBA* previous, const CKeyRGBA* next,
							CAnimationTime datePrevious, CAnimationTime dateNext,
							CAnimationTime date )
	{
		if(previous && next)
		{
			// lerp from previous to cur.
			date-= datePrevious;
			date/= (dateNext-datePrevious);
			NLMISC::clamp(date, 0,1);
			
			// blend.
			_Value.Value.blendFromui (previous->Value, next->Value, (uint)(255.f*date));
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
	CAnimatedValueBlendable<NLMISC::CRGBA>	_Value;
};



// ***************************************************************************
// ***************************************************************************
// Int special implementation..
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
/**
 * ITrack implementation for linear int keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CTrackKeyFramerLinear<CKeyInt, sint32>: public ITrackKeyFramer<CKeyInt>
{
public:

	/// From ITrack
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}
	
	/// From ITrackKeyFramer
	virtual void evalKey (	const CKeyInt* previous, const CKeyInt* next,
							CAnimationTime datePrevious, CAnimationTime dateNext,
							CAnimationTime date )
	{
		if(previous && next)
		{
			// lerp from previous to cur.
			date-= datePrevious;
			date/= (dateNext-datePrevious);
			NLMISC::clamp(date, 0,1);
			
			// blend.
			_Value.Value= (sint32) (0.5+floor (((float)previous->Value*(1.f-date) + (float)next->Value*date)));
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
	CAnimatedValueBlendable<sint32>	_Value;
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
class CTrackKeyFramerConstRGBA : public CTrackKeyFramerConstNotBlendable<CKeyRGBA, NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstRGBA);
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
class CTrackKeyFramerLinearRGBA : public CTrackKeyFramerLinear<CKeyRGBA, NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerLinearRGBA);
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
class CTrackKeyFramerTCBQuat : public CTrackKeyFramerTCB<CKeyTCBQuat, NLMISC::CAngleAxis>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerTCBQuat);
};
class CTrackKeyFramerTCBInt : public CTrackKeyFramerTCB<CKeyTCBInt, sint32>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerTCBInt);
};
class CTrackKeyFramerTCBRGBA : public CTrackKeyFramerTCB<CKeyTCBRGBA, NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerTCBRGBA);
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
class CTrackKeyFramerBezierRGBA : public CTrackKeyFramerBezier<CKeyBezierRGBA, NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerBezierRGBA);
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

	CTrackDefaultBlendable()
	{
	}
	CTrackDefaultBlendable(const T &val)
	{
		_Value.Value= val;
	}

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

	CTrackDefaultNotBlendable()
	{
	}
	CTrackDefaultNotBlendable(const T &val)
	{
		_Value.Value= val;
	}


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


#define	NL3D_TRACKDEF_CTOR(_Son, _Father, _T)	\
	_Son() {}									\
	_Son(const _T &v) : _Father<_T>(v) {}


// Predefined types
class CTrackDefaultFloat : public CTrackDefaultBlendable<float>
{
public:
	NL3D_TRACKDEF_CTOR(CTrackDefaultFloat, CTrackDefaultBlendable, float);
	NLMISC_DECLARE_CLASS (CTrackDefaultFloat);
};
class CTrackDefaultVector : public CTrackDefaultBlendable<NLMISC::CVector>
{
public:
	NL3D_TRACKDEF_CTOR(CTrackDefaultVector, CTrackDefaultBlendable, NLMISC::CVector);
	NLMISC_DECLARE_CLASS (CTrackDefaultVector);
};
class CTrackDefaultQuat : public CTrackDefaultBlendable<NLMISC::CQuat>
{
public:
	NL3D_TRACKDEF_CTOR(CTrackDefaultQuat, CTrackDefaultBlendable, NLMISC::CQuat);
	NLMISC_DECLARE_CLASS (CTrackDefaultQuat);
};
class CTrackDefaultInt : public CTrackDefaultBlendable<sint32>
{
public:
	NL3D_TRACKDEF_CTOR(CTrackDefaultInt, CTrackDefaultBlendable, sint32);
	NLMISC_DECLARE_CLASS (CTrackDefaultInt);
};

class CTrackDefaultRGBA : public CTrackDefaultBlendable<NLMISC::CRGBA>
{
public:
	NL3D_TRACKDEF_CTOR(CTrackDefaultRGBA, CTrackDefaultBlendable, NLMISC::CRGBA);
	NLMISC_DECLARE_CLASS (CTrackDefaultRGBA);
};


class CTrackDefaultString : public CTrackDefaultNotBlendable<std::string>
{
public:
	NL3D_TRACKDEF_CTOR(CTrackDefaultString, CTrackDefaultNotBlendable, std::string);
	NLMISC_DECLARE_CLASS (CTrackDefaultString);
};
class CTrackDefaultBool : public CTrackDefaultNotBlendable<bool>
{
public:
	NL3D_TRACKDEF_CTOR(CTrackDefaultBool, CTrackDefaultNotBlendable, bool);
	NLMISC_DECLARE_CLASS (CTrackDefaultBool);
};

} // NL3D


#endif // NL_TRACK_H

/* End of track.h */
