/** \file animatable.h
 * Class IAnimatable
 *
 * $Id: animatable.h,v 1.5 2001/03/19 14:07:57 berenguier Exp $
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
class CChannelMixer;


/**
 * An animatable object. 
 *
 * This object can have a set of animated values.
 * Animated values are animated by a CChannelMixer object.
 * Each value have a name and a default track.
 *
 * An IAnimatable may have IAnimatable sons (list of bones, list of materails etc...). The value count and valueId of 
 * the IAnimatable DO NOT count those sons, but register() should register his sons too.
 * A father propagated touch system (setFather()) is implemented. When a son is touched, he touchs his fathers, his grandfather
 * and so on.
 *
 * When a class derives from IAnimatable, it must implement all the 
 * interface's methods:
 *
 *	extend TAnimValues enum, beginning to BaseClass::AnimValueLast
 *	ctor(): just type "IAnimatable::resize (AnimValueLast);"
 *	virtual IAnimatedValue* getValue (uint valueId);
 *	virtual const char *getValueName (uint valueId) const;
 *	virtual ITrack* getDefaultTrack (uint valueId);
 *
 *	virtual register(CChannelMixer *, const string &prefix);
 *
 *
 * Watch NL3D::ITransformable and NL3D::CTransform for a good example.
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
	  * Deriver: should just write:  IAnimatable::resize (getValueCount());
	  * 
	  */
	IAnimatable ()
	{
		bitSet.resize (1);
		_Father= NULL;
	}

	/// \name Interface
	// @{
	/**
	  * The enum of animated values. (same system in CMOT). Deriver should extend this enum, beginning to BaseClass::AnimValueLast.
	  */
	enum	TAnimValues
	{
		AnimValueLast=0,
	};

	/** 
	  * Get a value pointer.
	  *
	  * \param valueId is the animated value ID in the object. IGNORING IANIMATABLE SONS (eg: bones, materials...).
	  * \return The pointer on the animated value.
	  */
	virtual IAnimatedValue* getValue (uint valueId) =0;

	/**
	  * Get animated value name.
	  *
	  * \param valueId is the animated value ID in the object we want the name. IGNORING IANIMATABLE SONS (eg: bones, materials...).
	  * \return the name of the animated value.
	  */
	virtual const char *getValueName (uint valueId) const =0;

	/** 
	  * Get default track pointer.
	  *
	  * \param valueId is the animated value ID in the object we want the default track. IGNORING IANIMATABLE SONS (eg: bones, materials...).
	  * \return The pointer on the default track of the value.
	  */
	virtual ITrack* getDefaultTrack (uint valueId) =0;

	/** 
	  * register the Aniamtable to a channelMixer (using CChannelMixer::addChannel()). You MUST use this method to register Animatable.
	  * This method should:
	  *		- call is BaseClass method.
	  *		- register local AnimatableValues, with channel name:	prefix+getValueName().
	  *		- register local sons!!. eg: matlist[0]->registerToChannelMixer(chanMixer, prefix+"mat0.").
	  *
	  * \param chanMixer is the channel mixer. Should not be NULL. for anim detail purpose , the IAnimatable may store a RefPtr on this channel mixer.
	  * \param prefix prefix to be append to valueNames
	  */
	virtual	void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix) =0;

	// @}



	/// \name Touch flags management
	// @{

	/**
	  * Say which (if any) IAnimatable owns this one. This is important for Touch propagation.
	  * By this system, Fathers and ancestors know if they must check their sons (isTouched() return true).
	  *
	  * \param valueId is the animated value ID in the object we want to touch. IGNORING IANIMATABLE SONS (eg: bones, materials...).
	  */
	void	setFather(IAnimatable *father) {_Father= father;}


	/**
	  * Touch a value because it has been modified.
	  *
	  * \param valueId is the animated value ID in the object we want to touch. IGNORING IANIMATABLE SONS (eg: bones, materials...).
	  */
	void touch (uint valueId)
	{
		// Set the bit
		bitSet.set (valueId+1);

		// propagate the touch to the bit 0, and the fathers.
		propagateTouch();
	}

	/**
	  * Return true if at least one value of this object as been touched, or if at least one son has been touched.
	  * Else return false.
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
	// @}


private:

	// Use a CBitSet to manage the flags
	NLMISC::CBitSet bitSet;
	// The owner of this IAnimatable.
	IAnimatable		*_Father;

	void	propagateTouch()
	{
		IAnimatable		*pCur= this;
		// Stop when no father, or when father is already touched (and so the grandfather...!!!).
		while(pCur && !pCur->isTouched())
		{
			// The first bit is the "something is touched" flag. touch it.
			pCur->bitSet.set (0);
			pCur= pCur->_Father;
		}
	}


protected:
	/// This is a tool function which add a given value to a channel.
	void	addValue(CChannelMixer *chanMixer, uint valueId, const std::string &prefix, bool detail);

	/// This method clear a bit in the bitset.
	void	clearFlag(uint valueId)
	{
		bitSet.clear(valueId+1);
	}

};


} // NL3D

#endif // NL_ANIMATABLE_H

/* End of animatable.h */
