/** \file animatable.h
 * Class IAnimatable
 *
 * $Id: animatable.h,v 1.3 2001/02/12 15:42:10 corvazier Exp $
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

#ifndef NL_ANIMATABLE_H
#define NL_ANIMATABLE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/bit_set.h"
#include <string>
#include <vector>
#include <map>


namespace NL3D 
{

class ITrack;

/**
 * An animatable object. 
 *
 * This object can have a set of animated values.
 * Animated values are animated by a CChannelMixer object.
 * Each value have a name and a default track.
 *
 * When a class derives from IAnimatable, it must implement all the 
 * interface's methods:
 *
 *	virtual uint getValueCount () const;
 *	virtual IAnimatedValue* getValue (uint valueId);
 *	virtual const std::string& getValueName (uint valueId) const;
 *	virtual ITrack* getDefaultTrack (uint valueId);
 *
 * Watch NL3D::CTransform for exemple.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class IAnimatable
{
	friend class IAnimatedValue;
public:

	/**
	  * Default Constructor. Set number of value to 0.
	  * 
	  */
	IAnimatable ()
	{
		bitSet.resize (1);
	}

	/// \name Interface

	/**
	  * Get animated value count.
	  *
	  * \return number of animated value in this object.
	  */
	virtual uint getValueCount () const =0;

	/** 
	  * Get a value pointer.
	  *
	  * \param valueId is the animated value ID in the object.
	  * \return The pointer on the animated value.
	  */
	virtual IAnimatedValue* getValue (uint valueId) =0;

	/**
	  * Get animated value name.
	  *
	  * \param valueId is the animated value ID in the object we want the name.
	  * \return the name of the animated value.
	  */
	virtual const std::string& getValueName (uint valueId) const =0;

	/** 
	  * Get default track pointer.
	  *
	  * \param valueId is the animated value ID in the object we want the default track.
	  * \return The pointer on the default track of the value.
	  */
	virtual ITrack* getDefaultTrack (uint valueId) =0;

	/// \name Touch flags management

	/**
	  * Touch a value because it has been modified.
	  *
	  * \param valueId is the animated value ID in the object we want to touch.
	  */
	void touch (uint valueId)
	{
		// Set the bit
		bitSet.set (valueId+1);

		// The first bit is the "something is touched" flag. touch it.
		bitSet.set (0);
	}

	/**
	  * Return true if at least one value of this object as been touched else false.
	  */
	bool isTouched () const
	{
		// The first bit is the "something is touched" flag
		return bitSet[0];
	}

	/**
	  * Return true if the value as been touched else false.
	  *
	  * \param valueId is the animated value ID in the object we want to test the touch flag.
	  */
	bool isTouched (uint valueId) const
	{
		return bitSet[valueId+1];
	}

	/**
	  * Clear the touch flags.
	  */
	void clearFlags ()
	{
		// Clear all flags
		bitSet.clearAll ();
	}

	/**
	  * Change value count
	  *
	  * \param count is the new value count.
	  */
	void resize (uint count)
	{
		// The first bit is the "something is touched" flag
		// Bit are reseted after resize (doc), nothing invalidate
		bitSet.resize (count+1);
	}

private:

	// Use a CBitSet to manage the flags
	NLMISC::CBitSet bitSet;
};


} // NL3D

#endif // NL_ANIMATABLE_H

/* End of animatable.h */
