/** \file channel.h
 * class IChannel.
 *
 * $Id: channel.h,v 1.1 2001/06/15 16:24:42 corvazier Exp $
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

#ifndef NL_CHANNEL_H
#define NL_CHANNEL_H

#include "nel/misc/types_nl.h"
#include "3d/animated_value.h"

namespace NL3D 
{


/**
 * An animated channel.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class IChannel
{
public:

	/** 
	  * Get the read only value reference
	  * 
	  * \return a read only reference on the value.
	  */
	virtual const IAnimatedValue& getValue (const IAnimatedValue& value) const=0;

	/** 
	  * Get a writable value reference
	  * 
	  * \return a writable reference on the value.
	  */
	virtual IAnimatedValue& getValue ()=0;

	/// Return the default value of this channel. Must be implemented
	virtual const IAnimatedValue& getDefaultValue () const=0;

	/// \name acces

	/// Get the name of this channel.
	const std::string& getName () const
	{
		return _Name;
	}

	/// Set the name of this channel.
	void setName (const std::string& name)
	{
		_Name=name;
	}
	
private:
	// The name of the channel
	std::string		_Name;
};


/**
 * An animated template channel.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CChannelBlendable : public IChannel
{
public:

	/// Set the channel value.
	virtual void setValue (const IAnimatedValue& value)
	{
		// Check good type !!
		nlassert (typeid (_DefaultValue)==typeid (value));

		// Ok assign!
		_Value=*(CAnimatedValueBlendable<T>*)&value;
	}

	/// Get the read only value reference
	virtual const IAnimatedValue& getValue (const IAnimatedValue& value) const
	{
		return _Value;
	}

	/// Get a writable value reference
	virtual IAnimatedValue& getValue ()
	{
		return _Value;
	}

	/// Return the default value of this channel. Must be implemented
	const IAnimatedValue& getDefaultValue () const
	{
		return _DefaultValue;
	}
private:
	// The value of the channel
	CAnimatedValueBlendable<T>	_Value;

	// The default value of the channel
	CAnimatedValueBlendable<T>	_DefaultValue;
};


/**
 * An animated template channel.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CChannelNotBlendable : public IChannel
{
public:

	/// Set the channel value.
	virtual void setValue (const IAnimatedValue& value)
	{
		// Check good type !!
		nlassert (typeid (_DefaultValue)==typeid (value));

		// Ok assign!
		_Value=*(CAnimatedValueNotBlendable<T>*)&value;
	}

	/// Get the read only value reference
	virtual const IAnimatedValue& getValue (const IAnimatedValue& value) const
	{
		return _Value;
	}

	/// Get a writable value reference
	virtual IAnimatedValue& getValue ()
	{
		return _Value;
	}

	/// Return the default value of this channel. Must be implemented
	const IAnimatedValue& getDefaultValue () const
	{
		return _DefaultValue;
	}
private:
	// The value of the channel
	CAnimatedValueNotBlendable<T>	_Value;

	// The default value of the channel
	CAnimatedValueNotBlendable<T>	_DefaultValue;
};


typedef CChannelNotBlendable<bool> CChannelBool;
typedef CChannelBlendable<int> CChannelInt;
typedef CChannelBlendable<float> CChannelFloat;
typedef CChannelBlendable<NLMISC::CVector> CChannelVector;
typedef CChannelBlendable<NLMISC::CQuat> CChannelQuat;
typedef CChannelNotBlendable<std::string> CChannelString;


} // NL3D


#endif // NL_CHANNEL_H

/* End of channel.h */
