/** \file animated_value.h
 * Class IAnimatedValue
 *
 * $Id: animated_value.h,v 1.3 2001/02/06 10:18:05 lecroart Exp $
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

#ifndef NL_ANIMATED_VALUE_H
#define NL_ANIMATED_VALUE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/quat.h"


namespace NL3D {


/**
 * A value handled by the animation system.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class IAnimatedValue
{
public:
	/** A blend method. This method blend two the animated values and store the result
	  * in the object. The two first args can be reference on the object itself.
	  * Idealy, it performs the operation this->value=(this->value*blendFactor + value*(1.f-blendFactor))
	  *
	  * \param value is the first value in the blend operation.
	  * \param blendFactor must be in the range [0..1].
	  */
	virtual void blend (const IAnimatedValue& value, float blendFactor)=0;
};


/**
 * A template implementation of IAnimatedValue.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CAnimatedValueBlendable : public IAnimatedValue
{
public:
	// NOT TESTED, JUST COMPILED. FOR PURPOSE ONLY.
	/// A default blend method. Doesn't work for all type.
	virtual void blend (const IAnimatedValue& value, float blendFactor)
	{
		// Check types of value
		nlassert (typeid (value)==typeid(*this));

		// Cast
		CAnimatedValueBlendable<T>	*pValue=(CAnimatedValueBlendable<T>*)&value;

		// Blend
		_Value=_Value*blendFactor+pValue->_Value*(1.f-blendFactor);
	}
	
	/// Access to the value
	const T& getValue() const
	{
		return _Value;
	}

private:
	// The value
	T	_Value;
};


/**
 * A INT implementation of IAnimatedValue.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CAnimatedValueBlendable<int> : public IAnimatedValue
{
public:
	// NOT TESTED, JUST COMPILED. FOR PURPOSE ONLY.
	/// A int blend method.
	virtual void blend (const IAnimatedValue& value, float blendFactor)
	{
		// Check types of value
		nlassert (typeid (value)==typeid(*this));

		// Cast
		CAnimatedValueBlendable<int>	*pValue=(CAnimatedValueBlendable<int>*)&value;

		// Blend
		_Value=(int)(((float)_Value)*blendFactor+((float)pValue->_Value)*(1.f-blendFactor));
	}
	
	/// Access to the value
	const int& getValue() const
	{
		return _Value;
	}

private:
	int		_Value;
};


/**
 * A QUATERNION implementation of IAnimatedValue.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CAnimatedValueBlendable<NLMISC::CQuat> : public IAnimatedValue
{
public:
	/// A quat blend method.
	virtual void blend (const IAnimatedValue& value, float blendFactor)
	{
		// Check types of value
		nlassert (typeid (value)==typeid(*this));

		// TODO: Implemente a slerp here.. But CQuat is not good enought..
		nlstop;
	}

	/// Access to the value
	const NLMISC::CQuat& getValue() const
	{
		return _Value;
	}

private:
	NLMISC::CQuat	_Value;
};


/**
 * A template implementation of IAnimatedValue not blendable.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CAnimatedValueNotBlendable : public IAnimatedValue
{
public:
	/// A default blend method. Doesn't work for all type.
	virtual void blend (const IAnimatedValue& value, float blendFactor)
	{
		// Check types of value
		nlassert (typeid (value)==typeid(*this));

		// Cast
		CAnimatedValueNotBlendable<T>	*pValue=(CAnimatedValueNotBlendable<T>*)&value;

		// Boolean blend
		if (blendFactor<0.5f)
			_Value=pValue->_Value;
	}
	
	/// Access to the value read only.
	const T& getValue() const
	{
		return _Value;
	}

	/// Access to the value writable.
	void setValue(const T& value)
	{
		_Value=value;
	}

private:
	// The value
	T	_Value;
};


typedef CAnimatedValueNotBlendable<bool> CAnimatedValueBool;
typedef CAnimatedValueBlendable<int> CAnimatedValueInt;
typedef CAnimatedValueBlendable<float> CAnimatedValueFloat;
typedef CAnimatedValueBlendable<NLMISC::CVector> CAnimatedValueVector;
typedef CAnimatedValueNotBlendable<std::string> CAnimatedValueString;
typedef CAnimatedValueBlendable<NLMISC::CQuat> CAnimatedValueQuad;


} // NL3D


#endif // NL_ANIMATED_VALUE_H

/* End of animated_value.h */
