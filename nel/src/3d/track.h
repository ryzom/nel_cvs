/** \file track.h
 * class ITrack
 *
 * $Id: track.h,v 1.5 2002/08/21 09:39:54 lecroart Exp $
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
#include "nel/3d/u_track.h"

#include "3d/animated_value.h"


namespace NL3D 
{


using NLMISC::CQuat;
using NLMISC::CVector;


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
class ITrack : public NLMISC::IStreamable, public UTrack
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
	virtual void eval (const TAnimationTime& date)=0;

	/**
	  * Get the track current value.
	  *
	  * \return the last value evaluated by ITrack::eval().
	  */
	virtual const IAnimatedValue& getValue () const=0;

	/**
	  * get LoopMode. 2 mode only: "constant" (<=>false), and "loop" (<=> true). 
	  *	NB: same mode if time < getBeginTIme() and if time > getEndTime()
	  */
	virtual bool getLoopMode() const=0;

	/// \name From UTrack
	// @{

	virtual bool interpolate (TAnimationTime time, float& res);
	virtual bool interpolate (TAnimationTime time, sint32& res);
	virtual bool interpolate (TAnimationTime time, NLMISC::CRGBA& res);
	virtual bool interpolate (TAnimationTime time, NLMISC::CVector& res);
	virtual bool interpolate (TAnimationTime time, NLMISC::CQuat& res);
	virtual bool interpolate (TAnimationTime time, std::string& res);
	virtual bool interpolate (TAnimationTime time, bool& res);

	// @}
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
	virtual void eval (const TAnimationTime& date) 
	{}
	TAnimationTime getBeginTime () const
	{
		return 0.f;
	}
	TAnimationTime getEndTime () const
	{
		return 0.f;
	}
	virtual bool getLoopMode() const {return true;}
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

	/// set the default value.
	void	setValue(const T &val)
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
		(void)f.serialVersion (0);

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

	/// set the default value.
	void	setValue(const T &val)
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
		(void)f.serialVersion (0);

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
class CTrackDefaultVector : public CTrackDefaultBlendable<CVector>
{
public:
	NL3D_TRACKDEF_CTOR(CTrackDefaultVector, CTrackDefaultBlendable, CVector);
	NLMISC_DECLARE_CLASS (CTrackDefaultVector);
};
class CTrackDefaultQuat : public CTrackDefaultBlendable<CQuat>
{
public:
	NL3D_TRACKDEF_CTOR(CTrackDefaultQuat, CTrackDefaultBlendable, CQuat);
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
